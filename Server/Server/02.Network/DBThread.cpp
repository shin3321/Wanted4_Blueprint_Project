#include "pch.h"
#include "DBThread.h"
#include "01.Game/Game.h"

void DBThread::databaseErr(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode)
{
	SQLSMALLINT iRec = 0;
	SQLINTEGER  iError;
	WCHAR       wszMessage[1000];
	WCHAR       wszState[SQL_SQLSTATE_SIZE + 1];

	if (RetCode == SQL_INVALID_HANDLE) {
		fwprintf(stderr, L"Invalid handle!\n");
		return;
	}
	while (SQLGetDiagRec(hType, hHandle, ++iRec, wszState, &iError, wszMessage,
		(SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)), (SQLSMALLINT*)NULL) == SQL_SUCCESS) {
		// Hide data truncated..
		if (wcsncmp(wszState, L"01004", 5)) {
			fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
		}
	}
}

DBThread::DBThread()
{
	setlocale(LC_ALL, "korean");

	// 1. 환경 핸들 할당
	ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
	if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
	{
		SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);

		// 2. 연결 핸들 할당
		ret = SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
		if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
		{
			ret = SQLConnect(hDbc, (SQLWCHAR*)L"DB_DeadByDaylight", SQL_NTS, NULL, 0, NULL, 0);
			if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
			{
				std::cout << "SQLConnect Error\n";
				databaseErr(hDbc, SQL_HANDLE_DBC, ret); // 연결 실패는 hDbc에 기록됨
			}

			// 3. 스테이트먼트 핸들 할당
			ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
			if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
			{
				std::cout << "SQLAllocHandle - STMT Error\n";
				databaseErr(hDbc, SQL_HANDLE_DBC, ret);
			}
		}
		else
		{
			// DBC 생성 실패 시 부모인 hEnv에게 물어봐야 합니다.
			std::cout << "SQLAllocHandle - SQL_HANDLE_DBC Error\n";
			databaseErr(hEnv, SQL_HANDLE_ENV, ret);
		}
	}
	else
	{
		// 환경 핸들 자체가 실패하면 핸들이 없으므로 에러 문구만 출력
		std::cout << "SQLAllocHandle - SQL_HANDLE_ENV Error\n";
	}
}

DBThread::~DBThread()
{
	SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
	SQLDisconnect(hDbc);
	SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
	SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
}

void DBThread::pushTask(DBTask task)
{
	{
		std::lock_guard<std::mutex> lock(_jobLock);
		_jobQueue.push(task);
	}
	_cv.notify_one();
}

void DBThread::run()
{
	while (_isRunning)
	{
		DBTask task;
		{
			std::unique_lock<std::mutex>lock(_jobLock);
			_cv.wait(lock, [this] {return !_jobQueue.empty() || !_isRunning; });

			if (!_isRunning && _jobQueue.empty()) break;

			task = _jobQueue.front();
			_jobQueue.pop();
		}

		switch (task.type)
		{
		case DB_TASK_TYPE::Player_Load:
		{
			loadPlayer(task.playerId, task.id, task.password);
			break;
		}

		}
	}
}

void DBThread::loadPlayer(int32 playerId, const char* id, const char* password)
{
	std::cout << "Load Player\n";

	int32 result = -1;
	SQLCloseCursor(hStmt);

	ret = SQLPrepare(hStmt, (SQLWCHAR*)L"{Call LoadPlayerInfo(?, ?)}", SQL_NTS);
	if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
	{
		std::cout << "SQLPrepare - Error\n";
		databaseErr(hStmt, SQL_HANDLE_STMT, ret);
		return;
	}
	SQLLEN lenId = SQL_NTS;
	SQLLEN lenPw = SQL_NTS;

	SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_WVARCHAR, 50, 0, (SQLPOINTER)id, 0, &lenId);
	SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_WVARCHAR, 50, 0, (SQLPOINTER)password, 0, &lenPw);

	ret = SQLExecute(hStmt);
	if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
	{
		if (SQLFetch(hStmt) == SQL_SUCCESS)
		{
			SQLGetData(hStmt, 1, SQL_C_SLONG, &result, 0, NULL);
		}
		else
		{
			std::cout << "SQLFetch - Error\n";
			databaseErr(hStmt, SQL_HANDLE_STMT, ret);
			return ;
		}
	}

	else
	{
		std::cout << "SQLExecute - Error\n";
		databaseErr(hStmt, SQL_HANDLE_STMT, ret);
		return ;
	}

	if (result > -1)
	{
		std::cout << "Login Success";
	}
	else
	{
		std::cout << "Wrong Password";
	}

	Game::get().sendLoginResult(playerId, result);
}
