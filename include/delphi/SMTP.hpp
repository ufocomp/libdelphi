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

            CLocation &Location() { return m_Location; }
            const CLocation &Location() const { return m_Location; }

            CString &Host() { return m_Location.hostname; };
            const CString &Host() const { return m_Location.hostname; };

            int Port() const { return m_Location.port; };

            CString &UserName() { return m_UserName; };
            const CString &UserName() const { return m_UserName; };

            CString &Password() { return m_Password; };
            const CString &Password() const { return m_Password; };

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
            CString m_Data;

            int m_LastCode;

            CString m_LastMessage;
            CString m_ErrorMessage;

            CStringList m_Reply;

        public:

            CSMTPCommand();

            CSMTPCommand(const CSMTPCommand &Value);

            explicit CSMTPCommand(const CString &Command, const CString &Data = CString());

            ~CSMTPCommand() override = default;

            void Assign(const CSMTPCommand &Value);

            void Clear();

            void ToBuffers(CMemoryStream &AStream) const;

            CString &Command() { return m_Command; };
            const CString &Command() const { return m_Command; };

            CString &Data() { return m_Data; }
            const CString &Data() const { return m_Data; }

            int LastCode() const { return m_LastCode; };
            void LastCode(int Value) { m_LastCode = Value; };

            CString &LastMessage() { return m_LastMessage; };
            const CString &LastMessage() const { return m_LastMessage; };

            CString &ErrorMessage() { return m_ErrorMessage; };
            const CString &ErrorMessage() const { return m_ErrorMessage; };

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

            CString m_Code;
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

        typedef TList<CSMTPCommand> CSMTPCommands;
        //--------------------------------------------------------------------------------------------------------------

        class CSMTPConnection: public CTCPClientConnection {
            typedef CTCPClientConnection inherited;

        private:

            CSMTPCommand m_Command;

        public:

            explicit CSMTPConnection(CPollSocketClient *AClient);

            ~CSMTPConnection() override;

            void Clear() override;

            bool ParseInput();
            bool SendCommand();

            void NewCommand(const CString &Command, const CString &Data = CString());

            CSMTPCommand &Command() { return m_Command; }
            const CSMTPCommand &Command() const { return m_Command; }

        }; // CHTTPServerConnection

        //--------------------------------------------------------------------------------------------------------------

        //-- CSMTPClient -----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CSMTPClient: public CAsyncClient {
        private:

            CSMTPConfig m_Config;

            CMessages m_Messages;

            int m_MessageIndex;
            int m_ToIndex;

            CNotifyEvent m_OnRequest;
            CNotifyEvent m_OnReply;

            void InitializeCommandHandlers() override;

            void SendNext();

        protected:

            void DoConnectStart(CIOHandlerSocket *AIOHandler, CPollEventHandler *AHandler) override;

            void DoConnect(CPollEventHandler *AHandler) override;

            void DoRead(CPollEventHandler *AHandler) override;

            void DoWrite(CPollEventHandler *AHandler) override;

            bool DoCommand(CTCPConnection *AConnection) override;

            bool DoExecute(CTCPConnection *AConnection) override;

            void DoCONNECT(CCommand *ACommand);
            void DoHELLO(CCommand *ACommand);
            void DoSTARTTLS(CCommand *ACommand);
            void DoAUTH(CCommand *ACommand);
            void DoFROM(CCommand *ACommand);
            void DoTO(CCommand *ACommand);
            void DoDATA(CCommand *ACommand);
            void DoCONTENT(CCommand *ACommand);
            void DoQUIT(CCommand *ACommand);

            void DoRequest(CObject *Sender);
            void DoReply(CObject *Sender);

        public:

            CSMTPClient();

            explicit CSMTPClient(const CSMTPConfig &Config);

            ~CSMTPClient() override = default;

            CMessage &NewMessage();

            void SendMail();

            CSMTPConfig &Config() { return m_Config; }
            const CSMTPConfig &Config() const { return m_Config; }

            CMessages &Messages() { return m_Messages; }
            const CMessages &Messages() const { return m_Messages; }

            CMessage &CurrentMessage() { return m_Messages[m_MessageIndex]; }
            const CMessage &CurrentMessage() const { return m_Messages[m_MessageIndex]; }

            int IndexOfMsgId(const CString &MsgId) const;
            int IndexOfMessageId(const CString &MessageId) const;

            const CNotifyEvent &OnRequest() { return m_OnRequest; }
            void OnRequest(CNotifyEvent && Value) { m_OnRequest = Value; }

            const CNotifyEvent &OnReply() { return m_OnReply; }
            void OnReply(CNotifyEvent && Value) { m_OnReply = Value; }

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

            bool InProgress(const CString &MsgId);

            CSMTPCollectionItem *Items(int Index) const override { return GetItem(Index); };

            CSMTPCollectionItem *operator[] (int Index) const override { return Items(Index); };

        };

    }
}

using namespace Delphi::SMTP;
}

#endif //DELPHI_SMTP_HPP
