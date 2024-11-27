#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <random>
#include <ctime>
#include <unordered_set>
#include <utility>

// Function to simulate a delay of 2 seconds
void delay() {
    std::this_thread::sleep_for(std::chrono::seconds(2));
}

// Function to get a random question and answer pair
std::pair<std::string, std::string> getRandomQuestionAndAnswer(std::unordered_set<int>& askedIndices) {
    std::vector<std::pair<std::string, std::string>> qaPairs = {
        {"What's the weather like today?", "It's sunny and warm!"},
        {"How do I improve my coding skills?", "Practice, practice, and more practice."},
        {"What's the best way to learn C++?", "Start with the basics and build projects."},
        {"Can you tell me a joke?", "Why did the scarecrow win an award? Because he was outstanding in his field!"},
        {"What's the meaning of life?", "42 is the answer to the ultimate question of life, the universe, and everything."},
        {"What is photosynthesis?", "Photosynthesis is the process used by plants to convert light energy into chemical energy."},
        {"How does gravity work?", "Gravity is a force by which a planet or other body draws objects toward its center."},
        {"Who was Albert Einstein?", "Albert Einstein was a theoretical physicist who developed the theory of relativity."},
        {"What is the speed of light?", "The speed of light in a vacuum is approximately 299,792 kilometers per second."},
        {"Can you explain quantum mechanics?", "Quantum mechanics is a fundamental theory in physics describing the properties of nature on an atomic scale."},
        {"What causes a rainbow?", "A rainbow is caused by reflection, refraction, and dispersion of light in water droplets."},
        {"What is DNA?", "DNA is a molecule that carries genetic instructions used in the growth, development, and functioning of all living organisms."},
        {"What is the Big Bang Theory?", "The Big Bang Theory is the prevailing cosmological model explaining the universe's origin from a singularity."},
        {"How does a computer work?", "A computer processes data through the use of integrated circuits, memory, storage, and input/output devices."},
        {"What are black holes?", "Black holes are regions of spacetime exhibiting gravitational acceleration so strong that nothing can escape from them."}
    };

    static std::mt19937 generator(static_cast<unsigned int>(std::time(nullptr)));

    std::uniform_int_distribution<int> distribution(0, qaPairs.size() - 1);
    int index;
    do {
        index = distribution(generator);
    } while (askedIndices.find(index) != askedIndices.end());

    askedIndices.insert(index);
    return qaPairs[index];
}

int main() {
    // Add two blank lines at the top
    std::cout << "\n\n";

    // Start-up effect
    std::cout << "Initializing Virtual Engine";
    for (int i = 0; i < 3; ++i) {
        std::cout << "." << std::flush;
        delay();
    }
    std::cout << "\rVirtual Engine Initialized! [*]\n" << std::endl;

    std::unordered_set<int> askedIndices;

    // Simulation loop
    while (askedIndices.size() < 15) {
        std::cout << "[*] Tweet: \"Ask me anything! [??]\"" << std::endl;
        delay();

        auto [question, answer] = getRandomQuestionAndAnswer(askedIndices);
        std::cout << "[?] User: \"" << question << "\" [?]" << std::endl;
        delay();

        std::cout << "[!] Virtual Engine: \"" << answer << "\" [!]" << std::endl;
        delay();

        std::cout << "-------------------------------------\n" << std::endl;
        delay();
    }

    std::cout << "All questions have been asked. Ending simulation." << std::endl;
    return 0;
}