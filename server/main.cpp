// 📁 main.cpp - Entry point
#include "handlers.h"
#include "database.h"
#include <sqlite3.h>
#include <iostream>

// ANSI color codes
#define GREEN "\033[32m"
#define CYAN "\033[36m"
#define RED "\033[31m"
#define YELLOW "\033[33m"
#define RESET "\033[0m"

int main()
{
    std::cout << CYAN << "🔧 Initializing database..." << RESET << std::endl;
    init_database();
    std::cout << GREEN << "✅ Database initialized successfully!" << RESET << std::endl;

    std::cout << CYAN << "📥 Loading all users from the database..." << RESET << std::endl;
    load_all_users();
    std::cout << GREEN << "✅ Users loaded successfully!" << RESET << std::endl;

    std::cout << YELLOW << "🚀 Starting the chat server..." << RESET << std::endl;
    start_server(); // This runs infinitely

    // This line technically won't be reached unless the server exits
    sqlite3_close(db);
    std::cout << RED << "🔒 Database connection closed." << RESET << std::endl;

    return 0;
}
