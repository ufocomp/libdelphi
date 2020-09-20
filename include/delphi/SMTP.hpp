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

        //-- CSMTPConnection -------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CSMTPConnection: public CTCPClientConnection {
            typedef CTCPClientConnection inherited;

        private:

            CString m_Request;
            CString m_Reply;

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

            CString &Request() { return m_Request; }
            const CString &Request() const { return m_Request; }

            CString &Reply() { return m_Reply; };
            const CString &Reply() const { return m_Reply; };

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
