/*++

Library name:

  libdelphi

Module Name:

  WSProtocol.hpp

Notices:

  Delphi classes for C++

Author:

  Copyright (c) Prepodobny Alen

  mailto: alienufo@inbox.ru
  mailto: ufocomp@gmail.com

--*/

#ifndef DELPHI_WSPROTOCOL_HPP
#define DELPHI_WSPROTOCOL_HPP
//----------------------------------------------------------------------------------------------------------------------

extern "C++" {

namespace Delphi {

    namespace WSProtocol {

        //--------------------------------------------------------------------------------------------------------------

        //-- CWSMessage ------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        enum CWSMessageType { mtOpen = 0, mtClose, mtCall, mtCallResult, mtCallError };

        typedef struct CWSMessage {
            CWSMessageType MessageTypeId = mtOpen;
            CString UniqueId {};
            CString Action {};
            int ErrorCode = -1;
            CString ErrorMessage {};
            CJSON Payload {};

            static CString MessageTypeIdToString(CWSMessageType Value) {
                switch (Value) {
                    case mtOpen:
                        return {"Open"};
                    case mtClose:
                        return {"Close"};
                    case mtCall:
                        return {"Call"};
                    case mtCallResult:
                        return {"CallResult"};
                    case mtCallError:
                        return {"CallError"};
                    default:
                        throw ExceptionFrm("Invalid \"MessageType\" value: %d", Value);
                }
            }

            size_t Size() const {
                return UniqueId.Size() + Action.Size() + ErrorMessage.Size();
            }

        } CWSMessage;

        //--------------------------------------------------------------------------------------------------------------

        //-- CWSProtocol -----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        // t: MessageTypeId
        // u: UniqueId
        // a: Action
        // c: ErrorCode
        // m: ErrorMessage
        // p: Payload

        class CWSProtocol {
        public:

            static bool Request(const CString &String, CWSMessage &Message);
            static void Response(const CWSMessage &Message, CString &String);

            static void PrepareResponse(const CWSMessage &Request, CWSMessage &Response);

            static void ResponseCall(const CString &UniqueId, const CString &Action, const CJSON &Payload, CString &Result);
            static void ResponseCallResult(const CString &UniqueId, const CJSON &Payload, CString &Result);
            static void ResponseCallError(const CString &UniqueId, int ErrorCode, const CString &ErrorMessage, const CJSON &Payload, CString &Result);

            static CWSMessage Call(const CString &UniqueId, const CString &Action, const CJSON &Payload);
            static CWSMessage CallResult(const CString &UniqueId, const CJSON &Payload);
            static CWSMessage CallError(const CString &UniqueId, int ErrorCode, const CString &ErrorMessage, const CJSON &Payload);

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CMessageHandler -------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CMessageManager;
        class CMessageHandler;
        //--------------------------------------------------------------------------------------------------------------

        typedef std::function<void (CMessageHandler *Handler, CHTTPServerConnection *Connection)> COnMessageHandlerEvent;
        //--------------------------------------------------------------------------------------------------------------

        class CMessageHandler: public CCollectionItem {
        private:

            CString m_UniqueId;
            CString m_Action;

            COnMessageHandlerEvent m_Handler;

        public:

            CMessageHandler(CMessageManager *AManager, COnMessageHandlerEvent && Handler);

            const CString &UniqueId() const { return m_UniqueId; }

            CString &Action() { return m_Action; }
            const CString &Action() const { return m_Action; }

            void Handler(CHTTPServerConnection *AConnection);

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CMessageManager -------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CSession;
        //--------------------------------------------------------------------------------------------------------------

        class CMessageManager: public CCollection {
            typedef CCollection inherited;

        private:

            CSession *m_pSession;

            CMessageHandler *Get(int Index) const;
            void Set(int Index, CMessageHandler *Value);

        public:

            explicit CMessageManager(CSession *ASession): CCollection(this), m_pSession(ASession) {

            }

            CMessageHandler *Add(COnMessageHandlerEvent &&Handler, const CString &Action, const CJSON &Payload);

            CMessageHandler *First() { return Get(0); };
            CMessageHandler *Last() { return Get(Count() - 1); };

            CMessageHandler *FindMessageById(const CString &Value);

            CMessageHandler *Handlers(int Index) const { return Get(Index); }
            void Handlers(int Index, CMessageHandler *Value) { Set(Index, Value); }

            CMessageHandler *operator[] (int Index) const override { return Handlers(Index); };

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CSession ---------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CSessionManager;
        //--------------------------------------------------------------------------------------------------------------

        class CSession: public CCollectionItem {
        private:

            CHTTPServerConnection *m_pConnection;

            CMessageManager m_Messages {this};

            CAuthorization m_Authorization;

            CString m_Identity;
            CString m_Session;
            CString m_Secret;
            CString m_IP;
            CString m_Agent;

            int m_UpdateCount;

            bool m_Authorized;

            void AddToConnection(CHTTPServerConnection *AConnection);
            void DeleteFromConnection(CHTTPServerConnection *AConnection);

        public:

            explicit CSession(CHTTPServerConnection *AConnection, CSessionManager *AManager);

            ~CSession() override;

            void SwitchConnection(CHTTPServerConnection *AConnection);

            void BeginUpdate() { m_UpdateCount++; }
            void EndUpdate() { m_UpdateCount--; }

            int UpdateCount() const { return m_UpdateCount; }

            CHTTPServerConnection *Connection() { return m_pConnection; };

            void Connection(CHTTPServerConnection *Value) { m_pConnection = Value; };

            CMessageManager &Messages() { return m_Messages; };
            const CMessageManager &Messages() const { return m_Messages; };

            CAuthorization &Authorization() { return m_Authorization; };
            const CAuthorization &Authorization() const { return m_Authorization; };

            CString &Identity() { return m_Identity; };
            const CString &Identity() const { return m_Identity; };

            CString &Session() { return m_Session; };
            const CString &Session() const { return m_Session; };

            CString &Secret() { return m_Secret; };
            const CString &Secret() const { return m_Secret; };

            CString &IP() { return m_IP; };
            const CString &IP() const { return m_IP; };

            CString &Agent() { return m_Agent; };
            const CString &Agent() const { return m_Agent; };

            bool Authorized() const { return m_Authorized; };
            void Authorized(bool Value) { m_Authorized = Value; };

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CSessionManager --------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CSessionManager: public CCollection {
            typedef CCollection inherited;

        private:

            CSession *Get(int Index) const;
            void Set(int Index, CSession *Value);

        public:

            CSessionManager();

            CSession *Add(CHTTPServerConnection *AConnection);

            CSession *First() { return Get(0); };
            CSession *Last() { return Get(Count() - 1); };

            CSession *Find(const CString &Session, const CString &Identity);
            CSession *FindByIP(const CString &Value);
            CSession *FindBySession(const CString &Value);
            CSession *FindByIdentity(const CString &Value);
            CSession *FindByConnection(CHTTPServerConnection *Value);

            CSession *Sessions(int Index) const { return Get(Index); }
            void Sessions(int Index, CSession *Value) { Set(Index, Value); }

            CSession *operator[] (int Index) const override { return Sessions(Index); };

        };

    }
}

using namespace Delphi::WSProtocol;
}

#endif // DELPHI_WSPROTOCOL_HPP
