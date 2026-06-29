#include "database.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <sqlite3.h>

bool Database::init() {
    auto db = get_connection();
    if (!db) return false;

    const char* create_users_table =
        "CREATE TABLE IF NOT EXISTS users ("
        "email TEXT PRIMARY KEY, password TEXT NOT NULL, name TEXT NOT NULL, role TEXT NOT NULL);";

    const char* create_tasks_table =
        "CREATE TABLE IF NOT EXISTS Tasks ("
        "task_id INTEGER PRIMARY KEY AUTOINCREMENT, poster_username TEXT NOT NULL, title TEXT NOT NULL, "
        "description TEXT, category TEXT, address TEXT, task_date TEXT, status TEXT DEFAULT 'Open', "
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP);";

    char* errMsg = nullptr;
    if (sqlite3_exec(db.get(), create_users_table, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "[Schema Error (Users)]: " << (errMsg ? errMsg : "Unknown error") << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    if (sqlite3_exec(db.get(), create_tasks_table, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "[Schema Error (Tasks)]: " << (errMsg ? errMsg : "Unknown error") << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

SqliteDbPtr Database::get_connection() {
    sqlite3* db = nullptr;
    if (sqlite3_open("GH_db.sqlite", &db) != SQLITE_OK) {
        std::cerr << "[Database Error]: Connection failed: " << (db ? sqlite3_errmsg(db) : "Unknown error") << std::endl;
        if (db) sqlite3_close(db);
        return nullptr;
    }
    return SqliteDbPtr(db);
}
