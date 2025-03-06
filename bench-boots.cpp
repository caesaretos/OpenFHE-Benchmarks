#define PROFILE

#include <iostream>
#include <cstdint>
#include <chrono>
#include <vector>
#include <iomanip>

#include "utils.h"
#include "openfhe.h"

using namespace lbcrypto;

int main()
{
  uint32_t numRuns = 2;
  std::vector<ProfileData> profiles;

  uint32_t scaleModSize = 59;
  uint32_t firstModSize = 60;
  uint32_t batchSize = (1 << 15);
  std::vector<uint32_t> levelBudget = {4, 4};
  SecretKeyDist secretKeyDist = UNIFORM_TERNARY;

  CCParams<CryptoContextCKKSRNS> parameters;
  parameters.SetFirstModSize(firstModSize);
  parameters.SetScalingModSize(scaleModSize);
  parameters.SetBatchSize(batchSize);
  parameters.SetSecurityLevel(HEStd_NotSet);
  parameters.SetRingDim(2 * batchSize);
  // parameters.SetKeySwitchTechnique(HYBRID);
  parameters.SetSecretKeyDist(secretKeyDist);

  uint32_t levelsAvailableAfterBootstrap = 3;
  usint depth = levelsAvailableAfterBootstrap + FHECKKSRNS::GetBootstrapDepth(levelBudget, secretKeyDist);
  std::cout << "Total circuit depth: " << depth << "\n";
  parameters.SetMultiplicativeDepth(depth);

  CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
  cc->Enable(PKE);
  cc->Enable(KEYSWITCH);
  cc->Enable(LEVELEDSHE);
  cc->Enable(ADVANCEDSHE);
  cc->Enable(FHE);

  std::cout << "\n\nNote this build is SINGLE-THREADED \n\n";
  std::cout << "CKKS scheme is using ring dimension " << cc->GetRingDimension() << std::endl
            << std::endl;

  cc->EvalBootstrapSetup(levelBudget);

  auto keys = cc->KeyGen();
  auto print_moduli_chain = [](const DCRTPoly &poly)
  {
    int num_primes = poly.GetNumOfElements();
    double total_bit_len = 0.0;
    for (int i = 0; i < num_primes; i++)
    {
      auto qi = poly.GetParams()->GetParams()[i]->GetModulus();
      std::cout << "q_" << i << ": "
                << qi
                << ",  log q_" << i << ": " << log(qi.ConvertToDouble()) / log(2)
                << std::endl;
      total_bit_len += log(qi.ConvertToDouble()) / log(2);
    }
    std::cout << "Total bit length: " << total_bit_len << std::endl;
  };
  const std::vector<DCRTPoly> &ckks_pk = keys.publicKey->GetPublicElements();
  std::cout << "Moduli chain of pk: " << std::endl;
  print_moduli_chain(ckks_pk[0]);

  cc->EvalMultKeyGen(keys.secretKey);
  cc->EvalBootstrapKeyGen(keys.secretKey, batchSize);

  uint32_t seed = 42;
  std::vector<double> x1 = generateRandomDoubleVector(batchSize, seed);

  Plaintext ptxt1 = cc->MakeCKKSPackedPlaintext(x1, 1, depth - 1);

  std::cout << "Input x1: " << ptxt1 << std::endl;

  auto c1 = cc->Encrypt(keys.publicKey, ptxt1);
  std::cout << "Initial number of levels remaining: " << depth - c1->GetLevel() << std::endl;

  ProfileData evalBootProfile;
  evalBootProfile.operationName = "EvalBootstrap";
  {
    double sum = 0;
    for (uint32_t i = 0; i < numRuns; i++)
    {
      auto start = std::chrono::high_resolution_clock::now();
      auto ciphertextAfter = cc->EvalBootstrap(c1);
      auto end = std::chrono::high_resolution_clock::now();
      sum += std::chrono::duration<double, std::milli>(end - start).count();
    }
    evalBootProfile.avgTimeExcludingFirst = sum / (numRuns);
    profiles.push_back(evalBootProfile);
  }

  auto ciphertextAfter = cc->EvalBootstrap(c1);

  Plaintext result;
  std::cout.precision(8);
  std::cout << std::endl
            << "Results of homomorphic computations: " << std::endl;

  cc->Decrypt(keys.secretKey, c1, &result);
  result->SetLength(batchSize);
  std::cout << "x1 = " << result;
  std::cout << "Estimated precision in bits: " << result->GetLogPrecision() << std::endl;

  cc->Decrypt(keys.secretKey, ciphertextAfter, &result);
  result->SetLength(batchSize);
  std::cout << "Bootstrapped x1 = " << result;
  std::cout << "Estimated precision in bits: " << result->GetLogPrecision() << std::endl;

  printProfileResults(profiles);

  return 0;
}