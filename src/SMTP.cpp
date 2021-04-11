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
//----------------------------------------------------------------------------------------------------------------------

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

        void CSMTPCommand::ToBuffers(CMemoryStream &Stream) const {
            m_Data.SaveToStream(Stream);
            Stream.Write(_T("\r\n"), 2);
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
            m_OnWaitReply = nullptr;
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

                    CSMTPReplyParser pParser((LPCSTR) LStream.Memory(), (LPCSTR) LStream.Memory() + LStream.Size());
                    const int ParseResult = pParser.Parse(m_Command);

                    switch (ParseResult) {
                        case 0:
                            m_ConnectionStatus = csReplyError;
                            break;

                        case 1:
                            m_ConnectionStatus = csReplyOk;
                            DoReply();
                            break;

                        default:
                            DoWaitReply();
                            m_ConnectionStatus = csWaitReply;
                            break;
                    }
                }
            }

            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CSMTPConnection::SendCommand() {
            m_Command.ToBuffers(*OutputBuffer());
            DoRequest();
            return WriteAsync();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPConnection::DoWaitReply() {
            if (m_OnWaitReply != nullptr) {
                m_OnWaitReply(this);
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

        CSMTPClient::CSMTPClient(const CSMTPConfig &Config): CSMTPClient() {
            m_Config = Config;
            m_Host = Config.Host();
            m_Port = Config.Port();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPClient::InitializeCommandHandlers() {
            CCommandHandler *pCommand;

            CommandHandlers()->ParseParamsDefault(false);

            pCommand = CommandHandlers()->Add();
            pCommand->Command() = _T("CONNECT");
            pCommand->Disconnect(false);
#if defined(_GLIBCXX_RELEASE) && (_GLIBCXX_RELEASE >= 9)
            pCommand->OnCommand([this](auto && ACommand) { DoCONNECT(ACommand); });
            pCommand->OnException([this](auto && AConnection, auto && E) { DoException(AConnection, E); });
#else
            pCommand->OnCommand(std::bind(&CSMTPClient::DoCONNECT, this, _1));
            pCommand->OnException(std::bind(&CSMTPClient::DoException, this, _1, _2));
#endif

            pCommand = CommandHandlers()->Add();
            pCommand->Command() = _T("HELLO");
            pCommand->Disconnect(false);
#if defined(_GLIBCXX_RELEASE) && (_GLIBCXX_RELEASE >= 9)
            pCommand->OnCommand([this](auto && ACommand) { DoHELLO(ACommand); });
            pCommand->OnException([this](auto && AConnection, auto && E) { DoException(AConnection, E); });
#else
            pCommand->OnCommand(std::bind(&CSMTPClient::DoHELLO, this, _1));
            pCommand->OnException(std::bind(&CSMTPClient::DoException, this, _1, _2));
#endif

            pCommand = CommandHandlers()->Add();
            pCommand->Command() = _T("STARTTLS");
            pCommand->Disconnect(false);
#if defined(_GLIBCXX_RELEASE) && (_GLIBCXX_RELEASE >= 9)
            pCommand->OnCommand([this](auto && ACommand) { DoSTARTTLS(ACommand); });
            pCommand->OnException([this](auto && AConnection, auto && E) { DoException(AConnection, E); });
#else
            pCommand->OnCommand(std::bind(&CSMTPClient::DoSTARTTLS, this, _1));
            pCommand->OnException(std::bind(&CSMTPClient::DoException, this, _1, _2));
#endif

            pCommand = CommandHandlers()->Add();
            pCommand->Command() = _T("AUTH");
            pCommand->Disconnect(false);
#if defined(_GLIBCXX_RELEASE) && (_GLIBCXX_RELEASE >= 9)
            pCommand->OnCommand([this](auto && ACommand) { DoAUTH(ACommand); });
            pCommand->OnException([this](auto && AConnection, auto && E) { DoException(AConnection, E); });
#else
            pCommand->OnCommand(std::bind(&CSMTPClient::DoAUTH, this, _1));
            pCommand->OnException(std::bind(&CSMTPClient::DoException, this, _1, _2));
#endif

            pCommand = CommandHandlers()->Add();
            pCommand->Command() = _T("FROM");
            pCommand->Disconnect(false);
#if defined(_GLIBCXX_RELEASE) && (_GLIBCXX_RELEASE >= 9)
            pCommand->OnCommand([this](auto && ACommand) { DoFROM(ACommand); });
            pCommand->OnException([this](auto && AConnection, auto && E) { DoException(AConnection, E); });
#else
            pCommand->OnCommand(std::bind(&CSMTPClient::DoFROM, this, _1));
            pCommand->OnException(std::bind(&CSMTPClient::DoException, this, _1, _2));
#endif

            pCommand = CommandHandlers()->Add();
            pCommand->Command() = _T("TO");
            pCommand->Disconnect(false);
#if defined(_GLIBCXX_RELEASE) && (_GLIBCXX_RELEASE >= 9)
            pCommand->OnCommand([this](auto && ACommand) { DoTO(ACommand); });
            pCommand->OnException([this](auto && AConnection, auto && E) { DoException(AConnection, E); });
#else
            pCommand->OnCommand(std::bind(&CSMTPClient::DoTO, this, _1));
            pCommand->OnException(std::bind(&CSMTPClient::DoException, this, _1, _2));
#endif

            pCommand = CommandHandlers()->Add();
            pCommand->Command() = _T("DATA");
            pCommand->Disconnect(false);
#if defined(_GLIBCXX_RELEASE) && (_GLIBCXX_RELEASE >= 9)
            pCommand->OnCommand([this](auto && ACommand) { DoDATA(ACommand); });
            pCommand->OnException([this](auto && AConnection, auto && E) { DoException(AConnection, E); });
#else
            pCommand->OnCommand(std::bind(&CSMTPClient::DoDATA, this, _1));
            pCommand->OnException(std::bind(&CSMTPClient::DoException, this, _1, _2));
#endif

            pCommand = CommandHandlers()->Add();
            pCommand->Command() = _T("CONTENT");
            pCommand->Disconnect(false);
#if defined(_GLIBCXX_RELEASE) && (_GLIBCXX_RELEASE >= 9)
            pCommand->OnCommand([this](auto && ACommand) { DoCONTENT(ACommand); });
            pCommand->OnException([this](auto && AConnection, auto && E) { DoException(AConnection, E); });
#else
            pCommand->OnCommand(std::bind(&CSMTPClient::DoCONTENT, this, _1));
            pCommand->OnException(std::bind(&CSMTPClient::DoException, this, _1, _2));
#endif

            pCommand = CommandHandlers()->Add();
            pCommand->Command() = _T("QUIT");
            pCommand->Disconnect(true);
#if defined(_GLIBCXX_RELEASE) && (_GLIBCXX_RELEASE >= 9)
            pCommand->OnCommand([this](auto && ACommand) { DoQUIT(ACommand); });
            pCommand->OnException([this](auto && AConnection, auto && E) { DoException(AConnection, E); });
#else
            pCommand->OnCommand(std::bind(&CSMTPClient::DoQUIT, this, _1));
            pCommand->OnException(std::bind(&CSMTPClient::DoException, this, _1, _2));
#endif
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPClient::DoConnectStart(CIOHandlerSocket *AIOHandler, CPollEventHandler *AHandler) {
            auto pConnection = new CSMTPConnection(this);
            pConnection->IOHandler(AIOHandler);
            pConnection->AutoFree(true);
            AHandler->Binding(pConnection);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPClient::DoConnect(CPollEventHandler *AHandler) {
            auto pConnection = dynamic_cast<CSMTPConnection *> (AHandler->Binding());

            if (pConnection == nullptr) {
                AHandler->Stop();
                return;
            }

            try {
                auto pIOHandler = (CIOHandlerSocket *) pConnection->IOHandler();

                if (pIOHandler->Binding()->CheckConnection()) {
#if defined(_GLIBCXX_RELEASE) && (_GLIBCXX_RELEASE >= 9)
                    pConnection->OnRequest([this](auto && Sender) { DoRequest(Sender); });
                    pConnection->OnReply([this](auto && Sender) { DoReply(Sender); });
                    pConnection->OnDisconnected([this](auto && Sender) { DoDisconnected(Sender); });
#else
                    pConnection->OnRequest(std::bind(&CSMTPClient::DoRequest, this, _1));
                    pConnection->OnReply(std::bind(&CSMTPClient::DoReply, this, _1));
                    pConnection->OnDisconnected(std::bind(&CSMTPClient::DoDisconnected, this, _1));
#endif
                    DoConnected(pConnection);

                    AHandler->Start(etIO);
                }
            } catch (Delphi::Exception::Exception &E) {
                DoException(pConnection, E);
                AHandler->Stop();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPClient::DoRead(CPollEventHandler *AHandler) {
            auto pConnection = dynamic_cast<CSMTPConnection *> (AHandler->Binding());
            try {
                if (pConnection->ParseInput()) {
                    switch (pConnection->ConnectionStatus()) {
                        case csReplyError:
                            pConnection->Clear();
                            pConnection->Disconnect();
                            break;

                        case csReplyOk:
                            DoExecute(pConnection);
                            break;

                        default:
                            break;
                    }
                }
            } catch (Delphi::Exception::Exception &E) {
                DoException(pConnection, E);
                pConnection->Disconnect();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPClient::DoWrite(CPollEventHandler *AHandler) {
            auto pConnection = dynamic_cast<CSMTPConnection *> (AHandler->Binding());
            try {
                switch (pConnection->ConnectionStatus()) {
                    case Socket::csConnected:
                        DoRead(AHandler);
                        break;

                    case csRequestError:
                        pConnection->Clear();
                        pConnection->Disconnect();
                        break;

                    case csRequestReady:
                        if (pConnection->SendCommand()) {
                            pConnection->ConnectionStatus(csWaitReply);
                        } else {
                            pConnection->ConnectionStatus(csRequestSent);
                        }
                        break;

                    case csRequestSent:
                        if (pConnection->WriteAsync()) {
                            pConnection->ConnectionStatus(csWaitReply);
                        }
                        break;
                    default:
                        break;
                }
            } catch (Delphi::Exception::Exception &E) {
                DoException(pConnection, E);
                pConnection->Disconnect();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CSMTPClient::DoCommand(CTCPConnection *AConnection) {
            CCommandHandler *Handler;

            auto pConnection = dynamic_cast<CSMTPConnection *> (AConnection);
            const auto& command = pConnection->Command();

            bool Result = CommandHandlers()->Count() > 0;

            if (Result) {
                DoBeforeCommandHandler(AConnection, command.Command());
                try {
                    int Index;
                    for (Index = 0; Index < CommandHandlers()->Count(); ++Index) {
                        Handler = CommandHandlers()->Commands(Index);
                        if (Handler->Enabled()) {
                            if (Handler->Check(command.Command(), AConnection))
                                break;
                        }
                    }
                    if (Index == CommandHandlers()->Count())
                        DoNoCommandHandler(command.Command(), AConnection);
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

        int CSMTPClient::IndexOfMsgId(const CString &MsgId) const {
            int Index = 0;
            while (Index < m_Messages.Count() && m_Messages[Index].MsgId() != MsgId)
                Index++;
            if (Index == m_Messages.Count())
                Index = -1;
            return Index;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CSMTPClient::IndexOfMessageId(const CString &MessageId) const {
            int Index = 0;
            while (Index < m_Messages.Count() && m_Messages[Index].MessageId() != MessageId)
                Index++;
            if (Index == m_Messages.Count())
                Index = -1;
            return Index;
        }
        //--------------------------------------------------------------------------------------------------------------

        CMessage &CSMTPClient::NewMessage() {
            const auto index = m_Messages.Add(CMessage());
            return m_Messages[index];
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPClient::SendNext() {
            if (++m_MessageIndex < m_Messages.Count()) {
                m_ToIndex = 0;
#ifdef WITH_SSL
                UsedSSL(m_Config.Port() == 465);
#endif
                ConnectStart();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPClient::SendMail() {
#ifdef WITH_SSL
            UsedSSL(m_Config.Port() == 465);
#endif
            Active(m_Messages.Count() != 0);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPClient::DoCONNECT(CCommand *ACommand) {
            auto pConnection = dynamic_cast<CSMTPConnection *> (ACommand->Connection());
            auto& command = pConnection->Command();
            if (command.LastCode() == 220) {
                pConnection->NewCommand("HELLO", CString().Format("EHLO %s", pConnection->Socket()->Binding()->IP()));
            } else {
                command.ErrorMessage() = command.LastMessage();
                pConnection->CloseConnection(true);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPClient::DoHELLO(CCommand *ACommand) {
            auto pConnection = dynamic_cast<CSMTPConnection *> (ACommand->Connection());
            auto& command = pConnection->Command();
            if (command.LastCode() == 250) {
                int Index = 0;
                while (Index < command.Reply().Count() && command.Reply()[Index].Find("STARTTLS") == -1)
                    Index++;

                if (Index == command.Reply().Count()) {
                    CString LPlain;

                    LPlain.Write("\0", 1);
                    LPlain << m_Config.UserName();
                    LPlain.Write("\0", 1);
                    LPlain << m_Config.Password();

                    pConnection->NewCommand("AUTH", "AUTH PLAIN " + base64_encode(LPlain));
                } else {
#ifdef WITH_SSL
                    pConnection->NewCommand("STARTTLS");
#else
                    throw Delphi::Exception::ESocketError("WARNING: TLS/SSL not supported in this build (Need WITH_SSL: ON).");
#endif
                }
            } else {
                command.ErrorMessage() = command.LastMessage();
                pConnection->NewCommand("QUIT");
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPClient::DoSTARTTLS(CCommand *ACommand) {
            auto pConnection = dynamic_cast<CSMTPConnection *> (ACommand->Connection());
            auto& command = pConnection->Command();
            if (command.LastCode() == 220) {
#ifdef WITH_SSL
                auto Socket = pConnection->Socket()->Binding();
                if (Assigned(Socket)) {
                    Socket->SSLMethod(sslClient);
                    Socket->AllocateSSL();
                    Socket->ConnectSSL();
                }
#endif
                pConnection->NewCommand("HELLO", CString().Format("EHLO %s", pConnection->Socket()->Binding()->IP()));
            } else {
                command.ErrorMessage() = command.LastMessage();
                pConnection->NewCommand("QUIT");
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPClient::DoAUTH(CCommand *ACommand) {
            auto pConnection = dynamic_cast<CSMTPConnection *> (ACommand->Connection());
            auto& command = pConnection->Command();
            if (command.LastCode() == 235) {
                const auto &message = CurrentMessage();
                pConnection->NewCommand("FROM", CString().Format("MAIL FROM: <%s>", message.From().c_str()));
            } else {
                command.ErrorMessage() = command.LastMessage();
                pConnection->NewCommand("QUIT");
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPClient::DoFROM(CCommand *ACommand) {
            auto pConnection = dynamic_cast<CSMTPConnection *> (ACommand->Connection());
            auto& command = pConnection->Command();
            if (command.LastCode() == 250) {
                const auto &message = CurrentMessage();
                pConnection->NewCommand("TO", CString().Format("RCPT TO: <%s>", message.To()[m_ToIndex++].c_str()));
            } else {
                command.ErrorMessage() = command.LastMessage();
                pConnection->NewCommand("QUIT");
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPClient::DoTO(CCommand *ACommand) {
            auto pConnection = dynamic_cast<CSMTPConnection *> (ACommand->Connection());
            auto& command = pConnection->Command();
            if (command.LastCode() == 250 || command.LastCode() == 251) {
                const auto &message = CurrentMessage();
                if (m_ToIndex < message.To().Count()) {
                    pConnection->NewCommand("TO", CString().Format("RCPT TO: <%s>", message.To()[m_ToIndex++].c_str()));
                } else {
                    pConnection->NewCommand("DATA");
                }
            } else {
                command.ErrorMessage() = command.LastMessage();
                pConnection->NewCommand("QUIT");
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPClient::DoDATA(CCommand *ACommand) {
            auto pConnection = dynamic_cast<CSMTPConnection *> (ACommand->Connection());
            auto& command = pConnection->Command();
            if (command.LastCode() == 250 || command.LastCode() == 354) {
                const auto &message = CurrentMessage();
                pConnection->NewCommand("CONTENT", message.Body().Text() + "\r\n.");
            } else {
                command.ErrorMessage() = command.LastMessage();
                pConnection->NewCommand("QUIT");
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPClient::DoCONTENT(CCommand *ACommand) {
            auto pConnection = dynamic_cast<CSMTPConnection *> (ACommand->Connection());
            auto& command = pConnection->Command();
            if (command.LastCode() == 250) {
                auto &message = CurrentMessage();
                const auto& Line = command.LastMessage();
                size_t Pos = Line.Find("id=");
                message.MessageId() = command.LastMessage().SubString(Pos == CString::npos ? 4 : Pos + 3);
                message.Submitted(true);
            } else {
                command.ErrorMessage() = command.LastMessage();
            }
            pConnection->NewCommand("QUIT");
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSMTPClient::DoQUIT(CCommand *ACommand) {
            auto pConnection = dynamic_cast<CSMTPConnection *> (ACommand->Connection());
            const auto& command = pConnection->Command();
            auto &message = CurrentMessage();
            if (message.Submitted()) {
                message.Done();
            } else {
                message.Fail(command.ErrorMessage());
            }
            SendNext();
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
            return dynamic_cast<CSMTPCollectionItem *> (inherited::GetItem(Index));
        }
        //--------------------------------------------------------------------------------------------------------------

        CSMTPCollectionItem *CSMTPManager::Add(const CSMTPConfig &Config) {
            return new CSMTPCollectionItem(this, Config);
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CSMTPManager::InProgress(const CString &MsgId) {
            for (int i = 0; i < Count(); ++i) {
                auto Item = GetItem(i);
                if (Item->IndexOfMsgId(MsgId) != -1)
                    return true;
            }
            return false;
        }
    }
}
}
