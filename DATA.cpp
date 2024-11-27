#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <thread>
#include <mutex>
#include <memory>
#include <ctime>
#include <sstream>
#include <iomanip>

// A class to manage data recording
class DataRecorder {
public:
    DataRecorder(const std::string& filename) : filename_(filename) {
        // Open the file in append mode
        file_.open(filename_, std::ios::out | std::ios::app);
        if (!file_.is_open()) {
            throw std::runtime_error("Could not open file for recording data");
        }
    }

    ~DataRecorder() {
        if (file_.is_open()) {
            file_.close();
        }
    }

    void recordData(const std::string& data) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::string timestampedData = getCurrentTimestamp() + " - " + data;
        file_ << timestampedData << std::endl;
        std::cout << "Recorded: " << timestampedData << std::endl;
    }

private:
    std::ofstream file_;
    std::string filename_;
    std::mutex mutex_;

    std::string getCurrentTimestamp() const {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);

        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
        return ss.str();
    }
};

// Function to simulate data recording
void simulateDataRecording(DataRecorder& recorder, const std::vector<std::string>& dataEntries) {
    for (const auto& data : dataEntries) {
        recorder.recordData(data);
    }
}

int main() {
    try {
        DataRecorder recorder("data.log");

        // List of data entries for simulation
        std::vector<std::string> dataEntries = {
            "User logged in",
            "User performed a search",
            "User logged out",
            "User updated profile",
            "User requested a password change"
        };

        // Simulate data recording in multiple threads
        std::vector<std::thread> threads;
        for (int i = 0; i < 3; ++i) {
            threads.emplace_back(simulateDataRecording, std::ref(recorder), std::ref(dataEntries));
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