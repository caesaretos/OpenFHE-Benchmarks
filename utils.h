#ifndef UTILS_H
#define UTILS_H

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>

std::vector<double> generateRandomDoubleVector(size_t size, uint32_t seed);
template<typename Func>
std::pair<double, double> measureOperation(Func&& func, int num_runs);

struct ProfileData {
    double firstRunTime;
    double avgTimeExcludingFirst;
    std::string operationName;
};

template<typename F, typename... Args>
ProfileData measureOperation(const std::string& opName, uint32_t numRuns, F&& func, Args&&... args);

void printProfileResults(const std::vector<ProfileData>& profiles);

std::vector<double> pointwiseAdd(const std::vector<double>& v1, const std::vector<double>& v2);
std::vector<double> pointwiseSubtract(const std::vector<double>& v1, const std::vector<double>& v2);
std::vector<double> pointwiseMultiply(const std::vector<double>& v1, const std::vector<double>& v2);
std::vector<double> scalarMultiply(const std::vector<double>& v, double scalar);

void printDoubleVector(const std::vector<double>& vec, const std::string& label, size_t numElements = 0);

#endif  // UTILS_H