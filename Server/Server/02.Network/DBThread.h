#pragma once

#include <windows.h>
#include <sqltypes.h>
#include <sqlext.h>
#include <sql.h>
#include <functional>

enum class DB_TASK_TYPE
{
	None,
	Player_Load,
	Player_Save,
	Item_Save,
	Item_Load,
};

struct DBTask
{
	DB_TASK_TYPE type;
	int32 playerId;
	char id[20];
	char password[20];

	DBTask()
		:type(DB_TASK_TYPE::None), playerId(-1)
	{
		memset(id, 0, sizeof(id));
		memset(password, 0, sizeof(password));
	}

	DBTask(DB_TASK_TYPE t, int32 pi, const char* i, const char* p) : type(t), playerId(pi)
	{
		// 0으로 초기화 후 안전하게 복사 (문자열 끝 NULL 처리 포함)
		memset(id, 0, sizeof(id));
		memset(password, 0, sizeof(password));

		if (i) strncpy_s(id, i, _TRUNCATE);
		if (p) strncpy_s(password, p, _TRUNCATE);
	}
};

class DBThread
{
public:
	DBThread();
	~DBThread();

	void pushTask(DBTask task);

	void run();
	void loadPlayer(int32 playerId, const char* id, const char* password);
	void databaseErr(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);

	static DBThread& get()
	{
		static DBThread instance;
		return instance;
	}

private:

	//큐 작업 변수
	std::queue<DBTask> _jobQueue;
	std::mutex _jobLock;
	std::condition_variable _cv;
	bool _isRunning = true;

	//SQL 핸들 변수
	//ODBC 환경변수
	SQLHENV hEnv = nullptr;
	//SQL 연결
	SQLHDBC hDbc = nullptr;
	//쿼리문 연결
	SQLHSTMT hStmt = nullptr;

	SQLRETURN ret;
};

