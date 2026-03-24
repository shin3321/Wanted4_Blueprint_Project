#pragma once

#include <windows.h>
#include <sqltypes.h>
#include <sqlext.h>
#include <sql.h>
#include <functional>

class Database
{
public:
	Database() {}
	~Database();
	void init();
	void bindParameter();


private:
	//큐 작업 변수
	std::queue<std::function<void()>> _jobQueue;
	std::mutex _jobLock;
	std::condition_variable _cv;

	//SQL 핸들 변수
	//ODBC 환경변수
	SQLHENV hEnv = nullptr;
	//SQL 연결
	SQLHDBC hDbc = nullptr;
	//쿼리문 연결
	SQLHSTMT hStmt = nullptr;

	SQLRETURN ret;
};

