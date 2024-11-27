#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <memory>
#include <stdexcept>
#include <sqlite3.h>

// Database connection class
class DatabaseConnection {
public:
    DatabaseConnection(const std::string& db_name) {
        if (sqlite3_open(db_name.c_str(), &db_) != SQLITE_OK) {
            throw std::runtime_error("Could not open database");
        }
    }

    ~DatabaseConnection() {
        if (db_) {
            sqlite3_close(db_);
        }
    }

    sqlite3* get() const { return db_; }

private:
    sqlite3* db_;
};

// Class to manage database operations
class DatabaseManager {
public:
    DatabaseManager(std::shared_ptr<DatabaseConnection> connection)
        : connection_(connection) {}

    void initializeDatabase() {
        std::lock_guard<std::mutex> lock(mutex_);

        const char* sql = R"(
            CREATE TABLE IF NOT EXISTS User (
                ID INTEGER PRIMARY KEY AUTOINCREMENT,
                Name TEXT NOT NULL,
                Age INTEGER NOT NULL
            );
        )";

        executeSQL(sql);
    }

    void insertUser(const std::string& name, int age) {
        std::lock_guard<std::mutex> lock(mutex_);

        std::string sql = "INSERT INTO User (Name, Age) VALUES ('" + name + "', " + std::to_string(age) + ");";
        executeSQL(sql.c_str());
    }

    void queryUsers() const {
        std::lock_guard<std::mutex> lock(mutex_);
        const char* sql = "SELECT * FROM User;";

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(connection_->get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare statement");
        }

        std::cout << "ID | Name | Age" << std::endl;
        std::cout << "---|------|---" << std::endl;

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            const unsigned char* name = sqlite3_column_text(stmt, 1);
            int age = sqlite3_column_int(stmt, 2);

            std::cout << id << " | " << name << " | " << age << std::endl;
        }

        sqlite3_finalize(stmt);
    }

private:
    std::shared_ptr<DatabaseConnection> connection_;
    mutable std::mutex mutex_;

    void executeSQL(const char* sql) const {
        char* errorMessage = nullptr;
        if (sqlite3_exec(connection_->get(), sql, nullptr, nullptr, &errorMessage) != SQLITE_OK) {
            std::string error = "SQL error: ";
            error += errorMessage;
            sqlite3_free(errorMessage);
            throw std::runtime_error(error);
        }
    }
};

// Function to simulate various operations
void simulateDbOperations(DatabaseManager& dbManager) {
    dbManager.initializeDatabase();

    dbManager.insertUser("Alice", 30);
    dbManager.insertUser("Bob", 24);
    dbManager.insertUser("Charlie", 29);

    dbManager.queryUsers();
}

int main() {
    try {
        std::shared_ptr<DatabaseConnection> connection = std::make_shared<DatabaseConnection>("virtual_engine.db");
        DatabaseManager dbManager(connection);

        std::vector<std::thread> threads;
        for (int i = 0; i < 3; ++i) {
            threads.emplace_back(simulateDbOperations, std::ref(dbManager));
        }

        for (auto& thread : threads) {
            thread.join();
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}