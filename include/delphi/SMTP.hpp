/*++

Library name:

  libdelphi

Module Name:

  SMTP.hpp

Notices:

  SMTP protocol

Author:

  Copyright (c) Prepodobny Alen

  mailto: alienufo@inbox.ru
  mailto: ufocomp@gmail.com

--*/

#ifndef DELPHI_SMTP_HPP
#define DELPHI_SMTP_HPP
//----------------------------------------------------------------------------------------------------------------------

extern "C++" {

namespace Delphi {

    namespace SMTP {

        class CSMTPMessage;

        typedef std::function<void (CSMTPMessage *Message)> COnSMTPMessageDoneEvent;
        typedef std::function<void (CSMTPMessage *Message)> COnSMTPMessageErrorEvent;

        //--------------------------------------------------------------------------------------------------------------

        //-- CSMTPConfig -----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CSMTPConfig: public CObject {
        private:

            CLocation m_Location;

            CString m_UserName;
            CString m_Password;

        public:

            CSMTPConfig();

            CSMTPConfig(const CSMTPConfig &Value);

            explicit CSMTPConfig(const CString &Host);
            explicit CSMTPConfig(const CString &Host, const CString &UserName, const CString &Password);

            ~CSMTPConfig() override = default;

            void Assign(const CSMTPConfig& Value);

            void Clear();

            CString &Host() { return m_Location.hostname; };
            const CString &Host() const { return m_Location.hostname; };

            int Port() const { return m_Location.port; };

            CString &UserName() { return m_UserName; };
            const CString &UserName() const { return m_UserName; };

            CString &Password() { return m_Password; };
            const CString &Password() const { return m_Password; };

            CLocation &Location() { return m_Location; }
            const CLocation &Location() const { return m_Location; }

            CSMTPConfig& operator= (const CSMTPConfig &Value) {
                if (this != &Value) {
                    Assign(Value);
                }
                return *this;
            };

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CSMTPCommand ----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CSMTPCommand: public CObject {
        private:

            CString m_Command;

            CStringList m_Params;

            int m_ErrorCode;

            CStringList m_Reply;

        public:

            CSMTPCommand();

            CSMTPCommand(const CSMTPCommand &Value);

            ~CSMTPCommand() override = default;

            void Assign(const CSMTPCommand &Value);

            void Clear();

            CString &Command() { return m_Command; };
            const CString &Command() const { return m_Command; };

            CStringList &Params() { return m_Params; }
            const CStringList &Location() const { return m_Params; }

            int ErrorCode() const { return m_ErrorCode; };
            void ErrorCode(int Value) { m_ErrorCode = Value; };

            CStringList &Reply() { return m_Reply; };
            const CStringList &Reply() const { return m_Reply; };

            CSMTPCommand& operator= (const CSMTPCommand &Value) {
                if (this != &Value) {
                    Assign(Value);
                }
                return *this;
            };

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CSMTPReplyParser ------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        /// Parser for incoming SMTP requests.
        class CSMTPReplyParser {
        private:

            enum CParserState {
                smtp_code,
                smtp_code_hyphen,
                smtp_code_space,
                smtp_text,
                smtp_newline
            } m_State;

            LPCTSTR m_pBegin;
            LPCTSTR m_pEnd;

            CString m_Line;

            int m_Result;

        public:

            CSMTPReplyParser(LPCTSTR ABegin, LPCTSTR AEnd);

            /// Check if a byte is an SMTP character.
            static bool IsChar(int c);

            /// Check if a byte is an SMTP control character.
            static bool IsCtl(int c);

            /// Check if a byte is a digit.
            static bool IsDigit(int c);

            /// Handle the next character of input.
            int Consume(CSMTPCommand &Command, char AInput);

            /// Parse some data. The int return value is "1" when a complete request
            /// has been parsed, "0" if the data is invalid, "-1" when more
            /// data is required.
            int Parse(CSMTPCommand &Command);
        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CSMTPConnection -------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CSMTPConnection: public CTCPClientConnection {
            typedef CTCPClientConnection inherited;

        private:

            CSMTPCommand m_Command;

            CConnectionStatus m_ConnectionStatus;

            bool m_CloseConnection;

            CNotifyEvent m_OnRequest;
            CNotifyEvent m_OnReply;

        protected:

            void DoRequest();
            void DoReply();

        public:

            explicit CSMTPConnection(CPollSocketClient *AClient);

            ~CSMTPConnection() override;

            virtual void Clear();

            bool ParseInput();

            CSMTPCommand &Command() { return m_Command; }
            const CSMTPCommand &Command() const { return m_Command; }

            bool CloseConnection() const { return m_CloseConnection; };
            void CloseConnection(bool Value) { m_CloseConnection = Value; };

            CConnectionStatus ConnectionStatus() const { return m_ConnectionStatus; };
            void ConnectionStatus(CConnectionStatus Value) { m_ConnectionStatus = Value; };

            void SendRequest(bool ASendNow = false);

            const CNotifyEvent &OnRequest() const { return m_OnRequest; }
            void OnRequest(CNotifyEvent && Value) { m_OnRequest = Value; }

            const CNotifyEvent &OnReply() const { return m_OnReply; }
            void OnReply(CNotifyEvent && Value) { m_OnReply = Value; }

        }; // CHTTPServerConnection

        //--------------------------------------------------------------------------------------------------------------

        //-- CSMTPMessage ----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CSMTPClient;
        //--------------------------------------------------------------------------------------------------------------

        class CSMTPMessage: public CSMTPConnection {
        private:

            CString m_MessageId;

            CString m_From;
            CStringList m_To;

            CString m_Subject;
            CStringList m_Body;

            COnSMTPMessageDoneEvent m_OnDone;
            COnSMTPMessageErrorEvent m_OnError;

        protected:

            void DoDone();
            void DoError();

        public:

            explicit CSMTPMessage(CSMTPClient *AClient);

            ~CSMTPMessage() override = default;

            void Assign(const CSMTPMessage& Message);

            void Clear() override;

            CString &MessageId() { return m_MessageId; };
            const CString &MessageId() const { return m_MessageId; };

            CString &From() { return m_From; };
            const CString &From() const { return m_From; };

            CStringList &To() { return m_To; };
            const CStringList &To() const { return m_To; };

            CString &Subject() { return m_Subject; };
            const CString &Subject() const { return m_Subject; };

            CStringList &Body() { return m_Body; };
            const CStringList &Body() const { return m_Body; };

            const COnSMTPMessageDoneEvent &OnDone() const { return m_OnDone; }
            void OnDone(COnSMTPMessageDoneEvent && Value) { m_OnDone = Value; }

            const COnSMTPMessageErrorEvent &OnError() const { return m_OnError; }
            void OnError(COnSMTPMessageErrorEvent && Value) { m_OnError = Value; }

            CSMTPMessage& operator= (const CSMTPMessage &Value) {
                if (this != &Value) {
                    Assign(Value);
                }
                return *this;
            };

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CSMTPClient -----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CSMTPClient: public CAsyncClient {
        private:

            CSMTPConfig m_Config;

        protected:

            void DoConnectStart(CIOHandlerSocket *AIOHandler, CPollEventHandler *AHandler) override;

            void DoConnect(CPollEventHandler *AHandler) override;

            void DoRead(CPollEventHandler *AHandler) override;

            void DoWrite(CPollEventHandler *AHandler) override;

            bool DoCommand(CTCPConnection *AConnection) override;

            bool DoExecute(CTCPConnection *AConnection) override;

        public:

            CSMTPClient();

            explicit CSMTPClient(const CSMTPConfig &Config);

            ~CSMTPClient() override = default;

            CSMTPMessage *NewMessage();

            void SendMail();

            CSMTPConfig &Config() { return m_Config; }
            const CSMTPConfig &Config() const { return m_Config; }

            int IndexOfMessageId(const CString &MessageId) const;

            CSMTPMessage *Items(int Index) const override { return (CSMTPMessage *) GetItem(Index); }
            void Items(int Index, CSMTPMessage *Value) { SetItem(Index, Value); }

            CSMTPMessage *operator[] (int Index) const override { return Items(Index); };

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CSMTPCollectionItem ---------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CSMTPManager;
        //--------------------------------------------------------------------------------------------------------------

        class CSMTPCollectionItem: public CCollectionItem, public CSMTPClient {
        public:

            explicit CSMTPCollectionItem(CSMTPManager *AManager);

            explicit CSMTPCollectionItem(CSMTPManager *AManager, const CSMTPConfig &Config);

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CSMTPManager ----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CSMTPManager: public CCollection {
            typedef CCollection inherited;

        protected:

            CSMTPCollectionItem *GetItem(int Index) const override;

        public:

            CSMTPManager(): CCollection(this) {

            };

            ~CSMTPManager() override = default;

            CSMTPCollectionItem *Add(const CSMTPConfig &Config);

            void CleanUp();

            CSMTPCollectionItem *Items(int Index) const override { return GetItem(Index); };

            CSMTPCollectionItem *operator[] (int Index) const override { return Items(Index); };

        };

    }
}

using namespace Delphi::SMTP;
}

#endif //DELPHI_SMTP_HPP
