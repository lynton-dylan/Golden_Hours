#include "routes.h"
#include "database.h"
#include "utilities.h"
#include <iostream>

void setup_routes(crow::App<crow::CookieParser>& app) {

    // landing / index page
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

    //Registration Route
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
            return crow::response(400, "Registration failed (User might already exist).");
        }

        crow::response res;
        res.code = 303;
        res.set_header("Location", "/");
        return res;
        });

    // Login Route
    CROW_ROUTE(app, "/login").methods(crow::HTTPMethod::POST)([&app](const crow::request& req) {
        std::string body_with_query = "?" + req.body;
        crow::query_string query_args(body_with_query);

        const char* email = query_args.get("email");
        const char* password = query_args.get("password");

        if (!email || !password) return crow::response(400, "Missing credentials.");

        auto db = Database::get_connection();
        if (!db) return crow::response(500, "Database unavailable.");

        sqlite3_stmt* raw_stmt = nullptr;
        const char* sql = "SELECT name FROM users WHERE email = ? AND password = ?;";
        if (sqlite3_prepare_v2(db.get(), sql, -1, &raw_stmt, nullptr) != SQLITE_OK) {
            return crow::response(500, "Database error.");
        }
        SqliteStmtPtr stmt(raw_stmt);

        sqlite3_bind_text(stmt.get(), 1, email, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt.get(), 2, password, -1, SQLITE_STATIC);

        if (sqlite3_step(stmt.get()) == SQLITE_ROW) {
            const unsigned char* name_text = sqlite3_column_text(stmt.get(), 0);
            std::string db_name = name_text ? reinterpret_cast<const char*>(name_text) : "";

            auto& ctx = app.get_context<crow::CookieParser>(req);
            ctx.set_cookie("username", db_name).path("/").httponly();

            crow::response res;
            res.code = 303;
            res.set_header("Location", "/dashboard");
            return res;
        }

        return crow::response(401, "Invalid email or password.");
        });

    // Dashboard
    CROW_ROUTE(app, "/dashboard")([&app](const crow::request& req) {
        auto& ctx = app.get_context<crow::CookieParser>(req);
        std::string username = ctx.get_cookie("username");

        if (username.empty()) {
            crow::response res;
            res.code = 303;
            res.set_header("Location", "/");
            return res;
        }
        return crow::response(200, "Welcome to the dashboard, " + username);
        });

    // Logout Route
    CROW_ROUTE(app, "/logout")([&app](const crow::request& req) {
        auto& ctx = app.get_context<crow::CookieParser>(req);
        ctx.set_cookie("username", "").max_age(0).path("/");

        crow::response res;
        res.code = 303;
        res.set_header("Location", "/");
        return res;
        });
}