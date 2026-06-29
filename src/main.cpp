#include <iostream>
#include "crow.h"
#include "crow/middlewares/cookie_parser.h"
#include "database.h"
#include "routes.h"

int main() {
    std::cout << "[Startup]: " << std::endl;

    if (!Database::init()) {
        std::cerr << "[Fatal Error]: Could not initialize SQLite database schema. Exiting." << std::endl;
        return 1;
    }
    std::cout << "[Startup]: Database loaded." << std::endl;

    crow::App<crow::CookieParser> app;

    setup_routes(app);

    std::cout << "[Network]: launch on port 8090." << std::endl;
    app.port(8090).multithreaded().run();

    return 0;
}