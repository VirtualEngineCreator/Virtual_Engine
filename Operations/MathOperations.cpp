   #include <iostream>
   #include <vector>
   #include <algorithm>
   #include <memory>
   #include <thread>
   #include <mutex>
   #include <numeric>
   #include <future>
   #include <cmath>

   // A class representing a complex mathematical operation
   class ComplexOperation {
   public:
       ComplexOperation(int data) : data_(data) {}

       // Function to simulate some complex computation
       void performOperation() {
           std::lock_guard<std::mutex> lock(mutex_);
           data_ = std::pow(data_, 3) + std::sqrt(data_);
       }

       int getData() const {
           std::lock_guard<std::mutex> lock(mutex_);
           return data_;
       }

   private:
       int data_;
       mutable std::mutex mutex_;
   };

   // Template function to process elements in a list with a given operation
   template<typename T, typename Func>
   std::vector<T> processList(const std::vector<T>& list, Func func) {
       std::vector<T> result;
       result.reserve(list.size());

       // Process each element using the provided function
       std::transform(list.begin(), list.end(), std::back_inserter(result), func);

       return result;
   }

   // Helper function to create a list of complex operations
   std::vector<std::unique_ptr<ComplexOperation>> createComplexOperationsList(int size) {
       std::vector<std::unique_ptr<ComplexOperation>> list;
       for (int i = 1; i <= size; ++i) {
           list.push_back(std::make_unique<ComplexOperation>(i));
       }
       return list;
   }

   // Function to execute a list of threads
   template<typename Func>
   void executeInThreads(Func func, int threadCount) {
       std::vector<std::thread> threads;
       for (int i = 0; i < threadCount; ++i) {
           threads.emplace_back(func);
       }
       for (auto& thread : threads) {
           thread.join();
       }
   }

   int main() {
       // Create a list of complex operations
       auto operations = createComplexOperationsList(10);

       // Execute all operations in separate threads
       executeInThreads([&]() {
           for (auto& operation : operations) {
               operation->performOperation();
           }
       }, 4);

       // Process the results
       auto results = processList(operations, [](const std::unique_ptr<ComplexOperation>& op) {
           return op->getData();
       });

       // Summarize the results
       int sum = std::accumulate(results.begin(), results.end(), 0);
       std::cout << "The sum of the complex operations is: " << sum << std::endl;

       return 0;
   }