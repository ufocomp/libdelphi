/*++

Library name:

  libdelphi

Module Name:

  SQLite.hpp

Notices:

  Delphi classes for C++

  SQLite3

Author:

  Copyright (c) Prepodobny Alen

  mailto: alienufo@inbox.ru
  mailto: ufocomp@gmail.com

--*/

#ifndef DELPHI_SQLITE_HPP
#define DELPHI_SQLITE_HPP
//----------------------------------------------------------------------------------------------------------------------

#include <sqlite3.h>

extern "C++" {

namespace Delphi {

    namespace SQLite3 {

        //--------------------------------------------------------------------------------------------------------------

        //- CSQLiteConnection -----------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CSQLiteConnection;

        typedef std::function<void (CSQLiteConnection *AConnection)> COnSQlLiteConnectionEvent;
        //--------------------------------------------------------------------------------------------------------------

        class CSQLiteConnection: public CObject {
        private:

            sqlite3 *m_Handle;

            CString m_DataBase;

            int m_ResultCode;

            COnSQlLiteConnectionEvent m_OnConnected;
            COnSQlLiteConnectionEvent m_OnDisconnected;

        protected:

            int GetResultCode();

            void DoConnected(CSQLiteConnection *AConnection);
            void DoDisconnected(CSQLiteConnection *AConnection);

        public:

            explicit CSQLiteConnection(const CString &ADataBase);

            ~CSQLiteConnection() override;

            const CString &DataBase() const { return m_DataBase; }

            bool Connect();

            void Disconnect();

            bool Connected() { return ResultCode() == SQLITE_OK; }

            int ResultCode() { return GetResultCode(); }

            LPCSTR GetErrorMessage();

            LPCSTR ResultCodeString();

            sqlite3 *Handle() { return m_Handle; }

            const COnSQlLiteConnectionEvent &OnConnected() { return m_OnConnected; }
            void OnConnected(COnSQlLiteConnectionEvent && Value) { m_OnConnected = Value; }

            const COnSQlLiteConnectionEvent &OnDisconnected() { return m_OnDisconnected; }
            void OnDisconnected(COnSQlLiteConnectionEvent && Value) { m_OnDisconnected = Value; }

        };

        //--------------------------------------------------------------------------------------------------------------

        //- CSQLiteResult ---------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CSQLiteQuery;
        class CSQLiteResult;

        typedef std::function<void (CSQLiteResult *AResult)> COnSQlLiteResultEvent;
        //--------------------------------------------------------------------------------------------------------------

        class CSQLiteResult: public CCollectionItem {
        private:

            sqlite3_stmt *m_Handle;

            CSQLiteQuery *m_Query;

            int m_ResultCode;

            COnSQlLiteResultEvent m_OnResult;

        protected:

            virtual void DoResult();

        public:

            explicit CSQLiteResult(CSQLiteQuery *AQQuery, sqlite3_stmt *AHandle);

            ~CSQLiteResult() override;

            void Clear();

            sqlite3_stmt *Handle() { return m_Handle; };

            CSQLiteQuery *Query() { return m_Query; };

            int ResultCode() { return m_ResultCode; };

            const COnSQlLiteResultEvent &OnResult() { return m_OnResult; }
            void OnResult(COnSQlLiteResultEvent && Value) { m_OnResult = Value; }

        };

        //--------------------------------------------------------------------------------------------------------------

        //- CSQLiteQuery ----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CSQLiteQuery;

        typedef std::function<void (CSQLiteQuery *AQuery)> COnSQlLiteQueryExecutedEvent;
        //--------------------------------------------------------------------------------------------------------------

        typedef TList<CVariant> CCSQLiteParams;

        class CSQLiteQuery: public CCollection {
            typedef CCollection inherited;

        private:

            CSQLiteConnection *m_pConnection;

            CString m_SQL;

            CCSQLiteParams m_Params;

            COnSQlLiteQueryExecutedEvent m_OnExecuted;

        protected:

            CSQLiteResult *GetResult(int Index);

            virtual void DoExecuted();

            void SetConnection(CSQLiteConnection *Value);

        public:

            CSQLiteQuery();

            explicit CSQLiteQuery(CSQLiteConnection *AConnection);

            ~CSQLiteQuery() override;

            CSQLiteConnection *Connection() { return m_pConnection; };

            void Connection(CSQLiteConnection *Value) { SetConnection(Value); };

            void Clear() override;

            int ResultCount() { return inherited::Count(); };

            void Execute();

            CString& SQL() { return m_SQL; }
            const CString& SQL() const { return m_SQL; }

            CCSQLiteParams& Params() { return m_Params; }
            const CCSQLiteParams& Params() const { return m_Params; }

            CSQLiteResult *Results(int Index) { return GetResult(Index); };

            const COnSQlLiteQueryExecutedEvent &OnExecuted() { return m_OnExecuted; }
            void OnExecute(COnSQlLiteQueryExecutedEvent && Value) { m_OnExecuted = Value; }

        };

    }
}

using namespace Delphi::SQLite3;
}
#endif //DELPHI_SQLITE_HPP
