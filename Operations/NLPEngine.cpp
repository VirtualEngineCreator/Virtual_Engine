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

// A class to manage text-related operations
class NlpEngine {
public:
    NlpEngine() {
        loadPredefinedAnswers();
    }

    std::string answerQuestion(const std::string& question) {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        auto tokens = tokenize(question);
        std::string loweredQuestion = toLowerCase(question);
        if (predefined_answers.find(loweredQuestion) != predefined_answers.end()) {
            return predefined_answers[loweredQuestion];
        } else {
            return "I'm not sure about that.";
        }
    }

    void addPredefinedAnswer(const std::string& question, const std::string& answer) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        predefined_answers[toLowerCase(question)] = answer;
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

    void loadPredefinedAnswers() {
        predefined_answers["hello"] = "Hi there!";
        predefined_answers["how are you?"] = "I'm a bot, so I don't have feelings, but thanks for asking!";
        predefined_answers["what is your name?"] = "I am a virtual assistant.";
        predefined_answers["what can you do?"] = "I can help you with your questions and manage operations.";
    }

    std::unordered_map<std::string, std::string> predefined_answers;
    std::shared_mutex mutex_;
};

// Function to simulate interaction with the bot
void simulateBotInteraction(NlpEngine& nlpEngine, const std::vector<std::string>& questions) {
    for (const auto& question : questions) {
        std::string answer = nlpEngine.answerQuestion(question);
        std::cout << "Q: " << question << "\nA: " << answer << std::endl;
    }
}

int main() {
    NlpEngine nlpEngine;

    // Add more predefined answers
    nlpEngine.addPredefinedAnswer("what is the capital of france?", "The capital of France is Paris.");
    nlpEngine.addPredefinedAnswer("tell me a joke", "Why don't scientists trust atoms? Because they make up everything!");

    // List of questions for simulation
    std::vector<std::string> questions = {
        "Hello",
        "How are you?",
        "What is your name?",
        "What can you do?",
        "What is the capital of France?",
        "Tell me a joke",
        "What is the meaning of life?"
    };

    // Simulate the bot interaction in multiple threads
    std::vector<std::thread> threads;
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back(simulateBotInteraction, std::ref(nlpEngine), std::ref(questions));
    }

    for (auto& thread : threads) {
        thread.join();
    }

    return 0;
}