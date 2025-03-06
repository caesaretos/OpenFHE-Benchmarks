#include "utils.h"
#include <random>
#include <cstdint>
#include <chrono>
#include <iostream>
#include <iomanip>


using namespace std::chrono;
using TimePoint = std::chrono::high_resolution_clock::time_point;

std::vector<double> generateRandomDoubleVector(size_t size, uint32_t seed)
{
  std::mt19937 gen(seed);
  std::uniform_real_distribution<double> dis(0.1, 5.0);

  std::vector<double> x(size);

  for (size_t i = 0; i < size; ++i) {
      x[i] = dis(gen);
  }

  return x;
}

using TimePoint = std::chrono::high_resolution_clock::time_point;

template<typename F, typename... Args>
ProfileData measureOperation(const std::string& opName, uint32_t numRuns, F&& func, Args&&... args) {
    ProfileData profile;
    profile.operationName = opName;
    
    if (numRuns == 0) {
        profile.firstRunTime = 0;
        profile.avgTimeExcludingFirst = 0;
        return profile;
    }
    
    std::vector<double> runTimes;
    runTimes.reserve(numRuns);
    
    for (uint32_t i = 0; i < numRuns; i++) {
        auto start = std::chrono::high_resolution_clock::now();
        auto result = func(std::forward<Args>(args)...);
        auto end = std::chrono::high_resolution_clock::now();
        
        double timeMs = std::chrono::duration<double, std::milli>(end - start).count();
        runTimes.push_back(timeMs);
        
        if (i == numRuns - 1) {
            return result;
        }
    }
    
    profile.firstRunTime = runTimes[0];
    
    double sum = 0;
    for (uint32_t i = 1; i < numRuns; i++) {
        sum += runTimes[i];
    }
    
    profile.avgTimeExcludingFirst = (numRuns > 1) ? sum / (numRuns - 1) : 0;
    return profile;
}

void printProfileResults(const std::vector<ProfileData>& profiles) {
    std::cout << "\n============ Performance Profiling Results ============\n";
    std::cout << std::left << std::setw(25) << "Operation"
              << std::right << std::setw(15) << "First Run (ms)"
              << std::right << std::setw(25) << "Avg (excl. first) (ms)"
              << std::right << std::setw(20) << "Memory Overhead (ms)" << std::endl;
    std::cout << std::string(85, '-') << std::endl;
    
    for (const auto& profile : profiles) {
        double memoryOverhead = profile.firstRunTime - profile.avgTimeExcludingFirst;
        std::cout << std::left << std::setw(25) << profile.operationName
                  << std::right << std::fixed << std::setprecision(3) << std::setw(15) << profile.firstRunTime
                  << std::right << std::fixed << std::setprecision(3) << std::setw(25) << profile.avgTimeExcludingFirst
                  << std::right << std::fixed << std::setprecision(3) << std::setw(20) << memoryOverhead << std::endl;
    }
    std::cout << std::string(85, '-') << std::endl;
}

std::vector<double> pointwiseAdd(const std::vector<double>& v1, const std::vector<double>& v2) {
  if (v1.size() != v2.size()) {
      throw std::invalid_argument("Vectors must have the same size.");
  }
  std::vector<double> result(v1.size());
  for (size_t i = 0; i < v1.size(); ++i) {
      result[i] = v1[i] + v2[i];
  }
  return result;
}

std::vector<double> pointwiseSubtract(const std::vector<double>& v1, const std::vector<double>& v2) {
  if (v1.size() != v2.size()) {
      throw std::invalid_argument("Vectors must have the same size.");
  }
  std::vector<double> result(v1.size());
  for (size_t i = 0; i < v1.size(); ++i) {
      result[i] = v1[i] - v2[i];
  }
  return result;
}

std::vector<double> pointwiseMultiply(const std::vector<double>& v1, const std::vector<double>& v2) {
  if (v1.size() != v2.size()) {
      throw std::invalid_argument("Vectors must have the same size.");
  }
  std::vector<double> result(v1.size());
  for (size_t i = 0; i < v1.size(); ++i) {
      result[i] = v1[i] * v2[i];
  }
  return result;
}

std::vector<double> scalarMultiply(const std::vector<double>& v, double scalar) {
  std::vector<double> result(v.size());
  for (size_t i = 0; i < v.size(); ++i) {
      result[i] = v[i] * scalar;
  }
  return result;
}

void printDoubleVector(const std::vector<double>& vec, const std::string& label, size_t numElements) {
  std::cout << label << ": [";

  size_t elementsToPrint = (numElements == 0 || numElements > vec.size()) ? vec.size() : numElements;

  for (size_t i = 0; i < elementsToPrint; ++i) {
      std::cout << std::fixed << std::setprecision(8) << vec[i]; // Adjust precision as needed
      if (i < elementsToPrint - 1) {
          std::cout << ", ";
      }
  }

  std::cout << "]" << std::endl;
}