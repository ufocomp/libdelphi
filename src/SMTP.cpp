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

        //-- CSMTPCommand ----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CSMTPCommand::CSMTPCommand(): CObject() {
            m_Command = _T("CONNECT");
            m_LastCode = 0;
        }
        //--------------------------------------------------------------------------------------------------------------

        CSMTPCommand::CSMTPCommand(const CSMTPCommand &Value): CSMTPCommand() {
            Assign(Value);
        }
        //--------------------------------------------------------------------------------------------------------------

        CSMTPCommand::CSMTPCommand(const CString &Command, const CString &Data): CSMTPCommand() {
            m_Command = Command;
            m_Data = Data.IsEmpty() ? Command : Data;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPCommand::Assign(const CSMTPCommand &Value) {
            m_Command = Value.m_Command;
            m_Data = Value.m_Data;
            m_LastCode = Value.m_LastCode;
            m_LastMessage = Value.m_LastMessage;
            m_Reply = Value.m_Reply;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPCommand::Clear() {
            m_Command = _T("CONNECT");
            m_Data.Clear();
            m_LastCode = 0;
            m_LastMessage.Clear();
            m_Reply.Clear();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPCommand::ToBuffers(CMemoryStream *AStream) const {
            m_Data.SaveToStream(AStream);
            AStream->Write(_T("\r\n"), 2);
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CSMTPMessage ----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CSMTPMessage::CSMTPMessage(CSMTPClient *AClient): CSMTPConnection(AClient) {
            m_Sent = false;
            m_OnSent = nullptr;
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

        void CSMTPMessage::DoSent() {
            if (m_OnSent != nullptr) {
                m_OnSent(this);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPMessage::DoError() {
            if (m_OnError != nullptr) {
                m_OnError(this);
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CSMTPReplyParser ------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CSMTPReplyParser::CSMTPReplyParser(LPCTSTR ABegin, LPCTSTR AEnd) {
            m_Result = -1;
            m_State = smtp_code;
            m_pBegin = ABegin;
            m_pEnd = AEnd;
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CSMTPReplyParser::IsChar(int c) {
            return c >= 0 && c <= 127;
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CSMTPReplyParser::IsCtl(int c) {
            return (c >= 0 && c <= 31) || (c == 127);
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CSMTPReplyParser::IsDigit(int c) {
            return c >= '0' && c <= '9';
        }
        //--------------------------------------------------------------------------------------------------------------

        int CSMTPReplyParser::Consume(CSMTPCommand &Command, char AInput) {
            switch (m_State) {
                case smtp_code:
                    if (IsCtl(AInput)) {
                        return 0;
                    } else if (AInput == ' ') {
                        m_Line.Append(AInput);
                        m_State = smtp_code_space;
                        return -1;
                    } else if (AInput == '-') {
                        m_Line.Append(AInput);
                        m_State = smtp_code_hyphen;
                        return -1;
                    } else {
                        m_Line.Append(AInput);
                        if (IsDigit(AInput)) {
                            m_Code.Append(AInput);
                        } else {
                            m_State = smtp_text;
                        }
                        return -1;
                    }
                case smtp_code_hyphen:
                case smtp_code_space:
                    if (!m_Line.IsEmpty()) {
                        Command.LastCode(StrToInt(m_Code.c_str()));
                        m_Code.Clear();
                        m_Line.Append(AInput);
                        m_State = smtp_text;
                        return -1;
                    }
                    return 0;
                case smtp_text:
                    if (AInput == '\r') {
                        m_State = smtp_newline;
                        return -1;
                    } else if (IsCtl(AInput)) {
                        return 0;
                    } else {
                        m_Line.Append(AInput);
                        return -1;
                    }
                case smtp_newline:
                    if (AInput == '\n') {
                        Command.Reply().Add(m_Line);
                        Command.LastMessage() = m_Line.SubString(4);
                        m_Line.Clear();
                        m_State = smtp_code;
                        return m_pBegin == m_pEnd ? 1 : -1;
                    }
                    return 0;
            }
            return 0;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CSMTPReplyParser::Parse(CSMTPCommand &Command) {
            int Result = -1;
            while (Result == -1 && m_pBegin != m_pEnd) {
                Result = Consume(Command, *m_pBegin++);
            }
            return Result;
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CSMTPConnection -------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CSMTPConnection::CSMTPConnection(CPollSocketClient *AClient) : CTCPClientConnection(AClient) {
            m_ConnectionStatus = csConnected;
            m_CloseConnection = false;
            m_OnWaitRequest = nullptr;
            m_OnRequest = nullptr;
            m_OnReply= nullptr;
        }
        //--------------------------------------------------------------------------------------------------------------

        CSMTPConnection::~CSMTPConnection() {
            CSMTPConnection::Clear();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPConnection::Clear() {
            m_Command.Clear();
            m_ConnectionStatus = csConnected;
            m_CloseConnection = false;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPConnection::NewCommand(const CString &Command, const CString &Data) {
            m_Command.Clear();
            m_Command.Command() = Command;
            m_Command.Data() = Data.IsEmpty() ? Command : Data;
            m_ConnectionStatus = csRequestReady;
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CSMTPConnection::ParseInput() {
            bool Result = false;
            if (Connected()) {
                CMemoryStream LStream(ReadAsync());
                Result = LStream.Size() > 0;
                if (Result) {
                    InputBuffer()->Extract(LStream.Memory(), LStream.Size());

                    CSMTPReplyParser LParser((LPCSTR) LStream.Memory(), (LPCSTR) LStream.Memory() + LStream.Size());
                    const int ParseResult = LParser.Parse(m_Command);

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
                            m_ConnectionStatus = csWaitReply;
                            break;
                    }
                }
            }

            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CSMTPConnection::SendCommand() {
            m_Command.ToBuffers(OutputBuffer());
            DoRequest();
            return WriteAsync();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPConnection::DoWaitRequest() {
            if (m_OnWaitRequest != nullptr) {
                m_OnWaitRequest(this);
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
            m_MessageIndex = 0;
            m_ToIndex = 0;
            m_OnRequest = nullptr;
            m_OnReply = nullptr;
        }
        //--------------------------------------------------------------------------------------------------------------

        CSMTPClient::CSMTPClient(const CSMTPConfig &Config): CAsyncClient(Config.Host(), Config.Port()) {
            m_MessageIndex = 0;
            m_ToIndex = 0;
            m_Config = Config;
            m_OnRequest = nullptr;
            m_OnReply = nullptr;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPClient::InitializeCommandHandlers() {
            CCommandHandler *LCommand;

            CommandHandlers()->ParseParamsDefault(false);

            LCommand = CommandHandlers()->Add();
            LCommand->Command() = _T("CONNECT");
            LCommand->Disconnect(false);
#if defined(_GLIBCXX_RELEASE) && (_GLIBCXX_RELEASE >= 9)
            LCommand->OnCommand([this](auto && ACommand) { DoCONNECT(ACommand); });
            LCommand->OnException([this](auto && AConnection, auto && E) { DoException(AConnection, E); });
#else
            LCommand->OnCommand(std::bind(&CSMTPClient::DoCONNECT, this, _1));
            LCommand->OnException(std::bind(&CSMTPClient::DoException, this, _1, _2));
#endif

            LCommand = CommandHandlers()->Add();
            LCommand->Command() = _T("HELLO");
            LCommand->Disconnect(false);
#if defined(_GLIBCXX_RELEASE) && (_GLIBCXX_RELEASE >= 9)
            LCommand->OnCommand([this](auto && ACommand) { DoHELLO(ACommand); });
            LCommand->OnException([this](auto && AConnection, auto && E) { DoException(AConnection, E); });
#else
            LCommand->OnCommand(std::bind(&CSMTPClient::DoHELLO, this, _1));
            LCommand->OnException(std::bind(&CSMTPClient::DoException, this, _1, _2));
#endif

            LCommand = CommandHandlers()->Add();
            LCommand->Command() = _T("STARTTLS");
            LCommand->Disconnect(false);
#if defined(_GLIBCXX_RELEASE) && (_GLIBCXX_RELEASE >= 9)
            LCommand->OnCommand([this](auto && ACommand) { DoSTARTTLS(ACommand); });
            LCommand->OnException([this](auto && AConnection, auto && E) { DoException(AConnection, E); });
#else
            LCommand->OnCommand(std::bind(&CSMTPClient::DoSTARTTLS, this, _1));
            LCommand->OnException(std::bind(&CSMTPClient::DoException, this, _1, _2));
#endif

            LCommand = CommandHandlers()->Add();
            LCommand->Command() = _T("AUTH");
            LCommand->Disconnect(false);
#if defined(_GLIBCXX_RELEASE) && (_GLIBCXX_RELEASE >= 9)
            LCommand->OnCommand([this](auto && ACommand) { DoAUTH(ACommand); });
            LCommand->OnException([this](auto && AConnection, auto && E) { DoException(AConnection, E); });
#else
            LCommand->OnCommand(std::bind(&CSMTPClient::DoAUTH, this, _1));
            LCommand->OnException(std::bind(&CSMTPClient::DoException, this, _1, _2));
#endif

            LCommand = CommandHandlers()->Add();
            LCommand->Command() = _T("FROM");
            LCommand->Disconnect(false);
#if defined(_GLIBCXX_RELEASE) && (_GLIBCXX_RELEASE >= 9)
            LCommand->OnCommand([this](auto && ACommand) { DoFROM(ACommand); });
            LCommand->OnException([this](auto && AConnection, auto && E) { DoException(AConnection, E); });
#else
            LCommand->OnCommand(std::bind(&CSMTPClient::DoFROM, this, _1));
            LCommand->OnException(std::bind(&CSMTPClient::DoException, this, _1, _2));
#endif

            LCommand = CommandHandlers()->Add();
            LCommand->Command() = _T("TO");
            LCommand->Disconnect(false);
#if defined(_GLIBCXX_RELEASE) && (_GLIBCXX_RELEASE >= 9)
            LCommand->OnCommand([this](auto && ACommand) { DoTO(ACommand); });
            LCommand->OnException([this](auto && AConnection, auto && E) { DoException(AConnection, E); });
#else
            LCommand->OnCommand(std::bind(&CSMTPClient::DoTO, this, _1));
            LCommand->OnException(std::bind(&CSMTPClient::DoException, this, _1, _2));
#endif

            LCommand = CommandHandlers()->Add();
            LCommand->Command() = _T("DATA");
            LCommand->Disconnect(false);
#if defined(_GLIBCXX_RELEASE) && (_GLIBCXX_RELEASE >= 9)
            LCommand->OnCommand([this](auto && ACommand) { DoDATA(ACommand); });
            LCommand->OnException([this](auto && AConnection, auto && E) { DoException(AConnection, E); });
#else
            LCommand->OnCommand(std::bind(&CSMTPClient::DoDATA, this, _1));
            LCommand->OnException(std::bind(&CSMTPClient::DoException, this, _1, _2));
#endif

            LCommand = CommandHandlers()->Add();
            LCommand->Command() = _T("CONTENT");
            LCommand->Disconnect(false);
#if defined(_GLIBCXX_RELEASE) && (_GLIBCXX_RELEASE >= 9)
            LCommand->OnCommand([this](auto && ACommand) { DoCONTENT(ACommand); });
            LCommand->OnException([this](auto && AConnection, auto && E) { DoException(AConnection, E); });
#else
            LCommand->OnCommand(std::bind(&CSMTPClient::DoCONTENT, this, _1));
            LCommand->OnException(std::bind(&CSMTPClient::DoException, this, _1, _2));
#endif

            LCommand = CommandHandlers()->Add();
            LCommand->Command() = _T("QUIT");
            LCommand->Disconnect(true);
#if defined(_GLIBCXX_RELEASE) && (_GLIBCXX_RELEASE >= 9)
            LCommand->OnCommand([this](auto && ACommand) { DoQUIT(ACommand); });
            LCommand->OnException([this](auto && AConnection, auto && E) { DoException(AConnection, E); });
#else
            LCommand->OnCommand(std::bind(&CSMTPClient::DoQUIT, this, _1));
            LCommand->OnException(std::bind(&CSMTPClient::DoException, this, _1, _2));
#endif
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
                    LConnection->OnRequest([this](auto && Sender) { DoRequest(Sender); });
                    LConnection->OnReply([this](auto && Sender) { DoReply(Sender); });
                    LConnection->OnDisconnected([this](auto && Sender) { DoDisconnected(Sender); });
#else
                    LConnection->OnRequest(std::bind(&CSMTPClient::DoRequest, this, _1));
                    LConnection->OnReply(std::bind(&CSMTPClient::DoReply, this, _1));
                    LConnection->OnDisconnected(std::bind(&CSMTPClient::DoDisconnected, this, _1));
#endif
                    DoConnected(LConnection);

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
                            LConnection->Disconnect();
                            break;

                        case csReplyOk:
                            DoExecute(LConnection);
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
                switch (LConnection->ConnectionStatus()) {
                    case Socket::csConnected:
                        break;

                    case csRequestError:
                        LConnection->Clear();
                        LConnection->Disconnect();
                        break;

                    case csRequestReady:
                        if (LConnection->SendCommand()) {
                            LConnection->ConnectionStatus(csWaitReply);
                        } else {
                            LConnection->ConnectionStatus(csRequestSent);
                        }
                        break;

                    case csRequestSent:
                        if (LConnection->WriteAsync()) {
                            LConnection->ConnectionStatus(csWaitReply);
                        }
                        break;
                    default:
                        break;
                }
            } catch (Delphi::Exception::Exception &E) {
                DoException(LConnection, E);
                LConnection->Disconnect();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CSMTPClient::DoCommand(CTCPConnection *AConnection) {
            CCommandHandler *Handler;

            auto LConnection = dynamic_cast<CSMTPConnection *> (AConnection);
            const auto& LCommand = LConnection->Command();

            bool Result = CommandHandlers()->Count() > 0;

            if (Result) {
                DoBeforeCommandHandler(AConnection, LCommand.Command());
                try {
                    int Index;
                    for (Index = 0; Index < CommandHandlers()->Count(); ++Index) {
                        Handler = CommandHandlers()->Commands(Index);
                        if (Handler->Enabled()) {
                            if (Handler->Check(LCommand.Command(), AConnection))
                                break;
                        }
                    }
                    if (Index == CommandHandlers()->Count())
                        DoNoCommandHandler(LCommand.Command(), AConnection);
                } catch (Delphi::Exception::Exception &E) {
                    DoException(AConnection, E);
                }
                DoAfterCommandHandler(AConnection);
            }

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
            //UsedSSL(m_Config.Port() == 465);
            Active(Count() != 0);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPClient::DoCONNECT(CCommand *ACommand) {
            auto LConnection = dynamic_cast<CSMTPConnection *> (ACommand->Connection());
            const auto& LCommand = LConnection->Command();
            if (LCommand.LastCode() == 220) {
                LConnection->NewCommand("HELLO", CString().Format("EHLO %s", LConnection->Socket()->Binding()->IP()));
            } else {
                LConnection->CloseConnection(true);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPClient::DoHELLO(CCommand *ACommand) {
            auto LConnection = dynamic_cast<CSMTPConnection *> (ACommand->Connection());
            const auto& LCommand = LConnection->Command();
            if (LCommand.LastCode() == 250) {

                int Index = 0;
                while (Index < LCommand.Reply().Count() && LCommand.Reply()[Index].Find("STARTTLS") == -1)
                    Index++;

                if (Index == LCommand.Reply().Count()) {
                    CString LPlain;

                    LPlain.Write("\0", 1);
                    LPlain << m_Config.UserName();
                    LPlain.Write("\0", 1);
                    LPlain << m_Config.Password();

                    LConnection->NewCommand("AUTH", "AUTH PLAIN " + base64_encode(LPlain));
                } else {
                    LConnection->NewCommand("STARTTLS");
                }
            } else {
                LConnection->NewCommand("QUIT");
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPClient::DoSTARTTLS(CCommand *ACommand) {
            auto LConnection = dynamic_cast<CSMTPConnection *> (ACommand->Connection());
            const auto& LCommand = LConnection->Command();
            if (LCommand.LastCode() == 220) {
                auto Socket = LConnection->Socket()->Binding();
                if (Assigned(Socket)) {
                    Socket->SSLMethod(sslClient);
                    Socket->AllocateSSL();
                    Socket->ConnectSSL();
                }
                LConnection->NewCommand("HELLO", CString().Format("EHLO %s", LConnection->Socket()->Binding()->IP()));
            } else {
                LConnection->NewCommand("QUIT");
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPClient::DoAUTH(CCommand *ACommand) {
            auto LConnection = dynamic_cast<CSMTPConnection *> (ACommand->Connection());
            const auto& LCommand = LConnection->Command();
            if (LCommand.LastCode() == 235) {
                auto LMessage = Items(m_MessageIndex);
                LConnection->NewCommand("FROM", CString().Format("MAIL FROM:<%s>", LMessage->From().c_str()));
            } else {
                LConnection->NewCommand("QUIT");
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPClient::DoFROM(CCommand *ACommand) {
            auto LConnection = dynamic_cast<CSMTPConnection *> (ACommand->Connection());
            const auto& LCommand = LConnection->Command();
            if (LCommand.LastCode() == 250) {
                auto LMessage = Items(m_MessageIndex);
                LConnection->NewCommand("TO", CString().Format("RCPT TO:<%s>", LMessage->To()[m_ToIndex++].c_str()));
            } else {
                LConnection->NewCommand("QUIT");
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPClient::DoTO(CCommand *ACommand) {
            auto LConnection = dynamic_cast<CSMTPConnection *> (ACommand->Connection());
            const auto& LCommand = LConnection->Command();
            if (LCommand.LastCode() == 250 || LCommand.LastCode() == 251) {
                auto LMessage = Items(m_MessageIndex);
                if (m_ToIndex < LMessage->To().Count()) {
                    LConnection->NewCommand("TO", CString().Format("RCPT TO:<%s>", LMessage->To()[m_ToIndex++].c_str()));
                } else {
                    LConnection->NewCommand("DATA");
                }
            } else {
                LConnection->NewCommand("QUIT");
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPClient::DoDATA(CCommand *ACommand) {
            auto LConnection = dynamic_cast<CSMTPConnection *> (ACommand->Connection());
            const auto& LCommand = LConnection->Command();
            if (LCommand.LastCode() == 250 || LCommand.LastCode() == 354) {
                auto LMessage = Items(m_MessageIndex);

                CStringList LBody;

                CString LDate;
                LDate.SetLength(64);
                if (CHTTPReply::GetGMT(LDate.Data(), LDate.Size()) != nullptr) {
                    LDate.Truncate();
                    LBody.Add("Date: " + LDate);
                }

                CString LTo(LMessage->To()[0]);
                for (int i = 1; i < LMessage->To().Count(); ++i) {
                    LTo << ", " << LMessage->To()[i];
                }

                LBody.Add("From: " + LMessage->From());
                LBody.Add("To: " + LTo);
                LBody.Add("Subject: " + LMessage->Subject());

                LBody << LMessage->Body();

                LConnection->NewCommand("CONTENT", LBody.Text() + "\r\n.");
            } else {
                LConnection->NewCommand("QUIT");
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPClient::DoCONTENT(CCommand *ACommand) {
            auto LConnection = dynamic_cast<CSMTPConnection *> (ACommand->Connection());
            const auto& LCommand = LConnection->Command();
            if (LCommand.LastCode() == 250) {
                auto LMessage = Items(m_MessageIndex);
                LMessage->m_Sent = true;
            }
            LConnection->NewCommand("QUIT");
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPClient::DoQUIT(CCommand *ACommand) {
            auto LMessage = Items(m_MessageIndex);

            if (LMessage->Sent()) {
                LMessage->DoSent();
            } else {
                LMessage->DoError();
            }

            m_ToIndex = 0;
            m_MessageIndex++;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPClient::DoRequest(CObject *Sender) {
            if (m_OnRequest != nullptr) {
                m_OnRequest(Sender);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPClient::DoReply(CObject *Sender) {
            if (m_OnReply != nullptr) {
                m_OnReply(Sender);
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CSMTPCollectionItem ---------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CSMTPCollectionItem::CSMTPCollectionItem(CSMTPManager *AManager): CCollectionItem(AManager), CSMTPClient() {

        }
        //--------------------------------------------------------------------------------------------------------------

        CSMTPCollectionItem::CSMTPCollectionItem(CSMTPManager *AManager, const CSMTPConfig &Config):
                CCollectionItem(AManager), CSMTPClient(Config) {

        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CSMTPManager ----------------------------------------------------------------------------------------------

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
