#include <sqlite3.h>
#include "database.h"
#include <iostream>
#include <cassert> //Used for simple testing
using namespace std;

//Check if a user with a specific email exists in users
bool userExists(sqlite3* db, const string& userEmail)
{
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, "SELECT * FROM users WHERE email = ?;", -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, userEmail.c_str(), -1, SQLITE_STATIC);
    bool found = sqlite3_step(stmt) == SQLITE_ROW;
    sqlite3_finalize(stmt);
    return found;
}

//Check if a task with a specific title exists in tasks
bool taskExists(sqlite3* db, const string& taskTitle)
{
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, "SELECT * FROM Tasks WHERE title = ?;", -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, taskTitle.c_str(), -1, SQLITE_STATIC);
    bool found = sqlite3_step(stmt) == SQLITE_ROW;
    sqlite3_finalize(stmt);
    return found;
}

//Main function to run tests
int main()
{
    //Initialize database
    assert(Database::init());
    auto db = Database::get_connection();
    assert(db);
    cout << "Golden Hours - Software Tests\n" << endl;

    //AC001 - Test to check if a new user account can be added to database; check if it exists afterward

    cout << "AC001: Test account register" << endl;

    sqlite3_exec(db.get(), "DELETE FROM users WHERE email = 'test@test.com';", nullptr, nullptr, nullptr);
    sqlite3_exec(db.get(), "INSERT INTO users VALUES " "('test@test.com','password123','Test User','Poster');", nullptr, nullptr, nullptr);

    assert(userExists(db.get(), "test@test.com"));
    cout << "Test AC001 passed.\n" << endl;

    //AC002 - Test to check if an existing user can log in; checks if provided email and password match a result in database

    cout << "AC002: Test login with existing account" << endl;

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db.get(), "SELECT * FROM users WHERE email = ? AND password = ?;", -1, &stmt, nullptr);

    sqlite3_bind_text(stmt, 1, "test@test.com", -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, "password123", -1, SQLITE_STATIC);

    assert(sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    cout << "Test AC002 passed.\n" << endl;

    //AC003: Test to make sure no user with duplicate email can register

    cout << "AC003: Test duplicate registration" << endl;

    int rc = sqlite3_exec(db.get(), "INSERT INTO users VALUES ""('test@test.com','abc','Duplicate','Poster');", nullptr, nullptr, nullptr);
    assert(rc != SQLITE_OK);
    cout << "Test AC003 passed.\n" << endl;

    //SM001: Test if a senior can create new task and ensure task goes into database

    cout << "SM001: Test newly created task" << endl;

    sqlite3_exec(db.get(), "DELETE FROM Tasks WHERE title = 'Yard Work';", nullptr, nullptr, nullptr);
    sqlite3_exec(db.get(),
        "INSERT INTO Tasks "
        "(poster_username,title,description,category,address,task_date,status,hours_expected)"
        "VALUES"
        "('Test User',"
        "'Yard Work',"
        "'Mow front lawn',"
        "'Yard',"
        "'123 Main Street',"
        "'2026-08-05',"
        "'Open',"
        "2);",
        nullptr,
        nullptr,
        nullptr);

    assert(taskExists(db.get(),"Yard Work"));
    cout << "Test SM001 passed.\n" << endl;

    //VM001: Test if a volunteer can accept a task, assign them to it, and change its status to Accepted

    cout << "VM001: Test a newly accepted task" << endl;

    sqlite3_exec(db.get(), "UPDATE Tasks " "SET status = 'Accepted'," "assigned_worker = 'Volunteer 1' " "WHERE title = 'Yard Work';", nullptr, nullptr, nullptr);
    sqlite3_prepare_v2(db.get(), "SELECT assigned_worker, status FROM Tasks WHERE title = 'Yard Work';", -1, &stmt, nullptr);

    assert(sqlite3_step(stmt) == SQLITE_ROW);

    string volunteer = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    string taskStatus = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

    assert(volunteer == "Volunteer 1");
    assert(taskStatus == "Accepted");

    sqlite3_finalize(stmt);
    cout << "Test VM001 passed.\n" << endl;

    //User Story: Test a task completion by changing it to Completed; ensure database makes the change visible

    cout << "User Story: Complete Task" << endl;

    sqlite3_exec(db.get(), "UPDATE Tasks " "SET status = 'Completed' " "WHERE title = 'Yard Work';", nullptr, nullptr, nullptr);
    sqlite3_prepare_v2(db.get(), "SELECT status FROM Tasks WHERE title = 'Yard Work';", -1, &stmt, nullptr);
    sqlite3_step(stmt);

    taskStatus = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));

    assert(taskStatus == "Completed");
    sqlite3_finalize(stmt);
    cout << "User Story test passed.\n" << endl;

    //User Story: Test the volunteer hour export functionality; mark completed tasks as Exported + confirm update to database

    cout << "User Story: Export Hours" << endl;

    sqlite3_exec(db.get(), "UPDATE Tasks " "SET status = 'Exported' " "WHERE assigned_worker = 'Volunteer 1';", nullptr, nullptr, nullptr);
    sqlite3_prepare_v2(db.get(), "SELECT status FROM Tasks " "WHERE assigned_worker = 'Volunteer 1';", -1, &stmt, nullptr);
    sqlite3_step(stmt);

    taskStatus = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));

    assert(taskStatus == "Exported");
    sqlite3_finalize(stmt);
    cout << "User Story test passed.\n" << endl;

    //Print once all tests passed
    cout << "All tests passed :)" << endl;

    return 0;
}
