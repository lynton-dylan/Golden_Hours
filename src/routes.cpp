#include "routes.h"
#include "database.h"
#include "utilities.h"
#include <iostream>

void setup_routes(crow::App<crow::CookieParser>& app) {

    //Root
    CROW_ROUTE(app, "/")([&app](const crow::request& req) {
        auto& ctx = app.get_context<crow::CookieParser>(req);
        std::string username = ctx.get_cookie("username");

        if (!username.empty()) {
            crow::response res;
            res.code = 303;
            res.set_header("Location", "/dashboard");
            return res;
        }

        crow::response res(load_html_file("html/index.html"));
        res.set_header("Content-Type", "text/html");
        return res;
        });

    //Registration
    CROW_ROUTE(app, "/register").methods(crow::HTTPMethod::POST)([](const crow::request& req) {
        std::string body_with_query = "?" + req.body;
        crow::query_string query_args(body_with_query);

        const char* email = query_args.get("email");
        const char* password = query_args.get("password");
        const char* name = query_args.get("name");
        const char* role = query_args.get("role");

        if (!email || !password || !name || !role) {
            return crow::response(400, "Missing data.");
        }

        auto db = Database::get_connection();
        if (!db) return crow::response(500, "Database unavailable.");

        sqlite3_stmt* raw_stmt = nullptr;
        const char* sql = "INSERT INTO users (email, password, name, role) VALUES (?, ?, ?, ?);";
        if (sqlite3_prepare_v2(db.get(), sql, -1, &raw_stmt, nullptr) != SQLITE_OK) {
            return crow::response(500, "Statement error.");
        }
        SqliteStmtPtr stmt(raw_stmt);

        sqlite3_bind_text(stmt.get(), 1, email, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt.get(), 2, password, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt.get(), 3, name, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt.get(), 4, role, -1, SQLITE_STATIC);

        if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
            return crow::response(400, "Registration failed.");
        }

        crow::response res;
        res.code = 303;
        res.set_header("Location", "/");
        return res;
        });

    //Login
    CROW_ROUTE(app, "/login").methods(crow::HTTPMethod::POST)([&app](const crow::request& req) {
        std::string body_with_query = "?" + req.body;
        crow::query_string query_args(body_with_query);

        const char* email = query_args.get("email");
        const char* password = query_args.get("password");

        if (!email || !password) return crow::response(400, "Missing credentials.");

        auto db = Database::get_connection();
        if (!db) return crow::response(500, "Database unavailable.");

        sqlite3_stmt* raw_stmt = nullptr;
        const char* sql = "SELECT name, role FROM users WHERE email = ? AND password = ?;";
        if (sqlite3_prepare_v2(db.get(), sql, -1, &raw_stmt, nullptr) != SQLITE_OK) {
            return crow::response(500, "Database error.");
        }
        SqliteStmtPtr stmt(raw_stmt);

        sqlite3_bind_text(stmt.get(), 1, email, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt.get(), 2, password, -1, SQLITE_STATIC);

        if (sqlite3_step(stmt.get()) == SQLITE_ROW) {
            std::string db_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 0));
            std::string db_role = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 1));

            auto& ctx = app.get_context<crow::CookieParser>(req);
            ctx.set_cookie("username", db_name).path("/").httponly();
            ctx.set_cookie("user_role", db_role).path("/").httponly();
            ctx.set_cookie("user_email", email).path("/").httponly();

            crow::response res;
            res.code = 303;
            res.set_header("Location", "/dashboard");
            return res;
        }

        return crow::response(401, "Invalid credentials.");
        });

    //Dashboard
    CROW_ROUTE(app, "/dashboard")([&app](const crow::request& req) {
        auto& ctx = app.get_context<crow::CookieParser>(req);
        std::string username = ctx.get_cookie("username");

        if (username.empty()) {
            crow::response res;
            res.code = 303;
            res.set_header("Location", "/");
            return res;
        }

        crow::response res(load_html_file("html/dashboard.html"));
        res.set_header("Content-Type", "text/html");
        return res;
        });

    //User Data
    CROW_ROUTE(app, "/api/user-info")([&app](const crow::request& req) {
        auto& ctx = app.get_context<crow::CookieParser>(req);
        crow::json::wvalue res;
        res["username"] = ctx.get_cookie("username");
        res["role"] = ctx.get_cookie("user_role");
        return crow::response(res);
        });

    //Task Creation 
    CROW_ROUTE(app, "/api/tasks/create").methods(crow::HTTPMethod::POST)([&app](const crow::request& req) {
        auto& ctx = app.get_context<crow::CookieParser>(req);
        std::string username = ctx.get_cookie("username");
        if (username.empty() || ctx.get_cookie("user_role") != "Poster") return crow::response(403, "Unauthorized.");

        std::string body_with_query = "?" + req.body;
        crow::query_string query_args(body_with_query);

        const char* title = query_args.get("title");
        const char* category = query_args.get("category");
        const char* task_date = query_args.get("task_date");
        const char* description = query_args.get("description");
        const char* address = query_args.get("address");
        const char* hours_str = query_args.get("hours_expected");

        if (!title || !category || !task_date || !description || !address || !hours_str) {
            return crow::response(400, "Missing data.");
        }

        int hours_expected = std::stoi(hours_str);

        auto db = Database::get_connection();
        const char* sql = "INSERT INTO Tasks (poster_username, title, description, category, address, task_date, hours_expected, status) VALUES (?, ?, ?, ?, ?, ?, ?, 'Open');";

        sqlite3_stmt* raw_stmt = nullptr;
        if (sqlite3_prepare_v2(db.get(), sql, -1, &raw_stmt, nullptr) != SQLITE_OK) return crow::response(500, "SQL Error");
        SqliteStmtPtr stmt(raw_stmt);

        sqlite3_bind_text(stmt.get(), 1, username.c_str(), -1, SQLITE_STATIC);
        // implement the rest. as in title, description, category, address , task_date, hours_expected

        sqlite3_step(stmt.get());

        crow::response res; res.code = 303; res.set_header("Location", "/dashboard"); return res;
        });

    //Task Data  
    CROW_ROUTE(app, "/api/tasks")([&app](const crow::request& req) {
        auto& ctx = app.get_context<crow::CookieParser>(req);
        std::string username = ctx.get_cookie("username");
        std::string role = ctx.get_cookie("user_role");
        if (username.empty()) return crow::response(401, "Unauthorized");

        auto db = Database::get_connection();
        std::string query = "SELECT task_id, poster_username, title, description, category, task_date, status, address, assigned_worker, hours_expected FROM Tasks";

        if (role == "Poster") {
            query += " WHERE poster_username = '" + username + "' AND status != 'Deleted';";
        }
        else {
            query += " WHERE status != 'Deleted';";
        }

        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(db.get(), query.c_str(), -1, &stmt, nullptr);
        SqliteStmtPtr stmt_ptr(stmt);

        std::vector<crow::json::wvalue> task_list;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            crow::json::wvalue t;
            t["task_id"] = sqlite3_column_int(stmt, 0);
            t["poster"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            t["title"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

            // implement the rest. as in description, category, date, status , address, worker, hours_expected


            task_list.push_back(std::move(t));
        }
        return crow::response(crow::json::wvalue(task_list));
        });

    //Task Actions
    CROW_ROUTE(app, "/api/tasks/action").methods(crow::HTTPMethod::POST)([&app](const crow::request& req) {
        auto& ctx = app.get_context<crow::CookieParser>(req);
        std::string username = ctx.get_cookie("username");
        if (username.empty()) return crow::response(401);

        std::string body_with_query = "?" + req.body;
        crow::query_string query_args(body_with_query);
        std::string action = query_args.get("action") ? query_args.get("action") : "";
        std::string id = query_args.get("task_id") ? query_args.get("task_id") : "";

        auto db = Database::get_connection();
        std::string sql = "";

        if (action == "accept") {
            sql = "UPDATE Tasks SET status = 'Accepted', assigned_worker = '" + username + "' WHERE task_id = " + id + ";";
        }
        else if (action == "abandon") {
            sql = "UPDATE Tasks SET status = 'Open', assigned_worker = NULL WHERE task_id = " + id + ";";
        }
        else if (action == "complete") {
            sql = "UPDATE Tasks SET status = 'Completed' WHERE task_id = " + id + ";";
        }
        else if (action == "unlist") {
            sql = "UPDATE Tasks SET status = 'Deleted' WHERE task_id = " + id + ";";
        }

        sqlite3_exec(db.get(), sql.c_str(), nullptr, nullptr, nullptr);

        crow::response res; res.code = 303; res.set_header("Location", "/dashboard"); return res;
        });

    //Account Delete
    CROW_ROUTE(app, "/api/account/delete").methods(crow::HTTPMethod::POST)([&app](const crow::request& req) {
        auto& ctx = app.get_context<crow::CookieParser>(req);
        std::string email = ctx.get_cookie("user_email");
        if (email.empty()) return crow::response(401);

        auto db = Database::get_connection();
        std::string sql = "DELETE FROM users WHERE email = '" + email + "';";
        sqlite3_exec(db.get(), sql.c_str(), nullptr, nullptr, nullptr);

        ctx.set_cookie("username", "").max_age(0).path("/");
        ctx.set_cookie("user_role", "").max_age(0).path("/");
        ctx.set_cookie("user_email", "").max_age(0).path("/");

        crow::response res; res.code = 303; res.set_header("Location", "/"); return res;
        });

    //Logout 
    CROW_ROUTE(app, "/logout")([&app](const crow::request& req) {
        auto& ctx = app.get_context<crow::CookieParser>(req);
        ctx.set_cookie("username", "").max_age(0).path("/");
        ctx.set_cookie("user_role", "").max_age(0).path("/");
        ctx.set_cookie("user_email", "").max_age(0).path("/");

        crow::response res;
        res.code = 303;
        res.set_header("Location", "/");
        return res;
        });
}





// things left to implement
// a better response then just "invalid credentials" for login 
// when you register it logs you in
// the hour export system, it should let you print a thing and then remove the hours from your account. 
// finish the "/api/tasks" route 
// finish the "/api/tasks/create" route
// a couple other misc polish changes, that i cant remember right now.