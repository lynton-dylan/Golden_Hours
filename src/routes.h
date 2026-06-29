#pragma once
#include "crow.h"
#include "crow/middlewares/cookie_parser.h"

void setup_routes(crow::App<crow::CookieParser>& app);