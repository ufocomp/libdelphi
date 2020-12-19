/*++

Library name:

  libdelphi

Module Name:

  Postgres.cpp

Notices:

  Delphi classes for C++

Author:

  Copyright (c) Prepodobny Alen

  mailto: alienufo@inbox.ru
  mailto: ufocomp@gmail.com

--*/

#ifdef WITH_POSTGRESQL
#include "delphi.hpp"
#include "delphi/Postgres.hpp"
//----------------------------------------------------------------------------------------------------------------------

void OnNoticeReceiver(void *AData, const PGresult *AResult) {
    if (Assigned(AData)) {
        auto pConnection = (CPQConnection *) AData;
        pConnection->CallReceiver(AResult);
    }
}
//----------------------------------------------------------------------------------------------------------------------

void OnNoticeProcessor(void *AData, LPCSTR AMessage) {
    if (Assigned(AData)) {
        auto pConnection = (CPQConnection *) AData;
        pConnection->CallProcessor(AMessage);
    }
}
//----------------------------------------------------------------------------------------------------------------------

extern "C++" {

namespace Delphi {

    namespace Postgres {

        CString PQQuoteLiteral(const CString &String) {

            if (String.IsEmpty())
                return "null";

            CString Result;
            TCHAR ch;

            Result = "E'";

            for (size_t Index = 0; Index < String.Size(); Index++) {
                ch = String.at(Index);
                if ((ch == '\'') || (ch == '\\'))
                    Result.Append('\\');
                Result.Append(ch);
            }

            Result << "'";

            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        void PQResultToJson(CPQResult *Result, CString &Json, bool DataArray, const CString &ObjectName) {

            LPCSTR Value = nullptr;
            Oid Type;

            const auto ResultObject = !ObjectName.IsEmpty();

            DataArray = ResultObject || DataArray || Result->nTuples() > 1;

            const auto EmptyData = DataArray ? _T("[]") : _T("{}");

            if (Result->nTuples() == 0) {
                Json = ResultObject ? CString().Format("{\"%s\": %s}", ObjectName.c_str(), EmptyData) : EmptyData;
                return;
            }

            if (ResultObject)
                Json.Format("{\"%s\": ", ObjectName.c_str());

            if (DataArray)
                Json += _T("[");

            for (int Row = 0; Row < Result->nTuples(); ++Row) {
                if (Row > 0) {
                    Json += ", ";
                }

                Json += "{";

                for (int Col = 0; Col < Result->nFields(); ++Col) {
                    if (Col > 0) {
                        Json += ", ";
                    }

                    Json += "\"";
                    Json += Result->fName(Col);
                    Json += "\"";
                    Json += ": ";

                    Value = Result->GetValue(Row, Col);
                    Type = Result->fType(Col);

                    if (Result->GetIsNull(Row, Col)) {
                        Json += _T("null");
                    } else if (Type == BOOLOID) {
                        if (SameText(Value, _T("t"))) {
                            Json += _T("true");
                        } else if (SameText(Value, _T("f"))) {
                            Json += _T("false");
                        }
                    } else if (((Type == INT2OID) || (Type == INT4OID) || (Type == INT8OID)) ||
                               ((Type == JSONOID) || (Type == JSONBOID)) ||
                               ((Type == NUMERICOID) && ((strchr(Value, '.') == nullptr) && (strchr(Value, ',') == nullptr)))) {
                        Json += Value;
                    } else {
                        Json += "\"";
                        Json += Delphi::Json::EncodeJsonString(Value);
                        Json += "\"";
                    }
                }

                Json += "}";
            }

            if (DataArray) {
                Json += "]";
            }

            if (ResultObject) {
                Json += "}";
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CPQConnInfo -----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CPQConnInfo::CPQConnInfo(): CPersistent(this) {
            m_Ping = PQPING_NO_ATTEMPT;
            m_PingValid = false;
            m_ExpandDBName = false;
            m_ApplicationName = _T("'") DELPHI_LIB_DESCRIPTION _T("'");
        }
        //--------------------------------------------------------------------------------------------------------------

        CPQConnInfo::CPQConnInfo(LPCSTR AConnInfo): CPQConnInfo() {
            m_ConnInfo = AConnInfo;
        }
        //--------------------------------------------------------------------------------------------------------------

        CPQConnInfo::CPQConnInfo(LPCSTR const *AKeywords, LPCSTR const *AValues, bool AExpandDBName): CPQConnInfo() {
            m_ExpandDBName = AExpandDBName;
            for (int i = 0; AKeywords[i]; ++i) {
                m_Parameters.AddPair(AKeywords[i], AValues[i]);
            }
            UpdateConnInfo();
        }
        //--------------------------------------------------------------------------------------------------------------

        CPQConnInfo::CPQConnInfo(LPCSTR AHost, LPCSTR APort, LPCSTR ADataBase, LPCSTR AUserName, LPCSTR APassword,
                                     LPCSTR AOptions): CPQConnInfo() {

            m_Parameters.AddPair(_T("host"), AHost);
            m_Parameters.AddPair(_T("port"), APort);
            m_Parameters.AddPair(_T("dbname"), ADataBase);
            m_Parameters.AddPair(_T("user"), AUserName);
            m_Parameters.AddPair(_T("password"), APassword);
            m_Parameters.AddPair(_T("options"), AOptions);

            UpdateConnInfo();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnInfo::UpdateConnInfo() const {
            m_ConnInfo.Clear();

            if (GetValue(_T("host")).IsEmpty())
                m_Parameters.AddPair(_T("host"), _T("localhost"));

            if (GetValue(_T("port")).IsEmpty())
                m_Parameters.AddPair(_T("port"), _T("5432"));

            if (GetValue(_T("dbname")).IsEmpty())
                m_Parameters.AddPair(_T("dbname"), _T("postgres"));

            if (GetValue(_T("user")).IsEmpty())
                m_Parameters.AddPair(_T("user"), _T("postgres"));

            m_ConnInfo = m_Parameters[0];
            for (int i = 1; i < m_Parameters.Count(); ++i)
                m_ConnInfo += _T(" ") + m_Parameters[i];

            const CString &ApplicationName = m_Parameters.Values(_T("application_name"));
            if (ApplicationName.IsEmpty()) {
                if (!m_ApplicationName.IsEmpty())
                    m_ConnInfo += _T(" application_name=") + m_ApplicationName;
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        const CString &CPQConnInfo::GetConnInfo() const {
            if (m_ConnInfo.IsEmpty())
                UpdateConnInfo();
            return m_ConnInfo;
        }
        //--------------------------------------------------------------------------------------------------------------

        PGPing CPQConnInfo::GetPing() {
            if (!m_PingValid) {
                m_Ping = PQping(GetConnInfo().c_str());
            }
            return m_Ping;
        }
        //--------------------------------------------------------------------------------------------------------------

        CString CPQConnInfo::GetValue(LPCSTR AKeyword) const {
            return m_Parameters.Values(AKeyword);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnInfo::Clear() {
            m_ConnInfo.Clear();
            m_Parameters.Clear();
            m_ExpandDBName = false;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnInfo::Add(LPCSTR Keyword, LPCSTR Value) {
            m_Parameters.AddPair(Keyword, Value);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnInfo::Assign(const CPQConnInfo &Source) {
            m_ApplicationName = Source.m_ApplicationName;
            m_ConnInfo = Source.m_ConnInfo;
            m_Parameters = Source.m_Parameters;
            m_ExpandDBName = Source.m_ExpandDBName;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnInfo::AssignParameters(const CStringList &Strings) {
            Clear();
            if (Strings.Count() > 0) {
                m_Parameters = Strings;
                m_ExpandDBName = GetValue(_T("dbname")).SubString(0, 11) == _T("postgresql:");
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CPQConnectionEvent ----------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CPQConnectionEvent::CPQConnectionEvent(CPollManager *AManager): CPollConnection(this, AManager) {
            m_OnReceiver = nullptr;
            m_OnProcessor = nullptr;

            m_OnChangeSocket = nullptr;

            m_OnNotify = nullptr;

            m_OnError = nullptr;
            m_OnStatus = nullptr;
            m_OnPollingStatus = nullptr;

            m_OnConnected = nullptr;
            m_OnDisconnected = nullptr;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnectionEvent::DoChangeSocket(CPQConnection *APQConnection, CSocket AOldSocket) {
            if (m_OnChangeSocket != nullptr) {
                m_OnChangeSocket(APQConnection, AOldSocket);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnectionEvent::DoNotify(CPQConnection *APQConnection, PGnotify *ANotify) {
            if (m_OnNotify != nullptr) {
                m_OnNotify(APQConnection, ANotify);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnectionEvent::DoError(CPQConnection *APQConnection) {
            if (m_OnError != nullptr) {
                m_OnError(APQConnection);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnectionEvent::DoStatus(CPQConnection *APQConnection) {
            if (m_OnStatus != nullptr) {
                m_OnStatus(APQConnection);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnectionEvent::DoPollingStatus(CPQConnection *APQConnection) {
            if (m_OnPollingStatus != nullptr) {
                m_OnPollingStatus(APQConnection);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnectionEvent::DoConnected(CPQConnection *APQConnection) {
            if (m_OnConnected != nullptr) {
                m_OnConnected(APQConnection);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnectionEvent::DoDisconnected(CPQConnection *APQConnection) {
            if (m_OnDisconnected != nullptr) {
                m_OnDisconnected(APQConnection);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        //-- CPQConnection ---------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CPQConnection::CPQConnection(CPollManager *AManager): CPQConnectionEvent(AManager) {
            m_pHandle = nullptr;
            m_Socket = INVALID_SOCKET;
            m_TryConnect = false;
            m_Connected = false;
            m_Listener = false;
            m_Status = CONNECTION_BAD;
            m_PollingStatus = PGRES_POLLING_WRITING;
            m_AntiFreeze = Now();
        }
        //--------------------------------------------------------------------------------------------------------------

        CPQConnection::CPQConnection(const CPQConnInfo &AConnInfo, CPollManager *AManager): CPQConnection(AManager) {
            m_ConnInfo = AConnInfo;
        }
        //--------------------------------------------------------------------------------------------------------------

        CPQConnection::~CPQConnection() {
            CPQConnection::Disconnect();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnection::Clear() {
            m_pHandle = nullptr;
            m_Socket = INVALID_SOCKET;
            m_TryConnect = false;
            m_Connected = false;
            m_Status = CONNECTION_BAD;
            m_PollingStatus = PGRES_POLLING_WRITING;
            m_AntiFreeze = Now();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnection::Finish() {
            if (m_TryConnect) {
                PQfinish(m_pHandle);
                Clear();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnection::SetReceiver() {
            if (m_OnReceiver != nullptr)
                PQsetNoticeReceiver(m_pHandle, OnNoticeReceiver, this);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnection::SetProcessor() {
            if (m_OnProcessor != nullptr)
                PQsetNoticeProcessor(m_pHandle, OnNoticeProcessor, this);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnection::SetListener(bool Value) {
            if (m_Listener != Value) {
                m_Listener = Value;
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnection::CheckSocket(bool AsyncMode) {
            CSocket oldSocket, newSocket;
            newSocket = PQSocket();
            if (m_Socket != newSocket) {
                oldSocket = m_Socket;
                m_Socket = newSocket;

                if (newSocket != SOCKET_ERROR && AsyncMode) {
                    if (PQsetnonblocking(m_pHandle, 1) == -1) {
                        throw EDBConnectionError(_T("[%d] Set non blocking connection failed: %s"), m_Socket, GetErrorMessage());
                    }
                }

                DoChangeSocket(this, oldSocket);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        LPCSTR CPQConnection::GetErrorMessage() {
            return PQerrorMessage(m_pHandle);
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CPQConnection::CheckStatus(int const AIgnore[], int ACount) {
            m_Status = PQstatus(m_pHandle);
            DoStatus(this);
            for (int i = 0; i < ACount; ++i)
                if (m_Status == AIgnore[i])
                    return true;
            return false;
        }
        //--------------------------------------------------------------------------------------------------------------

        ConnStatusType CPQConnection::GetStatus() {
            int Ignore[] = {0};
            CheckStatus(Ignore, 0);
            return m_Status;
        }
        //--------------------------------------------------------------------------------------------------------------

        PostgresPollingStatusType CPQConnection::GetPollingStatus() {
            return m_PollingStatus;
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CPQConnection::GetConnected() {
            return GetStatus() == CONNECTION_OK;
        }
        //--------------------------------------------------------------------------------------------------------------

        LPCSTR CPQConnection::StatusString() {
            switch (m_Status) {
                case CONNECTION_OK:
                    return "Connection success.";
                case CONNECTION_BAD:
                    return "Connection failed.";
                case CONNECTION_STARTED:
                    return "Waiting for connection to be made.";
                case CONNECTION_MADE:
                    return "Connection OK; waiting to send.";
                case CONNECTION_AWAITING_RESPONSE:
                    return "Waiting for a response from the server.";
                case CONNECTION_AUTH_OK:
                    return "Received authentication; waiting for backend start-up to finish.";
                case CONNECTION_SETENV:
                    return "Negotiating environment-driven parameter settings.";
                case CONNECTION_SSL_STARTUP:
                    return "Negotiating SSL encryption.";
                case CONNECTION_NEEDED:
                    return "Internal state: connect() needed.";
                case CONNECTION_CHECK_WRITABLE:
                    return "Check if we could make a writable connection.";
                case CONNECTION_CONSUME:
                    return "Wait for any pending message and consume them.";
                case CONNECTION_GSS_STARTUP:
                    return "Negotiating GSSAPI.";
                case CONNECTION_CHECK_TARGET:
                    return "Check if we have a proper target connection";
            }

            return nullptr;
        }
        //--------------------------------------------------------------------------------------------------------------

        LPCSTR CPQConnection::PollingStatusString() {
            switch (m_PollingStatus) {
                case PGRES_POLLING_FAILED:
                    return "Polling failed.";
                case PGRES_POLLING_READING:
                    return "Waiting for socket to become readable.";
                case PGRES_POLLING_WRITING:
                    return "Waiting for socket to become writable.";
                case PGRES_POLLING_OK:
                    return "Polling success.";
                case PGRES_POLLING_ACTIVE:
                    return "Polling active.";
            }

            return nullptr;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnection::CheckConnection() {
            if (m_pHandle == nullptr)
                throw EDBConnectionError(_T("Not enough memory."));

            m_TryConnect = true;

            GetStatus();

            if (m_Status == CONNECTION_BAD)
                throw EDBConnectionError(_T("[%d] Connection failed: %s"), m_Socket, GetErrorMessage());

            if (m_Status == CONNECTION_OK) {
                m_Connected = true;
                SetReceiver();
                SetProcessor();
                DoConnected(this);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnection::CheckPollConnection() {
            if (m_pHandle == nullptr)
                throw EDBConnectionError(_T("Not enough memory."));

            m_TryConnect = true;

            DoPollingStatus(this);

            if (m_PollingStatus == PGRES_POLLING_FAILED)
                throw EDBConnectionError(_T("[%d] Connection failed: %s"), m_Socket, GetErrorMessage());

            if (m_PollingStatus == PGRES_POLLING_WRITING)
                m_AntiFreeze = Now();

            if (m_PollingStatus == PGRES_POLLING_OK) {
                m_Connected = true;
                SetReceiver();
                SetProcessor();
                DoConnected(this);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnection::Connect() {
            m_pHandle = PQconnectdb(m_ConnInfo.ConnInfo().c_str());
            CheckConnection();
            CheckSocket();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnection::ConnectStart() {
            m_pHandle = PQconnectStart(m_ConnInfo.ConnInfo().c_str());
            CheckPollConnection();
            CheckSocket(true);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnection::ConnectPoll() {
            m_PollingStatus = PQconnectPoll(m_pHandle);
            CheckPollConnection();
            CheckSocket(true);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnection::ResetStart() {
            if (!PQresetStart(m_pHandle)) {
                ConnectStart();
                ConnectPoll();
            }
            CheckConnection();
            CheckSocket(true);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnection::ResetPoll() {
            m_PollingStatus = PQresetPoll(m_pHandle);
            CheckPollConnection();
            CheckSocket(true);
        }
        //--------------------------------------------------------------------------------------------------------------

        int CPQConnection::PID() {
            if (m_pHandle != nullptr)
                return PQbackendPID(m_pHandle);
            return -1;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CPQConnection::PQSocket() {
            if (m_pHandle != nullptr)
                return PQsocket(m_pHandle);
            return INVALID_SOCKET;
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CPQConnection::NeedsPassword() {
            return PQconnectionNeedsPassword(m_pHandle) == 1;
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CPQConnection::UsedPassword() {
            return PQconnectionUsedPassword(m_pHandle) == 1;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnection::ConsumeInput() {
            if (!PQconsumeInput(m_pHandle))
                throw EDBConnectionError(_T("PQconsumeInput failed: %s"), GetErrorMessage());
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CPQConnection::IsBusy() {
            return PQisBusy(m_pHandle) == 1;
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CPQConnection::Flush() {
            int Result = PQflush(m_pHandle);

            if (Result == -1)
                throw EDBConnectionError(_T("PQflush failed: %s"), GetErrorMessage());

            return Result == 0;
        }
        //--------------------------------------------------------------------------------------------------------------

        PGnotify *CPQConnection::Notify() {
            return PQnotifies(m_pHandle);
        }
        //--------------------------------------------------------------------------------------------------------------

        PGresult *CPQConnection::GetResult() {
            return PQgetResult(m_pHandle);
        }
        //--------------------------------------------------------------------------------------------------------------

        PGcancel *CPQConnection::GetCancel() {
            return PQgetCancel(m_pHandle);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnection::Disconnect() {
            if (m_Connected) {
                DoDisconnected(this);
                m_Connected = false;
            }
            if (m_TryConnect) {
                if (PQSocket() != INVALID_SOCKET)
                    ClosePoll();
                Finish();
                m_TryConnect = false;
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnection::CallReceiver(const PGresult *AResult) {
            if (m_OnReceiver != nullptr) {
                m_OnReceiver(this, AResult);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnection::CallProcessor(LPCSTR AMessage) {
            if (m_OnProcessor != nullptr) {
                m_OnProcessor(this, AMessage);
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CPQPollConnection -----------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CPQPollConnection::CPQPollConnection(const CPQConnInfo &AConnInfo, CPollManager *AManager):
                CPQConnection(AConnInfo, AManager) {
            m_ConnectionStatus = qsConnect;
            m_WorkQuery = nullptr;
            m_AutoFree = true;
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CPQPollConnection::CheckResult() {
            if (m_WorkQuery == nullptr)
                return false;
            return m_WorkQuery->CheckResult();
        }
        //--------------------------------------------------------------------------------------------------------------

        int CPQPollConnection::CheckNotify() {
            PGnotify *pNotify;
            int nNotifies = 0;

            ConsumeInput();
            while ((pNotify = Notify()) != nullptr) {
                DoNotify(this, pNotify);
                PQfreemem(pNotify);
                nNotifies++;
                ConsumeInput();
            }

            return nNotifies;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQPollConnection::QueryStart(CPQQuery *AQuery) {
            m_WorkQuery = AQuery;
            m_WorkQuery->Connection(this);
            m_WorkQuery->SendQuery();
            m_ConnectionStatus = qsBusy;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQPollConnection::QueryStop() {
            FreeAndNil(m_WorkQuery);
            m_ConnectionStatus = qsReady;
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CPQResult -------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CPQResult::CPQResult(CPQQuery *AQQuery, PGresult *AHandle): CCollectionItem(AQQuery) {
            m_Query = AQQuery;
            m_pHandle = AHandle;
            m_Status = PGRES_FATAL_ERROR;
            m_OnStatus = nullptr;
        }
        //--------------------------------------------------------------------------------------------------------------

        CPQResult::~CPQResult() {
            Clear();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQResult::Clear() {
            if (m_pHandle != nullptr)
                PQclear(m_pHandle);
        }
        //--------------------------------------------------------------------------------------------------------------

        ExecStatusType CPQResult::ResultStatus() {
            if (m_pHandle != nullptr) {
                m_Status = PQresultStatus(m_pHandle);
                DoStatus();
                return m_Status;
            }
            return PGRES_FATAL_ERROR;
        }
        //--------------------------------------------------------------------------------------------------------------

        LPCSTR CPQResult::GetErrorMessage() {
            return PQresultErrorMessage(m_pHandle);
        }
        //--------------------------------------------------------------------------------------------------------------

        LPCSTR CPQResult::StatusString() {
            return PQresStatus(m_Status);
        }
        //--------------------------------------------------------------------------------------------------------------

        LPCSTR CPQResult::VerboseErrorMessage(PGVerbosity Verbosity, PGContextVisibility Context) {
            LPCSTR lpError = PQresultVerboseErrorMessage(m_pHandle, Verbosity, Context);
            m_VerboseErrorMessage = lpError;
            PQfreemem((void *) lpError);
            return m_VerboseErrorMessage.c_str();
        }
        //--------------------------------------------------------------------------------------------------------------

        LPCSTR CPQResult::ErrorField(int FieldCode) {
            return PQresultErrorField(m_pHandle, FieldCode);
        }
        //--------------------------------------------------------------------------------------------------------------

        int CPQResult::nTuples() {
            return PQntuples(m_pHandle);
        }
        //--------------------------------------------------------------------------------------------------------------

        int CPQResult::nFields() {
            return PQnfields(m_pHandle);
        }
        //--------------------------------------------------------------------------------------------------------------

        LPCSTR CPQResult::fName(int ColumnNumber) {
            return PQfname(m_pHandle, ColumnNumber);
        }
        //--------------------------------------------------------------------------------------------------------------

        int CPQResult::fNumber(LPCSTR ColumnName) {
            return PQfnumber(m_pHandle, ColumnName);
        }
        //--------------------------------------------------------------------------------------------------------------

        Oid CPQResult::fTable(int ColumnNumber) {
            return PQftable(m_pHandle, ColumnNumber);
        }
        //--------------------------------------------------------------------------------------------------------------

        int CPQResult::fTableCol(int ColumnNumber) {
            return PQftablecol(m_pHandle, ColumnNumber);
        }
        //--------------------------------------------------------------------------------------------------------------

        int CPQResult::fFormat(int ColumnNumber) {
            return PQfformat(m_pHandle, ColumnNumber);
        }
        //--------------------------------------------------------------------------------------------------------------

        Oid CPQResult::fType(int ColumnNumber) {
            return PQftype(m_pHandle, ColumnNumber);
        }
        //--------------------------------------------------------------------------------------------------------------

        int CPQResult::fMod(int ColumnNumber) {
            return PQfmod(m_pHandle, ColumnNumber);
        }
        //--------------------------------------------------------------------------------------------------------------

        int CPQResult::fSize(int ColumnNumber) {
            return PQfsize(m_pHandle, ColumnNumber);
        }
        //--------------------------------------------------------------------------------------------------------------

        int CPQResult::BinaryTuples() {
            return PQbinaryTuples(m_pHandle);
        }
        //--------------------------------------------------------------------------------------------------------------

        LPCSTR CPQResult::GetValue(int RowNumber, int ColumnNumber) {
            return PQgetvalue(m_pHandle, RowNumber, ColumnNumber);
        }
        //--------------------------------------------------------------------------------------------------------------

        int CPQResult::GetIsNull(int RowNumber, int ColumnNumber) {
            return PQgetisnull(m_pHandle, RowNumber, ColumnNumber);
        }
        //--------------------------------------------------------------------------------------------------------------

        int CPQResult::GetLength(int RowNumber, int ColumnNumber) {
            return PQgetlength(m_pHandle, RowNumber, ColumnNumber);
        }
        //--------------------------------------------------------------------------------------------------------------

        int CPQResult::nParams() {
            return PQnparams(m_pHandle);
        }
        //--------------------------------------------------------------------------------------------------------------

        Oid CPQResult::ParamType(int ParamNumber) {
            return PQparamtype(m_pHandle, ParamNumber);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQResult::DoStatus() {
            if (m_OnStatus != nullptr) {
                m_OnStatus(this);
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CPQQuery --------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CPQQuery::CPQQuery(): CCollection(this) {
            m_pConnection = nullptr;

            m_OnSendQuery = nullptr;
            m_OnExecuted = nullptr;

            m_OnResultStatus = nullptr;
            m_OnResult = nullptr;
        }
        //--------------------------------------------------------------------------------------------------------------

        CPQQuery::CPQQuery(CPQConnection *AConnection): CPQQuery() {
            m_pConnection = AConnection;
        }
        //--------------------------------------------------------------------------------------------------------------

        CPQQuery::~CPQQuery() {
            CPQQuery::Clear();
        }
        //--------------------------------------------------------------------------------------------------------------

        CPQResult *CPQQuery::GetResult(int Index) {
            return (CPQResult *) inherited::GetItem(Index);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQQuery::SetConnection(CPQConnection *Value) {
            if (m_pConnection != Value) {
                m_pConnection = Value;
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQQuery::Clear() {
            CCollection::Clear();
            m_SQL.Clear();
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CPQQuery::CheckResult() {
            CPQResult *pQueryResult;
            PGresult *pResult;

            m_pConnection->ConsumeInput();
            if (m_pConnection->Flush() /*&& !m_pConnection->IsBusy()*/) {
                while ((pResult = m_pConnection->GetResult()) != nullptr) {
                    pQueryResult = new CPQResult(this, pResult);
#if defined(_GLIBCXX_RELEASE) && (_GLIBCXX_RELEASE >= 9)
                    pQueryResult->OnStatus([this](auto &&AResult) { DoResultStatus(AResult); });
#else
                    pQueryResult->OnStatus(std::bind(&CPQQuery::DoResultStatus, this, _1));
#endif
                    DoResult(pQueryResult, pQueryResult->ResultStatus());
                }

                DoExecuted();

                return true;
            }

            return false;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQQuery::SendQuery() {
            if (m_pConnection == nullptr)
                throw EDBError(_T("Not set connection!"));

            if (!m_pConnection->Connected())
                throw EDBError(_T("Not connected!"));

            if (m_SQL.Count() == 0)
                throw EDBError(_T("Empty SQL query!"));

            if (PQsendQuery(m_pConnection->Handle(), m_SQL.Text().c_str()) == 0) {
                throw EDBError("PQsendQuery failed: %s", m_pConnection->GetErrorMessage());
            }

            DoSendQuery();

            m_pConnection->Flush();
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CPQQuery::CancelQuery(CString &Error) {
            const auto cancel = m_pConnection->GetCancel();
            Error.Clear();
            Error.SetLength(512);
            const auto result = PQcancel(cancel, Error.Data(), Error.Size());
            PQfreeCancel(cancel);
            return result == 1;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQQuery::DoSendQuery() {
            if (m_OnSendQuery != nullptr) {
                try {
                    m_OnSendQuery(this);
                } catch (...) {
                }
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQQuery::DoResultStatus(CPQResult *AResult) {
            if (m_OnResultStatus != nullptr) {
                try {
                    m_OnResultStatus(AResult);
                } catch (...) {
                }
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQQuery::DoResult(CPQResult *AResult, ExecStatusType AExecStatus) {
            if (m_OnResult != nullptr) {
                try {
                    m_OnResult(AResult, AExecStatus);
                } catch (...) {
                }
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQQuery::DoExecuted() {
            if (m_OnExecuted != nullptr) {
                try {
                    m_OnExecuted(this);
                } catch (...) {
                }
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CPQPollQuery ----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CPQPollQuery::CPQPollQuery(CPQConnectPoll *AServer): CPQQuery(), CCollectionItem(AServer->PollQueryManager()) {
            m_pServer = AServer;
            m_pPollConnection = nullptr;

            m_OnExecuted = nullptr;
            m_OnException = nullptr;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CPQPollQuery::Start() {
            try {
                auto pConnection = m_pServer->GetReadyConnection();

                if (pConnection != nullptr) {
                    try {
                        pConnection->QueryStart(this);
                    } catch (Delphi::Exception::Exception &E) {
                        DoException(E);
                        pConnection->QueryStop();
                    }
                } else {

                    if (m_pServer->Queue()->Count() == 0x0FFF)
                        throw EPollServerError(_T("Query queue is full."));

                    return AddToQueue();
                }

                return -1;
            } catch (Delphi::Exception::Exception &E) {
                DoException(E);
            }

            return POLL_QUERY_START_ERROR;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CPQPollQuery::AddToQueue() {
            if (m_pPollConnection == nullptr)
                return m_pServer->Queue()->AddToQueue(m_pServer, this);
            return m_pServer->Queue()->AddToQueue(m_pPollConnection, this);;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQPollQuery::RemoveFromQueue() {
            if (m_pPollConnection == nullptr) {
                m_pServer->Queue()->RemoveFromQueue(m_pServer, this);
            } else {
                m_pServer->Queue()->RemoveFromQueue(m_pPollConnection, this);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQPollQuery::DoExecuted() {
            if (m_OnExecuted != nullptr) {
                try {
                    m_OnExecuted(this);
                } catch (...) {
                }
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQPollQuery::DoException(const Delphi::Exception::Exception &E) {
            if (m_OnException != nullptr) {
                try {
                    m_OnException(this, E);
                } catch (...) {
                }
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CPQConnectPollEvent ---------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CPQConnectPollEvent::CPQConnectPollEvent(): CObject() {
            m_OnReceiver = nullptr;
            m_OnProcessor = nullptr;

            m_OnNotify = nullptr;

            m_OnError = nullptr;
            m_OnStatus = nullptr;
            m_OnPollingStatus = nullptr;

            m_OnConnectException = nullptr;
            m_OnServerException = nullptr;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnectPollEvent::DoReceiver(CPQConnection *AConnection, const PGresult *AResult) {
            if (m_OnReceiver != nullptr) {
                m_OnReceiver(AConnection, AResult);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnectPollEvent::DoProcessor(CPQConnection *AConnection, LPCSTR AMessage) {
            if (m_OnProcessor != nullptr) {
                m_OnProcessor(AConnection, AMessage);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnectPollEvent::DoNotify(CPQConnection *AConnection, PGnotify *ANotify) {
            if (m_OnNotify != nullptr) {
                m_OnNotify(AConnection, ANotify);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnectPollEvent::DoError(CPQConnection *AConnection) {
            if (m_OnError != nullptr) {
                m_OnError(AConnection);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnectPollEvent::DoStatus(CPQConnection *AConnection) {
            if (m_OnStatus != nullptr) {
                m_OnStatus(AConnection);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnectPollEvent::DoPollingStatus(CPQConnection *AConnection) {
            if (m_OnPollingStatus != nullptr) {
                m_OnPollingStatus(AConnection);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnectPollEvent::DoConnectException(CPQConnection *AConnection,
                const Delphi::Exception::Exception &E) {
            if (m_OnConnectException != nullptr) {
                m_OnConnectException(AConnection, E);
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CPQConnectPoll --------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CPQConnectPoll::CPQConnectPoll(size_t ASizeMin, size_t ASizeMax): CPQConnectPollEvent(), CEPollClient() {
            m_pTimer = nullptr;
            m_TimerInterval = 0;

            m_Active = false;

            m_SizeMin = ASizeMin;
            m_SizeMax = ASizeMax;

            m_pQueue = new CQueue;

            m_pPollQueryManager = new CPQPollQueryManager();
            m_pPollManager = new CPollManager;

            m_OnEventHandlerException = nullptr;
        }
        //--------------------------------------------------------------------------------------------------------------

        CPQConnectPoll::CPQConnectPoll(const CPQConnInfo &AConnInfo, size_t ASizeMin, size_t ASizeMax):
            CPQConnectPoll(ASizeMin, ASizeMax) {
            m_ConnInfo = AConnInfo;
        }
        //--------------------------------------------------------------------------------------------------------------

        CPQConnectPoll::~CPQConnectPoll() {
            StopAll();
            FreeAndNil(m_pQueue);
            FreeAndNil(m_pPollManager);
            FreeAndNil(m_pPollQueryManager);
        }
        //--------------------------------------------------------------------------------------------------------------

        CPQPollConnection *CPQConnectPoll::GetConnection(CPollEventHandler *AHandler) {
            return dynamic_cast<CPQPollConnection *> (AHandler->Binding());
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnectPoll::Start() {
            for (size_t i = 0; i < m_SizeMin; ++i) {
                if (!NewConnection())
                    break;
            }
            //SetTimerInterval(5000);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnectPoll::Stop(int Index) {
            m_pEventHandlers->Delete(Index);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnectPoll::StopAll() {
            m_pEventHandlers->Clear();
            //SetTimerInterval(0);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnectPoll::SetActive(bool Value) {
            if (m_Active != Value) {
                m_Active = Value;
                if (Value) {
                    Start();
                }  else {
                    StopAll();
                }
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnectPoll::SetTimerInterval(int Value) {
            if (m_TimerInterval != Value) {
                m_TimerInterval = Value;
                UpdateTimer();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnectPoll::UpdateTimer() {
            if (m_pTimer == nullptr) {
                m_pTimer = CEPollTimer::CreateTimer(CLOCK_MONOTONIC, TFD_NONBLOCK);
                m_pTimer->AllocateTimer(m_pEventHandlers, m_TimerInterval, m_TimerInterval);
#if defined(_GLIBCXX_RELEASE) && (_GLIBCXX_RELEASE >= 9)
                m_pTimer->OnTimer([this](auto && AHandler) { DoTimer(AHandler); });
#else
                m_pTimer->OnTimer(std::bind(&CPQConnectPoll::DoTimer, this, _1));
#endif
            } else {
                m_pTimer->SetTimer(m_TimerInterval, m_TimerInterval);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        CPollEventHandler *CPQConnectPoll::NewEventHandler(CPQConnection *AConnection) {
            CPollEventHandler *pEventHandler = nullptr;

            try {
                pEventHandler = m_pEventHandlers->Add(AConnection->Socket());

                if (ExternalPollStack()) {
#if defined(_GLIBCXX_RELEASE) && (_GLIBCXX_RELEASE >= 9)
                    pEventHandler->OnTimeOutEvent([this](auto &&AHandler) { DoTimeOut(AHandler); });
                    pEventHandler->OnReadEvent([this](auto &&AHandler) { DoRead(AHandler); });
                    pEventHandler->OnWriteEvent([this](auto &&AHandler) { DoWrite(AHandler); });
#else
                    pEventHandler->OnTimeOutEvent(std::bind(&CPQConnectPoll::DoTimeOut, this, _1));
                    pEventHandler->OnReadEvent(std::bind(&CPQConnectPoll::DoRead, this, _1));
                    pEventHandler->OnWriteEvent(std::bind(&CPQConnectPoll::DoWrite, this, _1));
#endif
                }

                pEventHandler->Binding(AConnection);
                pEventHandler->Start(etIO);

            } catch (Delphi::Exception::Exception &E) {
                DoServerException(E);
                FreeAndNil(pEventHandler);
            }

            return pEventHandler;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnectPoll::OnChangeSocket(CPQConnection *AConnection, CSocket AOldSocket) {
            if (AOldSocket != SOCKET_ERROR) {
                auto pEventHandler = m_pEventHandlers->FindHandlerBySocket(AOldSocket);
                if (Assigned(pEventHandler)) {
                    DoError(AConnection);
                    pEventHandler->Start(etNull);
                    pEventHandler->Stop();
                }
            } else if (NewEventHandler(AConnection) == nullptr)
                throw EDBConnectionError(_T("Cannot change (%d) socket to (%d)"), AOldSocket, AConnection->Socket());
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CPQConnectPoll::NewConnection() {
            int PingCount = 0;
            auto pConnection = new CPQPollConnection(m_ConnInfo, m_pPollManager);

            try {
                m_ConnInfo.PingValid(m_ConnInfo.Ping() == PQPING_OK);
                while (!m_ConnInfo.PingValid() && (PingCount < 3)) {
                    sleep(1);
                    PingCount++;
                    m_ConnInfo.PingValid(m_ConnInfo.Ping() == PQPING_OK);
                }

                if (!m_ConnInfo.PingValid()) {
                    switch (m_ConnInfo.Ping()) {
                        case PQPING_OK:
                            m_ConnInfo.PingValid(true);
                            break;

                        case PQPING_REJECT:
                            throw EDBConnectionError(
                                    _T("The server is running but is in a state that disallows connections "
                                       "(startup, shutdown, or crash recovery)."));

                        case PQPING_NO_RESPONSE:
                            throw EDBConnectionError(_T("The server could not be contacted."));

                        case PQPING_NO_ATTEMPT:
                            throw EDBConnectionError(
                                    _T("No attempt was made to contact the server, because the supplied parameters "
                                       "were obviously incorrect or there was some client-side problem "
                                       "(for example, out of memory)."));
                    }
                }
#if defined(_GLIBCXX_RELEASE) && (_GLIBCXX_RELEASE >= 9)
                pConnection->OnChangeSocket([this](auto && AConnection, auto && AOldSocket) { OnChangeSocket(AConnection, AOldSocket); });

                if (m_OnReceiver != nullptr) {
                    pConnection->OnReceiver([this](auto && AConnection, auto && AResult) { DoReceiver(AConnection, AResult); });
                }

                if (m_OnProcessor != nullptr) {
                    pConnection->OnProcessor([this](auto && AConnection, auto && AMessage) { DoProcessor(AConnection, AMessage); });
                }

                if (m_OnNotify != nullptr) {
                    pConnection->OnNotify([this](auto && AConnection, auto && ANotify) { DoNotify(AConnection, ANotify); });
                }

                if (m_OnError != nullptr) {
                    pConnection->OnError([this](auto && AConnection) { DoError(AConnection); });
                }

                if (m_OnStatus != nullptr) {
                    pConnection->OnStatus([this](auto && AConnection) { DoStatus(AConnection); });
                }

                if (m_OnPollingStatus != nullptr) {
                    pConnection->OnPollingStatus([this](auto && AConnection) { DoPollingStatus(AConnection); });
                }

                if (m_OnConnected != nullptr) {
                    pConnection->OnConnected([this](auto && Sender) { DoConnected(Sender); });
                }

                if (m_OnDisconnected != nullptr) {
                    pConnection->OnDisconnected([this](auto && Sender) { DoDisconnected(Sender); });
                }
#else
                pConnection->OnChangeSocket(std::bind(&CPQConnectPoll::OnChangeSocket, this, _1, _2));

                if (m_OnReceiver != nullptr) {
                    pConnection->OnReceiver(std::bind(&CPQConnectPoll::DoReceiver, this, _1, _2));
                }

                if (m_OnProcessor != nullptr) {
                    pConnection->OnProcessor(std::bind(&CPQConnectPoll::DoProcessor, this, _1, _2));
                }

                if (m_OnNotify != nullptr) {
                    pConnection->OnNotify(std::bind(&CPQConnectPoll::DoNotify, this, _1, _2));
                }

                if (m_OnError != nullptr) {
                    pConnection->OnError(std::bind(&CPQConnectPoll::DoError, this, _1));
                }

                if (m_OnStatus != nullptr) {
                    pConnection->OnStatus(std::bind(&CPQConnectPoll::DoStatus, this, _1));
                }

                if (m_OnPollingStatus != nullptr) {
                    pConnection->OnPollingStatus(std::bind(&CPQConnectPoll::DoPollingStatus, this, _1));
                }

                if (m_OnConnected != nullptr) {
                    pConnection->OnConnected(std::bind(&CPQConnectPoll::DoConnected, this, _1));
                }

                if (m_OnDisconnected != nullptr) {
                    pConnection->OnDisconnected(std::bind(&CPQConnectPoll::DoDisconnected, this, _1));
                }
#endif
                pConnection->ConnectStart();
                pConnection->ConnectPoll();

                return true;
            } catch (Delphi::Exception::Exception &E) {
                DoConnectException(pConnection, E);
                delete pConnection;
            }

            return false;
        }
        //--------------------------------------------------------------------------------------------------------------

        CPQPollConnection *CPQConnectPoll::GetReadyConnection() {
            CPQPollConnection *pConnection;
            CPollEventHandler *pHandler;
            CPQPollConnection *pResult = nullptr;

            for (int i = 0; i < m_pEventHandlers->Count(); ++i) {
                pHandler = m_pEventHandlers->Handlers(i);

                const auto eventType = pHandler->EventType();
                if (!(eventType == etIO || eventType == etDelete))
                    continue;

                pConnection = GetConnection(pHandler);
                if (pConnection == nullptr)
                    continue;

                if (pConnection->Connected()) {
                    if (pConnection->ConnectionStatus() == qsReady) {
                        pResult = pConnection;
                        break;
                    }
                } else {
                    const auto status = pConnection->Status();
                    if (((status == CONNECTION_STARTED || status == CONNECTION_MADE) && (Now() - pConnection->AntiFreeze() >= (CDateTime) 10 / SecsPerDay))) {
                        DoError(pConnection);
                        pHandler->Start(etNull);
                        Stop(i);
                    } else if (status == CONNECTION_BAD) {
                        DoError(pConnection);
                        pConnection->ConnectionStatus(qsReset);
                        pConnection->ResetStart();
                        pConnection->ResetPoll();
                    }
                }
            }

            if (pResult == nullptr && (m_pPollManager->Count() < (int) m_SizeMax)) {
                if (!NewConnection())
                    throw Exception::EDBConnectionError(_T("Unable to create new database connection."));
            }

            return pResult;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnectPoll::DoTimer(CPollEventHandler *AHandler) {
            CPQPollConnection *pConnection;
            CPollEventHandler *pEventHandler;

            ConnStatusType Status;

            uint64_t exp;

            auto pTimer = dynamic_cast<CEPollTimer *> (AHandler->Binding());
            pTimer->Read(&exp, sizeof(uint64_t));

            for (int i = 0; i < m_pEventHandlers->Count(); ++i) {
                pEventHandler = m_pEventHandlers->Handlers(i);
                if (pEventHandler->EventType() == etIO) {
                    pConnection = GetConnection(pEventHandler);
                    if (Assigned(pConnection)) {
                        if (pConnection->Listener())
                            pConnection->CheckNotify();
/*
                        Status = PQstatus(pConnection->Handle());
                        if (((Status == CONNECTION_STARTED || Status == CONNECTION_MADE) && (Now() - pConnection->AntiFreeze() >= (CDateTime) 10 / SecsPerDay))) {
                            DoError(pConnection);
                            pEventHandler->Start(etNull);
                            Stop(i);
                        } else if (Status == CONNECTION_OK && pConnection->ConnectionStatus() == qsBusy) {
                            if (Assigned(pConnection->WorkQuery())) {
                                if (pConnection->CheckResult()) {
                                    pConnection->QueryStop();
                                    CheckQueue();
                                }
                            }
                        }
*/
                    }
                }
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnectPoll::DoTimeOut(CPollEventHandler *AHandler) {
            auto pConnection = GetConnection(AHandler);
            if (Assigned(pConnection)) {
                if (!pConnection->Connected()) {
                    DoError(pConnection);
                    AHandler->Stop();
                }
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnectPoll::DoConnect(CPollEventHandler *AHandler) {

        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnectPoll::DoRead(CPollEventHandler *AHandler) {
            auto pConnection = GetConnection(AHandler);
            try {
                switch (pConnection->ConnectionStatus()) {
                    case qsConnect:
                        pConnection->ConnectPoll();
                        break;

                    case qsReset:
                        pConnection->ResetPoll();
                        break;

                    case qsReady:
                        pConnection->CheckNotify();
                        break;

                    case qsBusy:
                        if (pConnection->CheckResult())
                            pConnection->QueryStop();
                        pConnection->CheckNotify();
                        break;

                    case qsError:
                        // Connection closed gracefully
                        m_ConnInfo.PingValid(false);
                        pConnection->Disconnect();
                        break;
                }
            } catch (Delphi::Exception::Exception &E) {
                pConnection->ConnectionStatus(qsError);
                DoConnectException(pConnection, E);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnectPoll::DoWrite(CPollEventHandler *AHandler) {
            auto pConnection = GetConnection(AHandler);
            try {
                switch (pConnection->ConnectionStatus()) {
                    case qsConnect:
                        if (pConnection->Connected()) {
                            pConnection->ConnectionStatus(qsReady);
                            CheckQueue();
                        } else {
                            pConnection->ConnectPoll();
                        }
                        break;

                    case qsReset:
                        if (pConnection->Connected()) {
                            pConnection->ConnectionStatus(qsReady);
                            CheckQueue();
                        } else {
                            pConnection->ResetPoll();
                        }
                        break;

                    case qsReady:
                        pConnection->Flush();

                        if ((m_pQueue->Count() == 0) && !pConnection->Listener() && (m_pPollManager->Count() > (int) m_SizeMax)) {
                            pConnection->Disconnect();
                        }

                        CheckQueue();
                        break;

                    case qsBusy:
                        if (pConnection->CheckResult())
                            pConnection->QueryStop();
                        pConnection->CheckNotify();
                        break;

                    case qsError:
                        // Connection closed gracefully
                        m_ConnInfo.PingValid(false);
                        pConnection->Disconnect();
                        break;
                }
            } catch (Delphi::Exception::Exception &E) {
                pConnection->ConnectionStatus(qsError);
                DoConnectException(pConnection, E);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQConnectPoll::CheckQueue() {
            CPQPollQuery *pPollQuery;
            if (m_pQueue->Count() > 0) {
                pPollQuery = (CPQPollQuery *) m_pQueue->First()->First();
                pPollQuery->RemoveFromQueue();
                pPollQuery->Start();
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        //- CPQServer --------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CPQPollQuery *CPQServer::GetQuery() {
            return new CPQPollQuery(this);
        }
        //--------------------------------------------------------------------------------------------------------------

        CPQPollQuery *CPQServer::FindQueryByConnection(CPollConnection *APollConnection) {
            for (int i = 0; i < PollQueryManager()->QueryCount(); ++i) {
                if (PollQueryManager()->Queries(i)->PollConnection() == APollConnection)
                    return PollQueryManager()->Queries(i);
            }
            return nullptr;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPQServer::DoServerException(const Delphi::Exception::Exception &E) {
            if (m_OnServerException != nullptr) {
                m_OnServerException(this, E);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CPQServer::DoCommand(CTCPConnection *AConnection) {
            return false;
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CPQServer::DoExecute(CTCPConnection *AConnection) {
            return CEPollClient::DoExecute(AConnection);
        }
    }
}
}
#endif