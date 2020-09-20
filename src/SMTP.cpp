/*++

Library name:

  libdelphi

Module Name:

  SMTP.cpp

Notices:

  Delphi classes for C++

Author:

  Copyright (c) Prepodobny Alen

  mailto: alienufo@inbox.ru
  mailto: ufocomp@gmail.com

--*/

#include "delphi.hpp"
#include "delphi/SMTP.hpp"

extern "C++" {

namespace Delphi {

    namespace SMTP {

        //--------------------------------------------------------------------------------------------------------------

        //-- CSMTPConfig -----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CSMTPConfig::CSMTPConfig(): CObject() {
            m_Location.port = 25;
        }
        //--------------------------------------------------------------------------------------------------------------

        CSMTPConfig::CSMTPConfig(const CSMTPConfig &Value) {
            Assign(Value);
        }
        //--------------------------------------------------------------------------------------------------------------

        CSMTPConfig::CSMTPConfig(const CString &Host): CSMTPConfig() {
            m_Location = Host;
        }
        //--------------------------------------------------------------------------------------------------------------

        CSMTPConfig::CSMTPConfig(const CString &Host, const CString &UserName, const CString &Password): CSMTPConfig(Host) {
            m_UserName = UserName;
            m_Password = Password;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPConfig::Assign(const CSMTPConfig &Value) {
            m_Location = Value.m_Location;
            m_UserName = Value.m_UserName;
            m_Password = Value.m_Password;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPConfig::Clear() {
            m_Location.Clear();
            m_UserName.Clear();
            m_Password.Clear();
            m_Location.port = 25;
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CSMTPMessage ----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CSMTPMessage::CSMTPMessage(CSMTPClient *AClient): CSMTPConnection(AClient) {
            m_OnDone = nullptr;
            m_OnError = nullptr;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPMessage::Assign(const CSMTPMessage &Message) {
            m_MessageId = Message.m_MessageId;
            m_From = Message.m_From;
            m_To = Message.m_To;
            m_Subject = Message.m_Subject;
            m_Body = Message.m_Body;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPMessage::Clear() {
            CSMTPConnection::Clear();
            m_MessageId.Clear();
            m_From.Clear();
            m_To.Clear();
            m_Subject.Clear();
            m_Body.Clear();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPMessage::DoDone() {
            if (m_OnDone != nullptr) {
                m_OnDone(this);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPMessage::DoError() {
            if (m_OnError != nullptr) {
                m_OnError(this);
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CSMTPConnection -------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CSMTPConnection::CSMTPConnection(CPollSocketClient *AClient) : CTCPClientConnection(AClient) {
            m_ConnectionStatus = csConnected;
            m_CloseConnection = false;
        }
        //--------------------------------------------------------------------------------------------------------------

        CSMTPConnection::~CSMTPConnection() {
            Clear();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPConnection::Clear() {
            m_Request.Clear();
            m_Reply.Clear();
            m_ConnectionStatus = csConnected;
            m_CloseConnection = false;
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CSMTPConnection::ParseInput() {
            bool Result = false;
            if (Connected()) {
                CMemoryStream LStream(ReadAsync());
                Result = LStream.Size() > 0;
                if (Result) {
                    InputBuffer()->Extract(LStream.Memory(), LStream.Size());
/*
                    CHTTPReplyContext Context = CHTTPReplyContext((LPCTSTR) LStream.Memory(), LStream.Size(), m_State, m_ContentLength, m_ChunkedLength);
                    const int ParseResult = CHTTPReplyParser::Parse(GetReply(), Context);

                    switch (ParseResult) {
                        case 0:
                            Tag(clock());
                            m_ConnectionStatus = csReplyError;
                            break;

                        case 1:
                            Tag(clock());
                            m_ConnectionStatus = csReplyOk;
                            DoReply();
                            break;

                        default:
                            m_State = Context.State;

                            m_ContentLength = Context.ContentLength;
                            m_ChunkedLength = Context.ChunkedLength;

                            if (RecvBufferSize() < m_ContentLength)
                                RecvBufferSize(m_ContentLength);

                            m_ConnectionStatus = csWaitReply;

                            break;
                    }
*/
                }
            }

            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPConnection::SendRequest(bool ASendNow) {

            OutputBuffer()->WriteBuffer(m_Request.Data(), m_Request.Size());

            m_ConnectionStatus = csRequestReady;

            DoRequest();

            if (ASendNow) {
                WriteAsync();
                m_ConnectionStatus = csRequestSent;
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPConnection::DoRequest() {
            if (m_OnRequest != nullptr) {
                m_OnRequest(this);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPConnection::DoReply() {
            if (m_OnReply != nullptr) {
                m_OnReply(this);
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CSMTPClient -----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CSMTPClient::CSMTPClient(): CAsyncClient() {

        }
        //--------------------------------------------------------------------------------------------------------------

        CSMTPClient::CSMTPClient(const CSMTPConfig &Config): CAsyncClient(Config.Host(), Config.Port()) {
            m_Config = Config;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPClient::DoConnectStart(CIOHandlerSocket *AIOHandler, CPollEventHandler *AHandler) {
            auto LConnection = new CSMTPConnection(this);
            LConnection->IOHandler(AIOHandler);
            LConnection->AutoFree(true);
            AHandler->Binding(LConnection);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPClient::DoConnect(CPollEventHandler *AHandler) {
            auto LConnection = dynamic_cast<CSMTPConnection *> (AHandler->Binding());

            if (LConnection == nullptr) {
                AHandler->Stop();
                return;
            }

            try {
                auto LIOHandler = (CIOHandlerSocket *) LConnection->IOHandler();

                if (LIOHandler->Binding()->CheckConnection()) {
#if defined(_GLIBCXX_RELEASE) && (_GLIBCXX_RELEASE >= 9)
                    LConnection->OnDisconnected([this](auto && Sender) { DoDisconnected(Sender); });
#else
                    LConnection->OnDisconnected(std::bind(&CSMTPClient::DoDisconnected, this, _1));
#endif
                    DoConnected(LConnection);
                    //DoRequest(LConnection);

                    AHandler->Start(etIO);
                }
            } catch (Delphi::Exception::Exception &E) {
                DoException(LConnection, E);
                AHandler->Stop();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPClient::DoRead(CPollEventHandler *AHandler) {
            auto LConnection = dynamic_cast<CSMTPConnection *> (AHandler->Binding());
            try {
                if (LConnection->ParseInput()) {
                    switch (LConnection->ConnectionStatus()) {
                        case csReplyError:
                            LConnection->Clear();
                            break;

                        case csReplyOk:
                            DoExecute(LConnection);
                            LConnection->Clear();

                            if (LConnection->CloseConnection()) {
                                LConnection->Disconnect();
                            }

                            break;

                        default:
                            break;
                    }
                }
            } catch (Delphi::Exception::Exception &E) {
                DoException(LConnection, E);
                LConnection->Disconnect();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPClient::DoWrite(CPollEventHandler *AHandler) {
            auto LConnection = dynamic_cast<CSMTPConnection *> (AHandler->Binding());
            try {
                if (LConnection->WriteAsync()) {
                    if (LConnection->ConnectionStatus() == csRequestReady) {
                        LConnection->ConnectionStatus(csRequestSent);
                    }
                }
            } catch (Delphi::Exception::Exception &E) {
                DoException(LConnection, E);
                LConnection->Disconnect();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CSMTPClient::DoCommand(CTCPConnection *AConnection) {
            auto LConnection = dynamic_cast<CSMTPConnection *> (AConnection);
            auto LRequest = LConnection->Request();

            bool Result = CommandHandlers()->Count() > 0;
/*
            if (Result) {
                DoBeforeCommandHandler(AConnection, LRequest->Method.c_str());
                try {
                    int i;
                    for (i = 0; i < CommandHandlers()->Count(); ++i) {
                        if (CommandHandlers()->Commands(i)->Enabled()) {
                            if (CommandHandlers()->Commands(i)->Check(LRequest->Method.c_str(),
                                                                      LRequest->Method.Size(), AConnection))
                                break;
                        }
                    }

                    if (i == CommandHandlers()->Count())
                        DoNoCommandHandler(LRequest->Method.c_str(), AConnection);
                } catch (Delphi::Exception::Exception &E) {
                    DoException(AConnection, E);
                }
                DoAfterCommandHandler(AConnection);
            }
*/
            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CSMTPClient::DoExecute(CTCPConnection *AConnection) {
            if (m_OnExecute != nullptr) {
                return m_OnExecute(AConnection);
            }
            return DoCommand(AConnection);
        }
        //--------------------------------------------------------------------------------------------------------------

        int CSMTPClient::IndexOfMessageId(const CString &MessageId) const {
            int Index = 0;
            while (Index < Count() && Items(Index)->MessageId() != MessageId)
                Index++;
            if (Index == Count())
                Index = -1;
            return Index;
        }
        //--------------------------------------------------------------------------------------------------------------

        CSMTPMessage *CSMTPClient::NewMessage() {
            return new CSMTPMessage(this);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPClient::SendMail() {
            ConnectStart();
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CSMTPCollectionItem -------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CSMTPCollectionItem::CSMTPCollectionItem(CSMTPManager *AManager): CCollectionItem(AManager), CSMTPClient() {

        }
        //--------------------------------------------------------------------------------------------------------------

        CSMTPCollectionItem::CSMTPCollectionItem(CSMTPManager *AManager, const CSMTPConfig &Config):
                CCollectionItem(AManager), CSMTPClient(Config) {

        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CSMTPManager ----------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CSMTPCollectionItem *CSMTPManager::GetItem(int Index) const {
            return (CSMTPCollectionItem *) inherited::GetItem(Index);
        }
        //--------------------------------------------------------------------------------------------------------------

        CSMTPCollectionItem *CSMTPManager::Add(const CSMTPConfig &Config) {
            return new CSMTPCollectionItem(this, Config);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPManager::CleanUp() {
            for (int i = Count() - 1; i >= 0; i--) {
                auto Item = GetItem(i);
                if (Item->Active() && Item->ConnectionCount() == 0)
                    Delete(i);
            }
        }

    }
}
}
