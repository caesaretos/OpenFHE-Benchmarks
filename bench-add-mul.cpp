#define PROFILE
#include <iostream>
#include <cstdint>
#include <vector>

#include "utils.h"
#include "openfhe.h"

using namespace lbcrypto;

int main()
{
  uint32_t numRuns = 100;
  std::vector<ProfileData> profiles;

  uint32_t multDepth = 10;
  uint32_t scaleModSize = 59;
  uint32_t firstModSize = 60;
  uint32_t batchSize = (1 << 15);

  CCParams<CryptoContextCKKSRNS> parameters;
  parameters.SetMultiplicativeDepth(multDepth);
  parameters.SetFirstModSize(firstModSize);
  parameters.SetScalingModSize(scaleModSize);
  parameters.SetBatchSize(batchSize);
  parameters.SetSecurityLevel(HEStd_NotSet);
  parameters.SetRingDim(2 * batchSize);
  // parameters.SetKeySwitchTechnique(HYBRID);

  CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
  cc->Enable(PKE);
  cc->Enable(KEYSWITCH);
  cc->Enable(LEVELEDSHE);

  std::cout << "\n\nNote this build is SINGLE-THREADED \n\n";
  std::cout << "CKKS scheme is using ring dimension " << cc->GetRingDimension() << std::endl
            << std::endl;

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
  cc->EvalRotateKeyGen(keys.secretKey, {1, -2});

  uint32_t seed = 42;
  std::vector<double> x1 = generateRandomDoubleVector(batchSize, seed);
  std::vector<double> x2 = generateRandomDoubleVector(batchSize, seed);

  ProfileData makePlaintextProfile;
  makePlaintextProfile.operationName = "MakeCKKSPackedPlaintext";
  {
    auto start = std::chrono::high_resolution_clock::now();
    Plaintext ptxt1 = cc->MakeCKKSPackedPlaintext(x1);
    auto end = std::chrono::high_resolution_clock::now();
    makePlaintextProfile.firstRunTime = std::chrono::duration<double, std::milli>(end - start).count();

    double sum = 0;
    for (uint32_t i = 1; i < numRuns; i++)
    {
      start = std::chrono::high_resolution_clock::now();
      ptxt1 = cc->MakeCKKSPackedPlaintext(x1);
      end = std::chrono::high_resolution_clock::now();
      sum += std::chrono::duration<double, std::milli>(end - start).count();
    }
    makePlaintextProfile.avgTimeExcludingFirst = sum / (numRuns - 1);
    profiles.push_back(makePlaintextProfile);
  }

  Plaintext ptxt1 = cc->MakeCKKSPackedPlaintext(x1);
  Plaintext ptxt2 = cc->MakeCKKSPackedPlaintext(x2);

  std::cout << "Input x1: " << ptxt1 << std::endl;
  std::cout << "Input x2: " << ptxt2 << std::endl;

  ProfileData encryptProfile;
  encryptProfile.operationName = "Encrypt";
  {
    auto start = std::chrono::high_resolution_clock::now();
    auto c1 = cc->Encrypt(keys.publicKey, ptxt1);
    auto end = std::chrono::high_resolution_clock::now();
    encryptProfile.firstRunTime = std::chrono::duration<double, std::milli>(end - start).count();

    double sum = 0;
    for (uint32_t i = 1; i < numRuns; i++)
    {
      start = std::chrono::high_resolution_clock::now();
      c1 = cc->Encrypt(keys.publicKey, ptxt1);
      end = std::chrono::high_resolution_clock::now();
      sum += std::chrono::duration<double, std::milli>(end - start).count();
    }
    encryptProfile.avgTimeExcludingFirst = sum / (numRuns - 1);
    profiles.push_back(encryptProfile);
  }

  auto c1 = cc->Encrypt(keys.publicKey, ptxt1);
  auto c2 = cc->Encrypt(keys.publicKey, ptxt2);

  ProfileData evalAddProfile;
  evalAddProfile.operationName = "EvalAdd";
  {
    auto start = std::chrono::high_resolution_clock::now();
    auto cAdd = cc->EvalAdd(c1, c2);
    auto end = std::chrono::high_resolution_clock::now();
    evalAddProfile.firstRunTime = std::chrono::duration<double, std::milli>(end - start).count();

    double sum = 0;
    for (uint32_t i = 1; i < numRuns; i++)
    {
      start = std::chrono::high_resolution_clock::now();
      cAdd = cc->EvalAdd(c1, c2);
      end = std::chrono::high_resolution_clock::now();
      sum += std::chrono::duration<double, std::milli>(end - start).count();
    }
    evalAddProfile.avgTimeExcludingFirst = sum / (numRuns - 1);
    profiles.push_back(evalAddProfile);
  }

  ProfileData evalSubProfile;
  evalSubProfile.operationName = "EvalSub";
  {
    auto start = std::chrono::high_resolution_clock::now();
    auto cSub = cc->EvalSub(c1, c2);
    auto end = std::chrono::high_resolution_clock::now();
    evalSubProfile.firstRunTime = std::chrono::duration<double, std::milli>(end - start).count();

    double sum = 0;
    for (uint32_t i = 1; i < numRuns; i++)
    {
      start = std::chrono::high_resolution_clock::now();
      cSub = cc->EvalSub(c1, c2);
      end = std::chrono::high_resolution_clock::now();
      sum += std::chrono::duration<double, std::milli>(end - start).count();
    }
    evalSubProfile.avgTimeExcludingFirst = sum / (numRuns - 1);
    profiles.push_back(evalSubProfile);
  }

  ProfileData evalMultScalarProfile;
  evalMultScalarProfile.operationName = "EvalMult (scalar)";
  {
    auto start = std::chrono::high_resolution_clock::now();
    auto cScalar = cc->EvalMult(c1, 4.0);
    auto end = std::chrono::high_resolution_clock::now();
    evalMultScalarProfile.firstRunTime = std::chrono::duration<double, std::milli>(end - start).count();

    double sum = 0;
    for (uint32_t i = 1; i < numRuns; i++)
    {
      start = std::chrono::high_resolution_clock::now();
      cScalar = cc->EvalMult(c1, 4.0);
      end = std::chrono::high_resolution_clock::now();
      sum += std::chrono::duration<double, std::milli>(end - start).count();
    }
    evalMultScalarProfile.avgTimeExcludingFirst = sum / (numRuns - 1);
    profiles.push_back(evalMultScalarProfile);
  }

  ProfileData evalMultProfile;
  evalMultProfile.operationName = "EvalMult (ciphertext)";
  {
    auto start = std::chrono::high_resolution_clock::now();
    auto cMul = cc->EvalMult(c1, c2);
    auto end = std::chrono::high_resolution_clock::now();
    evalMultProfile.firstRunTime = std::chrono::duration<double, std::milli>(end - start).count();

    double sum = 0;
    for (uint32_t i = 1; i < numRuns; i++)
    {
      start = std::chrono::high_resolution_clock::now();
      cMul = cc->EvalMult(c1, c2);
      end = std::chrono::high_resolution_clock::now();
      sum += std::chrono::duration<double, std::milli>(end - start).count();
    }
    evalMultProfile.avgTimeExcludingFirst = sum / (numRuns - 1);
    profiles.push_back(evalMultProfile);
  }

  ProfileData evalMultNoRelinProfile;
  evalMultNoRelinProfile.operationName = "EvalMultNoRelin";
  {
    auto start = std::chrono::high_resolution_clock::now();
    auto cMulNoRelin = cc->EvalMultNoRelin(c1, c2);
    auto end = std::chrono::high_resolution_clock::now();
    evalMultNoRelinProfile.firstRunTime = std::chrono::duration<double, std::milli>(end - start).count();

    double sum = 0;
    for (uint32_t i = 1; i < numRuns; i++)
    {
      start = std::chrono::high_resolution_clock::now();
      cMulNoRelin = cc->EvalMultNoRelin(c1, c2);
      end = std::chrono::high_resolution_clock::now();
      sum += std::chrono::duration<double, std::milli>(end - start).count();
    }
    evalMultNoRelinProfile.avgTimeExcludingFirst = sum / (numRuns - 1);
    profiles.push_back(evalMultNoRelinProfile);
  }

  auto cMulNoRelin = cc->EvalMultNoRelin(c1, c2);

  ProfileData relinearizeProfile;
  relinearizeProfile.operationName = "Relinearize";
  {
    auto start = std::chrono::high_resolution_clock::now();
    auto cMulRelined = cc->Relinearize(cMulNoRelin);
    auto end = std::chrono::high_resolution_clock::now();
    relinearizeProfile.firstRunTime = std::chrono::duration<double, std::milli>(end - start).count();

    double sum = 0;
    for (uint32_t i = 1; i < numRuns; i++)
    {
      start = std::chrono::high_resolution_clock::now();
      cMulRelined = cc->Relinearize(cMulNoRelin);
      end = std::chrono::high_resolution_clock::now();
      sum += std::chrono::duration<double, std::milli>(end - start).count();
    }
    relinearizeProfile.avgTimeExcludingFirst = sum / (numRuns - 1);
    profiles.push_back(relinearizeProfile);
  }

  ProfileData evalRotateProfile1;
  evalRotateProfile1.operationName = "EvalRotate (1)";
  {
    auto start = std::chrono::high_resolution_clock::now();
    auto cRot1 = cc->EvalRotate(c1, 1);
    auto end = std::chrono::high_resolution_clock::now();
    evalRotateProfile1.firstRunTime = std::chrono::duration<double, std::milli>(end - start).count();

    double sum = 0;
    for (uint32_t i = 1; i < numRuns; i++)
    {
      start = std::chrono::high_resolution_clock::now();
      cRot1 = cc->EvalRotate(c1, 1);
      end = std::chrono::high_resolution_clock::now();
      sum += std::chrono::duration<double, std::milli>(end - start).count();
    }
    evalRotateProfile1.avgTimeExcludingFirst = sum / (numRuns - 1);
    profiles.push_back(evalRotateProfile1);
  }

  ProfileData evalRotateProfile2;
  evalRotateProfile2.operationName = "EvalRotate (-2)";
  {
    auto start = std::chrono::high_resolution_clock::now();
    auto cRot2 = cc->EvalRotate(c1, -2);
    auto end = std::chrono::high_resolution_clock::now();
    evalRotateProfile2.firstRunTime = std::chrono::duration<double, std::milli>(end - start).count();

    double sum = 0;
    for (uint32_t i = 1; i < numRuns; i++)
    {
      start = std::chrono::high_resolution_clock::now();
      cRot2 = cc->EvalRotate(c1, -2);
      end = std::chrono::high_resolution_clock::now();
      sum += std::chrono::duration<double, std::milli>(end - start).count();
    }
    evalRotateProfile2.avgTimeExcludingFirst = sum / (numRuns - 1);
    profiles.push_back(evalRotateProfile2);
  }

  auto cAdd = cc->EvalAdd(c1, c2);
  auto cSub = cc->EvalSub(c1, c2);
  auto cScalar = cc->EvalMult(c1, 4.0);
  auto cMul = cc->EvalMult(c1, c2);
  auto cMulRelined = cc->Relinearize(cMulNoRelin);
  auto cRot1 = cc->EvalRotate(c1, 1);
  auto cRot2 = cc->EvalRotate(c1, -2);

  ProfileData decryptProfile;
  decryptProfile.operationName = "Decrypt";
  {
    auto start = std::chrono::high_resolution_clock::now();
    Plaintext result;
    cc->Decrypt(keys.secretKey, c1, &result);
    auto end = std::chrono::high_resolution_clock::now();
    encryptProfile.firstRunTime = std::chrono::duration<double, std::milli>(end - start).count();

    double sum = 0;
    for (uint32_t i = 1; i < numRuns; i++)
    {
      start = std::chrono::high_resolution_clock::now();
      cc->Decrypt(keys.secretKey, c1, &result);
      end = std::chrono::high_resolution_clock::now();
      sum += std::chrono::duration<double, std::milli>(end - start).count();
    }
    decryptProfile.avgTimeExcludingFirst = sum / (numRuns - 1);
    profiles.push_back(decryptProfile);
  }

  Plaintext result;
  std::cout.precision(8);
  std::cout << std::endl
            << "Results of homomorphic computations: " << std::endl;

  cc->Decrypt(keys.secretKey, c1, &result);
  result->SetLength(batchSize);
  std::cout << "x1 = " << result;
  std::cout << "Estimated precision in bits: " << result->GetLogPrecision() << std::endl;

  cc->Decrypt(keys.secretKey, cAdd, &result);
  result->SetLength(batchSize);
  std::cout << "x1 + x2 = " << result;
  std::cout << "Estimated precision in bits: " << result->GetLogPrecision() << std::endl;

  cc->Decrypt(keys.secretKey, cSub, &result);
  result->SetLength(batchSize);
  std::cout << "x1 - x2 = " << result << std::endl;

  cc->Decrypt(keys.secretKey, cScalar, &result);
  result->SetLength(batchSize);
  std::cout << "4 * x1 = " << result << std::endl;

  cc->Decrypt(keys.secretKey, cMul, &result);
  result->SetLength(batchSize);
  std::cout << "x1 * x2 = " << result << std::endl;

  cc->Decrypt(keys.secretKey, cRot1, &result);
  result->SetLength(batchSize);
  std::cout << std::endl
            << "In rotations, very small outputs (~10^-10 here) correspond to 0's:" << std::endl;
  std::cout << "x1 rotate by 1 = " << result << std::endl;

  cc->Decrypt(keys.secretKey, cRot2, &result);
  result->SetLength(batchSize);
  std::cout << "x1 rotate by -2 = " << result << std::endl;

  printProfileResults(profiles);

  return 0;
}