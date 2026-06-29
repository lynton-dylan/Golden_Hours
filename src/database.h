#pragma once
#include <sqlite3.h>
#include <memory>
#include <string>

struct SqliteDbDeleter 
{
    void operator()(sqlite3* db) const { if (db) sqlite3_close(db); }
};

struct SqliteStmtDeleter 
{
    void operator()(sqlite3_stmt* stmt) const { if (stmt) sqlite3_finalize(stmt); }
};

using SqliteDbPtr = std::unique_ptr<sqlite3, SqliteDbDeleter>;
using SqliteStmtPtr = std::unique_ptr<sqlite3_stmt, SqliteStmtDeleter>;

class Database
{
public:
    static bool init();
    static SqliteDbPtr get_connection();
};

