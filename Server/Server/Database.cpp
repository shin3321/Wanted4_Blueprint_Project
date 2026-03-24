#include "pch.h"
#include "Database.h"

Database::~Database()
{
	SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
	//여기서 디스커넥트도 해 줘야 함
	SQLDisconnect(hDbc);
	SQLDisconnect(hDbc);
	SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
	SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
}

void Database::init()
{
	//sql 연결
	if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv) == SQL_SUCCESS)
	{
		SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);

		if (SQLAllocHandle(SQL_HANDLE_DBC, SQL_NULL_HANDLE, &hDbc) == SQL_SUCCESS)
		{
			ret = SQLConnect(hDbc, (SQLWCHAR*)L"DB_DeadByDaylight", SQL_NTS, NULL, 0, NULL, 0);
			if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
			{
				//데이터베이스 에러 처리
			}
			if (SQLAllocHandle(SQL_HANDLE_STMT, SQL_NULL_HANDLE, &hStmt) == SQL_SUCCESS)
			{
				//Todo
				// 쿼리문 작성하고 연결
				//if(SQLPrepare(hStmt, (SQLWCHAR*)L"{}"))
				{
					//SQLBindParameter()
				}
			}
		}
	}

}

void Database::bindParameter()
{
}

