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

    //MAIN MENU
    
    std::cout << "Thank you for using Golden Hours!" << std::endl;
    std::cout << "1. Login" << std::endl;
    std::cout << "2. Sign Up" << std::endl;
    std::cout << "3. Exit" << std::endl;

    int userChoice;
    bool running = true;
    std::cin >> userChoice;
    if (userChoice == 1) {
        while (running) {
            string username;
            string password;
            std::cout << "Enter your username: " << std::endl;
            std::cout << "Enter 'm' to return to the Main Menu" << std::endl;
            std::cin >> userName;
            while (!isValid(userName)) {
                std::cout << "The username you entered is invalid. Please try again." << std::endl;
            }
            if (userName == 'm') {
                running = false;
            } else {
                std::cout << "Enter your password: " << std::endl;
                std::cin >> password;
                /////USERNAME & PASSWORD ARE VERIFIED HERE
                while (!isValid(password)) {
                    std::cout << "The password you entered is valid. Please try again." << std::endl;
                    std::cout << "Enter 'm' to return to the Main Menu" << std::endl;
                    std::cin >> password;
                    if (password == 'm') {
                        running = false;
                        break;
                    }
                }
                accountMenu(username);  ////Specific menus for senior/volunteer accounts are accessed here using an outside function.
            }
                    
        }
    } else if (userChoice == 2) {
        running = true;
        while (running) {
            std::cout << "Welcome to Golden Hours!" << std::endl;
            std::cout << "Please select your reason for signing up:" << std::endl;
            std::cout << "1. I'm a volunteer." << std::endl;
            std::cout << "2. I'm looking for a volunteer." << std::endl;
            std::cout << "3. Return to Main Menu" << std::endl;
            int signUpChoice;
            cin >> signUpChoice;

            if (signUpChoice == 1) {
                Volunteer() newUser;
                newUser.signUp();
            } else if (signUpChoice == 2) {
                Senior() newUser;
                newUser.signUp();
            } else if (signUpChoice == 3) {
                running = false;
            } else {
                while (signUpChoice != 1 && signUpChoice != 2) {
                    std::cout << "Your entry is invalid. Please enter 1, 2, or 3." << std::endl;
                }
            }
    } else if (userChoice == 3) {
        return 0;
    } else {
        while (userChoice != 1 && userChoice != 2) {
            std::cout << "Your entry is invalid. Please enter 1, 2, or 3." << std::endl;
        }
    }
        
}
