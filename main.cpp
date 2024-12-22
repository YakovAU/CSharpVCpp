#include <iostream>
#include <thread>
#include <vector>
#include <random>
#include <chrono>
#include <cstring>
#include <cstdlib>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <openssl/sha.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#endif

// Constants
const int THREAD_COUNT = std::thread::hardware_concurrency();
const int MIN_ALLOCATION_SIZE = 1 * 1024 * 1024;  // 1 MB
const int MAX_ALLOCATION_SIZE = 20 * 1024 * 1024; // 20 MB
const int ITERATIONS = 500;

// Allocation Patterns
enum class AllocationType {
    Sequential,
    Random,
    Burst
};

struct AllocationPattern {
    std::string name;
    AllocationType type;
};

// Predefined Allocation Patterns
const std::vector<AllocationPattern> allocationPatterns = {
    {"Sequential", AllocationType::Sequential},
    {"Random", AllocationType::Random},
    {"Burst", AllocationType::Burst}
};

// Mutex for console output to prevent interleaving
std::mutex coutMutex;

// Function to calculate Fibonacci number (naïve recursive implementation)
unsigned long long naiveFibonacci(int n) {
    if (n <= 1) return n;
    return naiveFibonacci(n - 1) + naiveFibonacci(n - 2);
}

// Function to get allocation size
int getAllocationSize(std::mt19937 &rng) {
    std::uniform_int_distribution<int> dist(MIN_ALLOCATION_SIZE, MAX_ALLOCATION_SIZE);
    return dist(rng);
}

// Function to simulate allocation patterns
void simulateAllocationPattern(AllocationType type, std::mt19937 &rng) {
    switch (type) {
        case AllocationType::Sequential:
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            break;
        case AllocationType::Random:
            std::this_thread::sleep_for(std::chrono::milliseconds(rng() % 15 + 5)); // 5 to 20 ms
            break;
        case AllocationType::Burst:
            // No sleep for burst allocation
            break;
    }
}

// Function to perform matrix multiplication
void performMatrixMultiplication(int dimension) {
    // Initialize matrices with random values
    std::vector<std::vector<double>> matrixA(dimension, std::vector<double>(dimension));
    std::vector<std::vector<double>> matrixB(dimension, std::vector<double>(dimension));
    std::vector<std::vector<double>> result(dimension, std::vector<double>(dimension, 0.0));

    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    for(int i = 0; i < dimension; ++i) {
        for(int j = 0; j < dimension; ++j) {
            matrixA[i][j] = dist(rng);
            matrixB[i][j] = dist(rng);
        }
    }

    // Perform matrix multiplication
    for(int i = 0; i < dimension; ++i) {
        for(int j = 0; j < dimension; ++j) {
            for(int k = 0; k < dimension; ++k) {
                result[i][j] += matrixA[i][k] * matrixB[k][j];
            }
        }
    }

    // Calculate checksum to prevent optimization
    double checksum = 0.0;
    for(int i = 0; i < dimension; ++i) {
        for(int j = 0; j < dimension; ++j) {
            checksum += result[i][j];
        }
    }

    if(checksum == -1.0) { // Unlikely condition to prevent optimization
        std::lock_guard<std::mutex> lock(coutMutex);
        std::cout << "Matrix multiplication checksum: " << checksum << std::endl;
    }
}

// Function to perform SHA-256 hash
void performSha256Hash(unsigned char* buffer, int size) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(buffer, size, hash);

    // Calculate checksum to prevent optimization
    unsigned long long checksum = 0;
    for(unsigned char i : hash) {
        checksum += i;
    }

    if(checksum == ULLONG_MAX) { // Unlikely condition to prevent optimization
        std::lock_guard<std::mutex> lock(coutMutex);
        std::cout << "SHA-256 checksum: " << checksum << std::endl;
    }
}

// Function to perform advanced CPU tasks
void performAdvancedCpuTasks(unsigned char* buffer, int size, std::mt19937 &rng) {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    double random = dist(rng);
    
    if(random < 0.33) {
        performMatrixMultiplication(300); // 300x300 matrices
    } else if(random < 0.66) {
        performSha256Hash(buffer, size);
    } else {
        // Calculate Fibonacci number (using a smaller n to avoid excessive runtime)
        unsigned long long fibResult = naiveFibonacci(35);
        if(fibResult == 0) { // Unlikely condition to prevent optimization
            std::lock_guard<std::mutex> lock(coutMutex);
            std::cout << "Fibonacci result: " << fibResult << std::endl;
        }
    }
}

// Thread work function
void threadWork(int threadId) {
    // Initialize random number generator with thread-specific seed
    std::mt19937 rng(threadId + std::random_device{}());

    // Select allocation pattern randomly
    AllocationPattern currentPattern = allocationPatterns[rng() % allocationPatterns.size()];

    {
        std::lock_guard<std::mutex> lock(coutMutex);
        std::cout << "Thread " << threadId << " started with Allocation Pattern: " 
                  << currentPattern.name << std::endl;
    }

    for(int i = 0; i < ITERATIONS; ++i) {
        int allocationSize = getAllocationSize(rng);
        unsigned char* buffer = nullptr;

        try {
            // Allocate memory manually
            buffer = static_cast<unsigned char *>(std::malloc(allocationSize));
            if(!buffer) {
                throw std::bad_alloc();
            }

            // Initialize buffer with random data
            std::uniform_int_distribution<int> byteDist(0, 255);
            for(int j = 0; j < allocationSize; ++j) {
                buffer[j] = static_cast<unsigned char>(byteDist(rng));
            }

            // Perform advanced CPU tasks
            performAdvancedCpuTasks(buffer, allocationSize, rng);
        }
        catch(const std::exception &e) {
            std::lock_guard<std::mutex> lock(coutMutex);
            std::cerr << "Thread " << threadId << " encountered an error: " 
                      << e.what() << std::endl;
        }
        finally:
        {
            // Free allocated memory
            if(buffer) {
                std::free(buffer);
            }
        }

        // Simulate allocation pattern
        simulateAllocationPattern(currentPattern.type, rng);
    }

    {
        std::lock_guard<std::mutex> lock(coutMutex);
        std::cout << "Thread " << threadId << " completed." << std::endl;
    }
}

int main() {
    std::cout << "Starting Enhanced Performance Benchmark...\n" << std::endl;

    // Record start time
    auto startTime = std::chrono::high_resolution_clock::now();

    // Launch worker threads
    std::vector<std::thread> threads;
    for(int i = 0; i < THREAD_COUNT; ++i) {
        threads.emplace_back(threadWork, i);
    }

    // Wait for all worker threads to finish
    for(auto &t : threads) {
        if(t.joinable())
            t.join();
    }

    // Record end time
    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = endTime - startTime;

    std::cout << "\nTotal Execution Time: " << elapsed.count() << " seconds" << std::endl;
    std::cout << "Benchmark Completed." << std::endl;

    return 0;
}