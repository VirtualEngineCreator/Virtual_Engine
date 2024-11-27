#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <sstream>
#include <cctype>
#include <mutex>
#include <thread>
#include <shared_mutex>

// Enumeration for question types
enum class QuestionType {
    GREETING,
    INFORMATION,
    JOKE_REQUEST,
    UNKNOWN
};

// A class to manage question classification
class QuestionClassifier {
public:
    QuestionClassifier() {
        loadKeywords();
    }

    QuestionType classifyQuestion(const std::string& question) {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        std::string loweredQuestion = toLowerCase(question);
        auto tokens = tokenize(loweredQuestion);

        for (const auto& token : tokens) {
            if (greeting_keywords.find(token) != greeting_keywords.end()) {
                return QuestionType::GREETING;
            }
            if (information_keywords.find(token) != information_keywords.end()) {
                return QuestionType::INFORMATION;
            }
            if (joke_keywords.find(token) != joke_keywords.end()) {
                return QuestionType::JOKE_REQUEST;
            }
        }

        return QuestionType::UNKNOWN;
    }

    std::string questionTypeToString(QuestionType type) const {
        switch (type) {
            case QuestionType::GREETING: return "Greeting";
            case QuestionType::INFORMATION: return "Information";
            case QuestionType::JOKE_REQUEST: return "Joke Request";
            case QuestionType::UNKNOWN: return "Unknown";
        }
        return "Unknown";
    }

private:
    std::vector<std::string> tokenize(const std::string& text) {
        std::istringstream stream(text);
        std::string word;
        std::vector<std::string> tokens;
        while (stream >> word) {
            tokens.push_back(word);
        }
        return tokens;
    }

    std::string toLowerCase(const std::string& str) {
        std::string lower_str = str;
        std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(), [](unsigned char c) {
            return std::tolower(c);
        });
        return lower_str;
    }

    void loadKeywords() {
        greeting_keywords = {"hello", "hi", "hey"};
        information_keywords = {"what", "who", "how", "why", "where", "when"};
        joke_keywords = {"joke", "funny", "laugh"};
    }

    std::unordered_set<std::string> greeting_keywords;
    std::unordered_set<std::string> information_keywords;
    std::unordered_set<std::string> joke_keywords;
    std::shared_mutex mutex_;
};

// Function to simulate question classification
void simulateQuestionClassification(QuestionClassifier& classifier, const std::vector<std::string>& questions) {
    for (const auto& question : questions) {
        QuestionType type = classifier.classifyQuestion(question);
        std::cout << "Q: " << question << "\nType: " << classifier.questionTypeToString(type) << std::endl;
    }
}

int main() {
    QuestionClassifier classifier;

    // List of questions for simulation
    std::vector<std::string> questions = {
        "Hello",
        "How are you?",
        "What is your name?",
        "Tell me a joke",
        "Why is the sky blue?"
    };

    // Simulate the question classification in multiple threads
    std::vector<std::thread> threads;
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back(simulateQuestionClassification, std::ref(classifier), std::ref(questions));
    }

    for (auto& thread : threads) {
        thread.join();
    }

    return 0;
}