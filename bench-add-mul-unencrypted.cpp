#define PROFILE
#include <iostream>
#include <cstdint>
#include <vector>

#include "utils.h"
#include "openfhe.h"

using namespace lbcrypto;

int main()
{
  uint32_t numRuns = 1000;
  std::vector<ProfileData> profiles;

  uint32_t batchSize = (1 << 15);
  uint32_t seed = 42;
  std::vector<double> x1 = generateRandomDoubleVector(batchSize, seed);
  std::vector<double> x2 = generateRandomDoubleVector(batchSize, seed);

  ProfileData evalAddProfile;
  evalAddProfile.operationName = "Add UnEnc";
  {
    auto start = std::chrono::high_resolution_clock::now();
    auto cAdd = pointwiseAdd(x1, x2);
    auto end = std::chrono::high_resolution_clock::now();
    evalAddProfile.firstRunTime = std::chrono::duration<double, std::milli>(end - start).count();

    double sum = 0;
    for (uint32_t i = 1; i < numRuns; i++)
    {
      start = std::chrono::high_resolution_clock::now();
      cAdd = pointwiseAdd(x1, x2);
      end = std::chrono::high_resolution_clock::now();
      sum += std::chrono::duration<double, std::milli>(end - start).count();
    }
    evalAddProfile.avgTimeExcludingFirst = sum / (numRuns - 1);
    profiles.push_back(evalAddProfile);
  }

  ProfileData evalSubProfile;
  evalSubProfile.operationName = "Sub UnEnc";
  {
    auto start = std::chrono::high_resolution_clock::now();
    auto cSub = pointwiseSubtract(x1, x2);
    auto end = std::chrono::high_resolution_clock::now();
    evalSubProfile.firstRunTime = std::chrono::duration<double, std::milli>(end - start).count();

    double sum = 0;
    for (uint32_t i = 1; i < numRuns; i++)
    {
      start = std::chrono::high_resolution_clock::now();
      cSub = pointwiseSubtract(x1, x2);      end = std::chrono::high_resolution_clock::now();
      sum += std::chrono::duration<double, std::milli>(end - start).count();
    }
    evalSubProfile.avgTimeExcludingFirst = sum / (numRuns - 1);
    profiles.push_back(evalSubProfile);
  }

  ProfileData evalMultScalarProfile;
  evalMultScalarProfile.operationName = "Mult (scalar)";
  {
    auto start = std::chrono::high_resolution_clock::now();
    auto cScalar = scalarMultiply(x1, 4.0);
    auto end = std::chrono::high_resolution_clock::now();
    evalMultScalarProfile.firstRunTime = std::chrono::duration<double, std::milli>(end - start).count();

    double sum = 0;
    for (uint32_t i = 1; i < numRuns; i++)
    {
      start = std::chrono::high_resolution_clock::now();
      cScalar = scalarMultiply(x1, 4.0);
      end = std::chrono::high_resolution_clock::now();
      sum += std::chrono::duration<double, std::milli>(end - start).count();
    }
    evalMultScalarProfile.avgTimeExcludingFirst = sum / (numRuns - 1);
    profiles.push_back(evalMultScalarProfile);
  }

  ProfileData evalMultProfile;
  evalMultProfile.operationName = "Mult UnEnc";
  {
    auto start = std::chrono::high_resolution_clock::now();
    auto cMul = pointwiseMultiply(x1, x2);
    auto end = std::chrono::high_resolution_clock::now();
    evalMultProfile.firstRunTime = std::chrono::duration<double, std::milli>(end - start).count();

    double sum = 0;
    for (uint32_t i = 1; i < numRuns; i++)
    {
      start = std::chrono::high_resolution_clock::now();
      cMul = pointwiseMultiply(x1, x2);
      end = std::chrono::high_resolution_clock::now();
      sum += std::chrono::duration<double, std::milli>(end - start).count();
    }
    evalMultProfile.avgTimeExcludingFirst = sum / (numRuns - 1);
    profiles.push_back(evalMultProfile);
  }

  auto cAdd = pointwiseAdd(x1, x2);
  auto cSub = pointwiseSubtract(x1, x2);
  auto cScalar = scalarMultiply(x1, 4.0);
  auto cMul = pointwiseMultiply(x1, x2);

  std::cout.precision(8);
  std::cout << std::endl
            << "Results of unencrypted computations: " << std::endl;

  printDoubleVector(x1, "x1", 5);
  printDoubleVector(x2, "x2", 5);

  printDoubleVector(cAdd, "Add", 5);
  printDoubleVector(cSub, "Sub", 5);
  printDoubleVector(cScalar, "Scalar", 5);
  printDoubleVector(cMul, "Mul", 5);

  printProfileResults(profiles);

  return 0;
}