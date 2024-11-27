#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <memory>
#include <iomanip>
#include <sstream>
#include <nlohmann/json.hpp>
#include <curl/curl.h>

using json = nlohmann::json;
std::mutex io_mutex;

// A class for handling HTTP requests
class HttpClient {
public:
    HttpClient() {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl_handle = curl_easy_init();
    }

    ~HttpClient() {
        curl_easy_cleanup(curl_handle);
        curl_global_cleanup();
    }

    std::string get(const std::string& url) {
        curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &response_);
        curl_easy_perform(curl_handle);
        return response_;
    }

private:
    CURL* curl_handle;
    std::string response_;

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }
};

// A function to read configuration from a JSON file
json readConfig(const std::string& filename) {
    std::ifstream config_file(filename);
    if (!config_file.is_open()) {
        throw std::runtime_error("Could not open config file");
    }

    json config;
    config_file >> config;
    return config;
}

// A class representing a logger
class Logger {
public:
    enum class Level {
        INFO,
        WARNING,
        ERROR
    };

    Logger(const std::string& filename) : log_file_(filename, std::ios::out | std::ios::app) {
        if (!log_file_.is_open()) {
            throw std::runtime_error("Could not open log file");
        }
    }

    void log(const std::string& message, Level level = Level::INFO) {
        std::lock_guard<std::mutex> lock(io_mutex);
        log_file_ << "[" << currentDateTime() << "] [" << levelToString(level) << "] " << message << std::endl;
    }

private:
    std::ofstream log_file_;

    std::string levelToString(Level level) const {
        switch (level) {
            case Level::INFO: return "INFO";
            case Level::WARNING: return "WARNING";
            case Level::ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }

    std::string currentDateTime() const {
        std::time_t now = std::time(nullptr);
        std::tm tm;
        localtime_s(&tm, &now);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }
};

// A thread-safe function to perform an API call and log the response time
void performApiCallAndLog(const std::string& url, Logger& logger) {
    HttpClient client;
    auto start = std::chrono::high_resolution_clock::now();
    std::string response = client.get(url);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::lock_guard<std::mutex> lock(io_mutex);
    logger.log("API call to " + url + " took " + std::to_string(elapsed.count()) + " seconds", Logger::Level::INFO);

    // Parse JSON response (just a dummy parse to show complexity)
    try {
        auto json_response = json::parse(response);
        logger.log("Parsed JSON response successfully.", Logger::Level::INFO);
    } catch (const std::exception& e) {
        logger.log("Failed to parse JSON response: " + std::string(e.what()), Logger::Level::ERROR);
    }
}

int main() {
    try {
        Logger logger("bot.log");
        json config = readConfig("config.json");

        std::vector<std::string> urls = config["api_urls"];
        std::vector<std::thread> threads;
        for (const auto& url : urls) {
            threads.emplace_back(performApiCallAndLog, url, std::ref(logger));
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