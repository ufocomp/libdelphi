/*++

Library name:

  libdelphi

Module Name:

  Postgres.hpp

Notices:

  Delphi classes for C++

Author:

  Copyright (c) Prepodobny Alen

  mailto: alienufo@inbox.ru
  mailto: ufocomp@gmail.com

--*/

#ifndef DELPHI_POSTGRES_HPP
#define DELPHI_POSTGRES_HPP
//----------------------------------------------------------------------------------------------------------------------

#include "postgresql/libpq-fe.h"
//----------------------------------------------------------------------------------------------------------------------

#define BOOLOID 16
#define INT8OID 20
#define INT2OID 21
#define INT4OID 23
#define JSONOID 114
#define NUMERICOID 1700
#define JSONBOID 3802
#define JSONPATHOID 4072
//----------------------------------------------------------------------------------------------------------------------

extern "C++" {

namespace Delphi {

    namespace Postgres {

        class CPQResult;
        //--------------------------------------------------------------------------------------------------------------

        typedef TList<CStringPairs> CPQueryResult;
        typedef TList<CPQueryResult> CPQueryResults;
        //--------------------------------------------------------------------------------------------------------------

        CString PQQuoteLiteral(const CString &String);
        void PQResultToJson(CPQResult *Result, CString& Json, bool DataArray = false, const CString &ObjectName = CString());

        //--------------------------------------------------------------------------------------------------------------

        //-- CPQConnInfo -----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CPQConnInfo: public CPersistent {
        private:

            PGPing m_Ping;

            mutable CString m_ConnInfo;

            mutable CStringList m_Parameters;

            CString m_ApplicationName;

            bool m_ExpandDBName;

            bool m_PingValid;

            void UpdateConnInfo() const;

            void AssignParameters(const CStringList& Strings);

        protected:

            PGPing GetPing();

            CString GetValue(LPCSTR Keyword) const;

            const CString& GetConnInfo() const;

        public:

            CPQConnInfo();

            explicit CPQConnInfo(LPCSTR AConnInfo);

            explicit CPQConnInfo(LPCSTR const *AKeywords, LPCSTR const *AValues, bool AExpandDBName = false);

            explicit CPQConnInfo(LPCSTR AHost, LPCSTR APort, LPCSTR ADataBase, LPCSTR AUserName, LPCSTR APassword,
                LPCSTR AOptions);

            const CString& ConnInfo() const { return GetConnInfo(); };

            PGPing Ping() { return GetPing(); };

            bool PingValid() const { return m_PingValid; };

            void PingValid(bool Value) { m_PingValid = Value; };

            void Add(LPCSTR Keyword, LPCSTR Value);

            void Clear();

            void Assign(const CPQConnInfo &Source);

            bool ExpandDBName() const { return m_ExpandDBName; };
            void ExpandDBName(bool Value) { m_ExpandDBName = Value; };

            void SetParameters(const CStringList& Strings) { AssignParameters(Strings); };

            CStringList& GetParameters() { return m_Parameters; };
            const CStringList& GetParameters() const { return m_Parameters; };

            CString& ApplicationName() { return m_ApplicationName; };
            const CString& ApplicationName() const { return m_ApplicationName; };

            CString Parameters(LPCSTR Keyword) const { return GetValue(Keyword); };

            CPQConnInfo& operator=(const CPQConnInfo& Info) {
                if (&Info != this) {
                    Assign(Info);
                }
                return *this;
            }

            CString operator[](LPCSTR Keyword) const { return GetValue(Keyword); }

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CPQConnection ---------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CPQConnection;
        //--------------------------------------------------------------------------------------------------------------

        typedef std::function<void (CPQConnection *AConnection, const PGresult *AResult)> COnPQConnectionReceiverEvent;
        typedef std::function<void (CPQConnection *AConnection, LPCSTR AMessage)> COnPQConnectionProcessorEvent;

        typedef std::function<void (CPQConnection *AConnection)> COnPQConnectionEvent;
        typedef std::function<void (CPQConnection *AConnection, CSocket AOldSocket)> COnPQConnectionChangeSocketEvent;

        typedef std::function<void (CPQConnection *AConnection, PGnotify *ANotify)> COnPQConnectionNotifyEvent;
        //--------------------------------------------------------------------------------------------------------------

        class CPQConnectionEvent: public CPollConnection {
            friend CPQConnection;

        private:

            COnPQConnectionChangeSocketEvent m_OnChangeSocket;

            COnPQConnectionReceiverEvent m_OnReceiver;
            COnPQConnectionProcessorEvent m_OnProcessor;

            COnPQConnectionNotifyEvent m_OnNotify;

            COnPQConnectionEvent m_OnError;
            COnPQConnectionEvent m_OnTimeOut;
            COnPQConnectionEvent m_OnStatus;
            COnPQConnectionEvent m_OnPollingStatus;

            COnPQConnectionEvent m_OnConnected;
            COnPQConnectionEvent m_OnDisconnected;

        protected:

            void DoChangeSocket(CPQConnection *AConnection, CSocket AOldSocket);

            void DoNotify(CPQConnection *AConnection, PGnotify *ANotify);

            void DoError(CPQConnection *AConnection);
            void DoTimeOut(CPQConnection *AConnection);
            void DoStatus(CPQConnection *AConnection);
            void DoPollingStatus(CPQConnection *AConnection);
            void DoConnected(CPQConnection *AConnection);
            void DoDisconnected(CPQConnection *AConnection);

        public:

            explicit CPQConnectionEvent(CPollManager *AManager);

            ~CPQConnectionEvent() override = default;

            const COnPQConnectionChangeSocketEvent &OnChangeSocket() const { return m_OnChangeSocket; }
            void OnChangeSocket(COnPQConnectionChangeSocketEvent && Value) { m_OnChangeSocket = Value; }

            const COnPQConnectionReceiverEvent &OnReceiver() const { return m_OnReceiver; }
            void OnReceiver(COnPQConnectionReceiverEvent && Value) { m_OnReceiver = Value; }

            const COnPQConnectionProcessorEvent &OnProcessor() const { return m_OnProcessor; }
            void OnProcessor(COnPQConnectionProcessorEvent && Value) { m_OnProcessor = Value; }

            const COnPQConnectionNotifyEvent &OnNotify() const { return m_OnNotify; }
            void OnNotify(COnPQConnectionNotifyEvent && Value) { m_OnNotify = Value; }

            const COnPQConnectionEvent &OnError() const { return m_OnError; }
            void OnError(COnPQConnectionEvent && Value) { m_OnError = Value; }

            const COnPQConnectionEvent &OnTimeOut() const { return m_OnTimeOut; }
            void OnTimeOut(COnPQConnectionEvent && Value) { m_OnTimeOut = Value; }

            const COnPQConnectionEvent &OnStatus() const { return m_OnStatus; }
            void OnStatus(COnPQConnectionEvent && Value) { m_OnStatus = Value; }

            const COnPQConnectionEvent &OnPollingStatus() const { return m_OnPollingStatus; }
            void OnPollingStatus(COnPQConnectionEvent && Value) { m_OnPollingStatus = Value; }

            const COnPQConnectionEvent &OnConnected() const { return m_OnConnected; }
            void OnConnected(COnPQConnectionEvent && Value) { m_OnConnected = Value; }

            const COnPQConnectionEvent &OnDisconnected() const { return m_OnDisconnected; }
            void OnDisconnected(COnPQConnectionEvent && Value) { m_OnDisconnected = Value; }

        };
        //--------------------------------------------------------------------------------------------------------------

        class CPQConnection: public CPQConnectionEvent {
        private:

            CPQConnInfo m_ConnInfo;

            PGconn *m_pHandle;

            ConnStatusType m_Status;

            PostgresPollingStatusType m_PollingStatus;

            CSocket m_Socket;

            bool m_TryConnect;
            bool m_Connected;

            CStringList m_Listeners;

            CDateTime m_AntiFreeze;

            bool GetConnected();

            void CheckSocket(bool AsyncMode = false);

            void SetReceiver();
            void SetProcessor();

        protected:

            void Finish();

            void CheckConnection();
            void CheckPollConnection();

            ConnStatusType GetStatus();

            PostgresPollingStatusType GetPollingStatus();

            bool CheckStatus(int const AIgnore[], int ACount);

        public:

            explicit CPQConnection(CPollManager *AManager);

            explicit CPQConnection(const CPQConnInfo &AConnInfo, CPollManager *AManager);

            ~CPQConnection() override;

            void Clear();

            void ConnInfo(const CPQConnInfo &AConnInfo) { m_ConnInfo = AConnInfo; }

            CPQConnInfo &ConnInfo() { return m_ConnInfo; }
            const CPQConnInfo &ConnInfo() const { return m_ConnInfo; }

            void CallReceiver(const PGresult *AResult);

            void CallProcessor(LPCSTR AMessage);

            LPCSTR GetErrorMessage();

            LPCSTR StatusString();

            LPCSTR PollingStatusString();

            ConnStatusType Status() { return GetStatus(); }

            PostgresPollingStatusType PollingStatus() { return GetPollingStatus(); }

            CSocket Socket() const { return m_Socket; }

            int PID();

            CSocket PQSocket();

            bool NeedsPassword();

            bool UsedPassword();

            void Connect();
            void Disconnect();

            void ConnectStart();

            void ConnectPoll();

            void ResetStart();

            void ResetPoll();

            void ConsumeInput();

            bool IsBusy();

            bool Flush();

            PGnotify *Notify();

            PGresult *GetResult();
            PGcancel *GetCancel();

            PGconn *Handle() { return m_pHandle; }

            void Close() override;

            bool Connected() { return GetConnected(); }

            CStringList &Listeners() { return m_Listeners; }
            const CStringList &Listeners() const { return m_Listeners; }

            CDateTime AntiFreeze() const { return m_AntiFreeze; }
            void AntiFreeze(CDateTime Value) { m_AntiFreeze = Value; }

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CPQPollConnection -----------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CPQQuery;
        //--------------------------------------------------------------------------------------------------------------

        enum CPollConnectionStatus { qsConnect, qsReset, qsReady, qsWait, qsError };
        //--------------------------------------------------------------------------------------------------------------

        class CPQPollConnection: public CPQConnection {
        private:

            CPQQuery *m_WorkQuery;

            CPollConnectionStatus m_ConnectionStatus;

        public:

            explicit CPQPollConnection(const CPQConnInfo &AConnInfo, CPollManager *AManager);

            void QueryStart(CPQQuery *AQuery);
            void QueryStop();

            CPollConnectionStatus ConnectionStatus() const { return m_ConnectionStatus; };
            void ConnectionStatus(CPollConnectionStatus Value) { m_ConnectionStatus = Value; };

            CPQQuery *WorkQuery() const { return m_WorkQuery; }

            bool CheckResult();
            int CheckNotify();

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CPQResult -------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CPQResult;
        //--------------------------------------------------------------------------------------------------------------

        typedef std::function<void (CPQResult *AResult)> COnPQResultEvent;
        typedef std::function<void (CPQResult *AResult, ExecStatusType AExecStatus)> COnPQExecResultEvent;
        typedef std::function<void (CPQResult *AResult, const Delphi::Exception::Exception &E)> COnPQResultExceptionEvent;
        //--------------------------------------------------------------------------------------------------------------

        class CPQResult: public CCollectionItem {
        private:

            PGresult *m_pHandle;

            CPQQuery *m_Query;

            ExecStatusType m_Status;

            CString m_VerboseErrorMessage;

            COnPQResultEvent m_OnStatus;

        protected:

            virtual void DoStatus();

        public:

            explicit CPQResult(CPQQuery *AQQuery, PGresult *AHandle);

            ~CPQResult() override;

            void Clear();

            PGresult *Handle() { return m_pHandle; };

            CPQQuery *Query() { return m_Query; };

            ExecStatusType ResultStatus();

            ExecStatusType ExecStatus() { return m_Status; };

            int nTuples();

            int nFields();

            LPCSTR fName(int ColumnNumber);

            int fNumber(LPCSTR ColumnName);

            Oid fTable(int ColumnNumber);

            int fTableCol(int ColumnNumber);

            int fFormat(int ColumnNumber);

            Oid fType(int ColumnNumber);

            int fMod(int ColumnNumber);

            int fSize(int ColumnNumber);

            int BinaryTuples();

            LPCSTR GetValue(int RowNumber, int ColumnNumber);

            int GetIsNull(int RowNumber, int ColumnNumber);

            int GetLength(int RowNumber, int ColumnNumber);

            int nParams();

            Oid ParamType(int ParamNumber);

            LPCSTR GetErrorMessage();

            LPCSTR StatusString();

            LPCSTR VerboseErrorMessage(PGVerbosity Verbosity = PQERRORS_DEFAULT,
                    PGContextVisibility Context = PQSHOW_CONTEXT_ERRORS);

            LPCSTR ErrorField(int FieldCode);

            const COnPQResultEvent &OnStatus() const { return m_OnStatus; }
            void OnStatus(COnPQResultEvent && Value) { m_OnStatus = Value; }

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CPQQuery --------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CPQQuery;
        //--------------------------------------------------------------------------------------------------------------

        typedef std::function<void (CPQQuery *AQuery)> COnPQQueryExecutedEvent;
        //--------------------------------------------------------------------------------------------------------------

        class CPQQuery: public CCollection {
            typedef CCollection inherited;

        private:

            CPQConnection *m_pConnection;

            CStringList m_SQL;

            CDateTime m_StartTime;

            COnPQQueryExecutedEvent m_OnSendQuery;
            COnPQQueryExecutedEvent m_OnExecuted;

            COnPQResultEvent m_OnResultStatus;
            COnPQExecResultEvent m_OnResult;

            CPQResult *GetResult(int Index);

        protected:

            virtual void DoExecuted();

            void DoSendQuery();
            void DoResultStatus(CPQResult *AResult);
            void DoResult(CPQResult *AResult, ExecStatusType AExecStatus);

            void SetConnection(CPQConnection *Value);

        public:

            CPQQuery();

            explicit CPQQuery(CPQConnection *AConnection);

            ~CPQQuery() override;

            CPQConnection *Connection() { return m_pConnection; };

            void Connection(CPQConnection *Value) { SetConnection(Value); };

            void GetResult();

            int ResultCount() { return inherited::Count(); };

            void SendQuery();
            bool CancelQuery(CString &Error);

            CDateTime StartTime() const { return m_StartTime; }

            CStringList& SQL() { return m_SQL; }
            const CStringList& SQL() const { return m_SQL; }

            CPQResult *Results(int Index) { return GetResult(Index); };

            const COnPQQueryExecutedEvent &OnExecuted() const { return m_OnExecuted; }
            void OnExecute(COnPQQueryExecutedEvent && Value) { m_OnExecuted = Value; }

            const COnPQQueryExecutedEvent &OnSendQuery() const { return m_OnSendQuery; }
            void OnSendQuery(COnPQQueryExecutedEvent && Value) { m_OnSendQuery = Value; }

            const COnPQResultEvent &OnResultStatus() const { return m_OnResultStatus; }
            void OnResultStatus(COnPQResultEvent && Value) { m_OnResultStatus = Value; }

            const COnPQExecResultEvent &OnResult() const { return m_OnResult; }
            void OnResult(COnPQExecResultEvent && Value) { m_OnResult = Value; }

        };

        //--------------------------------------------------------------------------------------------------------------

        //- CPQPollQuery -----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CPQConnectPoll;
        class CPQPollQuery;
        //--------------------------------------------------------------------------------------------------------------

        typedef std::function<void (CPQPollQuery *APollQuery)> COnPQPollQueryExecutedEvent;
        typedef std::function<void (CPQPollQuery *APollQuery, const Delphi::Exception::Exception &E)> COnPQPollQueryExceptionEvent;
        //--------------------------------------------------------------------------------------------------------------

        #define POLL_QUERY_START_OK (-1)
        #define POLL_QUERY_START_FAIL (-2)
        //--------------------------------------------------------------------------------------------------------------

        class CPQPollQuery: public CPollConnection, public CPQQuery {
        private:

            CPQConnectPoll *m_pConnectPoll;

            CStringList m_Data;

            COnPQPollQueryExecutedEvent m_OnExecuted;

            COnPQPollQueryExceptionEvent m_OnException;

        protected:

            void DoExecuted() override;
            void DoException(const Delphi::Exception::Exception &E);

        public:

            explicit CPQPollQuery(CPQConnectPoll *AConnectPoll);

            int Start();

            void Close() override;

            int AddToQueue();
            void RemoveFromQueue();

            CPQConnectPoll *ConnectPoll() const { return m_pConnectPoll; };

            CStringList &Data() { return m_Data; }
            const CStringList &Data() const { return m_Data; }

            const COnPQPollQueryExecutedEvent &OnPollExecuted() const { return m_OnExecuted; }
            void OnPollExecuted(COnPQPollQueryExecutedEvent && Value) { m_OnExecuted = Value; }

            const COnPQPollQueryExceptionEvent &OnException() const { return m_OnException; }
            void OnException(COnPQPollQueryExceptionEvent && Value) { m_OnException = Value; }
        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CPQPollQueryManager ---------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CPQPollQueryManager: public CPollManager {
        public:

            explicit CPQPollQueryManager(): CPollManager(this) {

            };

            ~CPQPollQueryManager() override = default;

            int QueryCount() const { return CPollManager::Count(); }

            CPQPollQuery *Queries(int Index) const { return (CPQPollQuery *) CPollManager::Items(Index); }

            CPQPollQuery *operator[] (int Index) const override { return Queries(Index); }

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CPQConnectPollEvent ---------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CPQClient;
        //--------------------------------------------------------------------------------------------------------------

        typedef std::function<void (CPQConnection *AConnection, const Delphi::Exception::Exception &E)> COnPQConnectionExceptionEvent;
        typedef std::function<void (CPQClient *AClient, const Delphi::Exception::Exception &E)> COnPQClientExceptionEvent;
        //--------------------------------------------------------------------------------------------------------------

        class CPQConnectPollEvent: public CObject {
        protected:

            COnPQConnectionReceiverEvent m_OnReceiver;
            COnPQConnectionProcessorEvent m_OnProcessor;

            COnPQConnectionNotifyEvent m_OnNotify;

            COnPQConnectionEvent m_OnError;
            COnPQConnectionEvent m_OnTimeOut;
            COnPQConnectionEvent m_OnStatus;
            COnPQConnectionEvent m_OnPollingStatus;

            COnPQConnectionExceptionEvent m_OnConnectException;
            COnPQClientExceptionEvent m_OnServerException;

            virtual void DoPQReceiver(CPQConnection *AConnection, const PGresult *AResult);
            virtual void DoPQProcessor(CPQConnection *AConnection, LPCSTR AMessage);

            virtual void DoPQNotify(CPQConnection *AConnection, PGnotify *ANotify);

            virtual void DoPQError(CPQConnection *AConnection);
            virtual void DoPQTimeOut(CPQConnection *AConnection);
            virtual void DoPQStatus(CPQConnection *AConnection);
            virtual void DoPQPollingStatus(CPQConnection *AConnection);

            virtual void DoPQConnectException(CPQConnection *AConnection, const Delphi::Exception::Exception &E);
            virtual void DoPQServerException(const Delphi::Exception::Exception &E) abstract;

        public:

            CPQConnectPollEvent();

            const COnPQConnectionReceiverEvent &OnReceiver() const { return m_OnReceiver; }
            void OnReceiver(COnPQConnectionReceiverEvent && Value) { m_OnReceiver = Value; }

            const COnPQConnectionProcessorEvent &OnProcessor() const { return m_OnProcessor; }
            void OnProcessor(COnPQConnectionProcessorEvent && Value) { m_OnProcessor = Value; }

            const COnPQConnectionNotifyEvent &OnNotify() const { return m_OnNotify; }
            void OnNotify(COnPQConnectionNotifyEvent && Value) { m_OnNotify = Value; }

            const COnPQConnectionEvent &OnError() const { return m_OnError; }
            void OnError(COnPQConnectionEvent && Value) { m_OnError = Value; }

            const COnPQConnectionEvent &OnTimeOut() const { return m_OnTimeOut; }
            void OnTimeOut(COnPQConnectionEvent && Value) { m_OnTimeOut = Value; }

            const COnPQConnectionEvent &OnStatus() const { return m_OnStatus; }
            void OnStatus(COnPQConnectionEvent && Value) { m_OnStatus = Value; }

            const COnPQConnectionEvent &OnPollingStatus() const { return m_OnPollingStatus; }
            void OnPollingStatus(COnPQConnectionEvent && Value) { m_OnPollingStatus = Value; }

            const COnPQConnectionExceptionEvent &OnConnectException() const { return m_OnConnectException; }
            void OnConnectException(COnPQConnectionExceptionEvent && Value) { m_OnConnectException = Value; }

            const COnPQClientExceptionEvent &OnServerException() const { return m_OnServerException; }
            void OnServerException(COnPQClientExceptionEvent && Value) { m_OnServerException = Value; }

        };

        //--------------------------------------------------------------------------------------------------------------

        //- CPQConnectPoll ---------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CPQConnectPoll: public CPQConnectPollEvent, public CEPollClient {
            friend CPQPollQuery;

        private:

            CQueue m_Queue;

            CEPollTimer *m_pTimer;

            bool m_Active;

            void CheckQueue();

            void UpdateTimer();

            void Fault(CPollEventHandler *AHandler);

            CPollEventHandler *NewEventHandler(CPQConnection *AConnection);

            CPQPollConnection *GetConnection(int Index) const;

            static CPQPollConnection *GetHandlerConnection(CPollEventHandler *AHandler);

            void OnChangeSocket(CPQConnection *AConnection, CSocket AOldSocket);

        protected:

            CPQConnInfo m_ConnInfo;
            CPollManager m_ConnectManager;
            CPQPollQueryManager m_QueryManager;

            int m_TimerInterval;

            size_t m_SizeMin;
            size_t m_SizeMax;

            void Start();

            void Stop(int Index);

            void StopAll();

            CPQPollConnection *GetReadyConnection();

            bool NewConnection();

            void PackConnections(CDateTime Now, CDateTime Period);

            void DoTimer(CPollEventHandler *AHandler);

            void DoTimeOut(CPollEventHandler *AHandler) override;
            void DoConnect(CPollEventHandler *AHandler) override;
            void DoWrite(CPollEventHandler *AHandler) override;
            void DoRead(CPollEventHandler *AHandler) override;
            void DoError(CPollEventHandler *AHandler) override;

            void SetActive(bool Value);
            void SetTimerInterval(int Value);

        public:

            explicit CPQConnectPoll(size_t ASizeMin, size_t ASizeMax);

            explicit CPQConnectPoll(const CPQConnInfo &AConnInfo, size_t ASizeMin = 5, size_t ASizeMax = 10);

            ~CPQConnectPoll() override = default;

            void ConnInfo(const CPQConnInfo &AConnInfo) { m_ConnInfo = AConnInfo; };

            CPQConnInfo &ConnInfo() { return m_ConnInfo; };
            const CPQConnInfo &ConnInfo() const { return m_ConnInfo; };

            bool Active() const { return m_Active; };
            void Active(bool Value) { SetActive(Value); };

            int AddToQueue(CPQPollQuery *AQuery);
            void RemoveFromQueue(CPQPollQuery *AQuery);

            const CQueue &Queue() const { return m_Queue; }

            const CPollManager &ConnectManager() const { return m_ConnectManager; }

            const CPQPollQueryManager &QueryManager() const { return m_QueryManager; }
            CPQPollQueryManager *ptrQueryManager() { return &m_QueryManager; }

            size_t SizeMin() const { return m_SizeMin; }
            void SizeMin(size_t Value) { m_SizeMin = Value; }

            size_t SizeMax() const { return m_SizeMax; }
            void SizeMax(size_t Value) { m_SizeMax = Value; }

            CPQPollConnection *Connections(int Index) const { return GetConnection(Index); }

        };

        //--------------------------------------------------------------------------------------------------------------

        //- CPQClient --------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CPQClient: public CPQConnectPoll {
        protected:

            bool DoCommand(CTCPConnection *AConnection) override;
            bool DoExecute(CTCPConnection *AConnection) override;

            void DoPQServerException(const Delphi::Exception::Exception &E) override;

        public:

            explicit CPQClient(size_t ASizeMin = 5, size_t ASizeMax = 10): CPQConnectPoll(ASizeMin, ASizeMax) {};

            ~CPQClient() override = default;

            CPQPollQuery *GetQuery();

            bool CheckListen(const CString &Listen);

            CPQPollQuery *FindQueryByConnection(CPollConnection *APollConnection) const;

        };
    }
}

using namespace Delphi::Postgres;
}

#endif //DELPHI_POSTGRES_HPP
