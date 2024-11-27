#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <fstream>
#include <sstream>

// Class to handle individual user sessions
class Session {
public:
    Session(const std::string& userId) : userId_(userId) {
        startTime_ = std::chrono::system_clock::now();
    }

    void endSession() {
        endTime_ = std::chrono::system_clock::now();
        sessionActive_ = false;
    }

    std::string getSessionDetails() const {
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(endTime_ - startTime_).count();
        std::ostringstream details;
        details << "UserID: " << userId_
                << ", Start Time: " << formatTime(startTime_)
                << ", End Time: " << formatTime(endTime_)
                << ", Duration: " << duration << " seconds";
        return details.str();
    }

private:
    std::string userId_;
    std::chrono::system_clock::time_point startTime_;
    std::chrono::system_clock::time_point endTime_;
    bool sessionActive_ = true;

    std::string formatTime(std::chrono::system_clock::time_point timePoint) const {
        std::time_t time = std::chrono::system_clock::to_time_t(timePoint);
        std::ostringstream oss;
        oss << std::put_time(std::localtime(&time), "%Y-%m-%d %X");
        return oss.str();
    }
};

// Class to manage multiple user sessions
class SessionManager {
public:
    void startSession(const std::string& userId) {
        std::lock_guard<std::mutex> lock(mutex_);
        sessions_[userId] = std::make_shared<Session>(userId);
    }

    void endSession(const std::string& userId) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (sessions_.count(userId)) {
            sessions_[userId]->endSession();
            logSession(sessions_[userId]);
            sessions_.erase(userId);
        }
    }

    std::shared_ptr<Session> getSession(const std::string& userId) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (sessions_.count(userId)) {
            return sessions_[userId];
        }
        return nullptr;
    }

private:
    std::unordered_map<std::string, std::shared_ptr<Session>> sessions_;
    std::mutex mutex_;

    void logSession(const std::shared_ptr<Session>& session) {
        std::ofstream logFile("session.log", std::ios::app);
        if (logFile.is_open()) {
            logFile << session->getSessionDetails() << std::endl;
        }
    }
};

// Function to simulate user session management
void simulateSessionManagement(SessionManager& manager, const std::vector<std::string>& userIds) {
    // Start sessions
    for (const auto& userId : userIds) {
        manager.startSession(userId);
    }

    // Simulate some delay
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // End sessions
    for (const auto& userId : userIds) {
        manager.endSession(userId);
    }
}

int main() {
    SessionManager sessionManager;

    // List of user IDs for simulation
    std::vector<std::string> userIds = {
        "user1",
        "user2",
        "user3",
    };

    // Simulate session management in multiple threads
    std::vector<std::thread> threads;
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back(simulateSessionManagement, std::ref(sessionManager), std::ref(userIds));
    }

    for (auto& thread : threads) {
        thread.join();
    }

    return 0;
}