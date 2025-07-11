/*************************************************************************
 * Copyright (c) 2022 Advanced Micro Devices, Inc. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "EnvVars.hpp"
#include "CollectiveArgs.hpp"
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <unordered_map>

namespace RcclUnitTesting
{
  int const UT_SINGLE_PROCESS = (1<<0);
  int const UT_MULTI_PROCESS  = (1<<1);

  int getArchInfo(bool *isRightArch,  const char *gfx)
  {
    // Prepare parent->child pipe
    int pipefd[2];
    if (pipe(pipefd) == -1) {
      ERROR("Unable to create parent->child pipe for getting number of devices\n");
      return TEST_FAIL;
    }
    pid_t pid = fork();
    if (0 == pid) {
      bool isGfxTest = false;
      int dev;
      hipGetDeviceCount(&dev);
      for (int deviceId = 0; deviceId < dev; deviceId++) {
        char gcn[256];
        hipDeviceProp_t devProp;
        hipGetDeviceProperties(&devProp, deviceId);
        char *gcnArchNameToken = strtok(devProp.gcnArchName, ":");
        strcpy(gcn, gcnArchNameToken);
        if(std::strncmp(gfx, gcn, 5) == 0) {
          isGfxTest = true;
        } else {
          isGfxTest = false;
          break;
        }
      }
      if (write(pipefd[1], &isGfxTest, sizeof(isGfxTest)) != sizeof(isGfxTest)) return TEST_FAIL;
      close(pipefd[0]);
      close(pipefd[1]);
      exit(EXIT_SUCCESS);
    }
    else {
      int status;
      if (read(pipefd[0], isRightArch, sizeof(*isRightArch)) != sizeof(*isRightArch)) return TEST_FAIL;
      waitpid(pid, &status, 0);
      assert(!status);
      close(pipefd[0]);
      close(pipefd[1]);
    }
    return TEST_SUCCESS;
  }

  int getDeviceCount(int *devices)
  {
    // Prepare parent->child pipe
    int pipefd[2];
    if (pipe(pipefd) == -1)
    {
      ERROR("Unable to create parent->child pipe for getting number of devices\n");
      return TEST_FAIL;
    }
    pid_t pid = fork();
    if (0 == pid)
    {
      int dev;
      hipGetDeviceCount(&dev);
      if (write(pipefd[1], &dev, sizeof(dev)) != sizeof(dev)) return TEST_FAIL;
      close(pipefd[0]);
      close(pipefd[1]);
      _exit(EXIT_SUCCESS);
    }
    else
    {
      int status;
      if (read(pipefd[0], devices, sizeof(*devices)) != sizeof(*devices)) return TEST_FAIL;
      waitpid(pid, &status, 0);
      assert(!status);
      close(pipefd[0]);
      close(pipefd[1]);
    }
    return TEST_SUCCESS;
  }

  int getDeviceMode (bool *cpxMode){
    // Prepare parent->child pipe
    int pipefd[2];
    if (pipe(pipefd) == -1)
    {
      ERROR("Unable to create parent->child pipe for getting the device mode\n");
      return TEST_FAIL;
    }
    pid_t pid = fork();
    if (0 == pid)
    {
      bool isCpxMode = false;
      int numDeviceCUs;
      int deviceIdx = 0;
      hipDeviceGetAttribute(&numDeviceCUs, hipDeviceAttributeMultiprocessorCount, deviceIdx);
      if(numDeviceCUs == 20 || numDeviceCUs == 38) isCpxMode = true;
      if (write(pipefd[1], &isCpxMode, sizeof(isCpxMode)) != sizeof(isCpxMode)) return TEST_FAIL;
      close(pipefd[0]);
      close(pipefd[1]);
      exit(EXIT_SUCCESS);
    }
    else {
      int status;
      if (read(pipefd[0], cpxMode, sizeof(*cpxMode)) != sizeof(*cpxMode)) return TEST_FAIL;
      waitpid(pid, &status, 0);
      assert(!status);
      close(pipefd[0]);
      close(pipefd[1]);
    }
    return TEST_SUCCESS;
    return 0;
  }

  ncclResult_t busIdToInt64(const char* busId, int64_t* id) {
    char hexStr[17];  // Longest possible int64 hex string + null terminator.
    int hexOffset = 0;
    for (int i = 0; hexOffset < sizeof(hexStr) - 1; i++) {
      char c = busId[i];
      if (c == ':') continue;
      if (c == '.') break; //ignore everything after . as they belong to same physical pci
      if ((c >= '0' && c <= '9') ||
          (c >= 'A' && c <= 'F') ||
          (c >= 'a' && c <= 'f')) {
        hexStr[hexOffset++] = busId[i];
      } else break;
    }
    hexStr[hexOffset] = '\0';
    *id = strtol(hexStr, NULL, 16);
    return ncclSuccess;
  }

  int getDevicePriority (std::vector<int> *gpuPriorityOrder){
    // Prepare parent->child pipe
    int pipefd[2];
    if (pipe(pipefd) == -1) {
      ERROR("Unable to create parent->child pipe for getting the device priority vector.\n");
      return TEST_FAIL;
    }
    pid_t pid = fork();
    if (0 == pid) {
      std::vector<int> result;
      try {
          int numDev;
          hipGetDeviceCount(&numDev);
          std::unordered_map<int64_t, std::vector<int>> uniqueIdToGpuIndexes;
          for(int dev=0;dev<numDev;dev++){
            char busIdStr[] = "00000000:00:00.0";
            int64_t busId;
            hipDeviceGetPCIBusId(busIdStr, sizeof(busIdStr), dev);
            busIdToInt64(busIdStr, &busId);
            uniqueIdToGpuIndexes[busId].push_back(dev);
          }
          std::vector<std::pair<int64_t, std::vector<int>>> sortedIds(uniqueIdToGpuIndexes.begin(), uniqueIdToGpuIndexes.end());
          std::sort(sortedIds.begin(), sortedIds.end(), [](const auto& a, const auto& b) {
              return a.second.size() > b.second.size();
          });
          for (const auto& pair : sortedIds) {
              result.insert(result.end(), pair.second.begin(), pair.second.end());
          }
      } catch (const std::exception& e) {
          std::cerr << "Error: " << e.what() << std::endl;
          return 1;
      }
      if (write(pipefd[1], result.data(), gpuPriorityOrder->size() * sizeof(int)) != gpuPriorityOrder->size() * sizeof(int)) return TEST_FAIL;
      close(pipefd[0]);
      close(pipefd[1]);
      exit(EXIT_SUCCESS);
    }
    else {
      int status;
      if (read(pipefd[0], gpuPriorityOrder->data(), gpuPriorityOrder->size() * sizeof(int)) != gpuPriorityOrder->size() * sizeof(int)) return TEST_FAIL;
      waitpid(pid, &status, 0);
      assert(!status);
      close(pipefd[0]);
      close(pipefd[1]);
    }
    return TEST_SUCCESS;
    return 0;
  }


  EnvVars::EnvVars()
  {
    // Collect number of GPUs available
    // NOTE: Cannot use HIP call prior to launching unless it is inside another child process
    numDetectedGpus = 0;
    getDeviceCount(&numDetectedGpus);
  }

  std::vector<ncclRedOp_t> const& EnvVars::GetAllSupportedRedOps()
  {
    return redOps;
  }

  std::vector<ncclDataType_t> const& EnvVars::GetAllSupportedDataTypes()
  {
    return dataTypes;
  }

  std::vector<int> const& EnvVars::GetNumGpusList()
  {
    return numGpusList;
  }

  std::vector<int> const& EnvVars::GetGpuPriorityOrder()
  {
    return gpuPriorityOrder;
  }

  std::vector<int> const& EnvVars::GetIsMultiProcessList()
  {
    return isMultiProcessList;
  }

  int EnvVars::GetEnvVar(std::string const varname, int defaultValue)
  {
    if (getenv(varname.c_str()))
      return atoi(getenv(varname.c_str()));
    return defaultValue;
  };

  std::vector<std::string> EnvVars::GetEnvVarsList(std::string const varname)
  {
    std::vector<std::string> result;
    if (getenv(varname.c_str()))
    {
      char* token = strtok(getenv(varname.c_str()), ",;");
      while (token != NULL)
      {
        result.push_back(token);
        token = strtok(NULL, ",;");
      }
    }
    return result;
  }

  void EnvVars::ShowConfig()
  {
    std::vector<std::tuple<std::string, int, std::string>> supported =
      {
        std::make_tuple("UT_DEBUG_PAUSE"      , debugPause    , "Pause for debugger attach"),
        std::make_tuple("UT_SHOW_NAMES"       , showNames     , "Show test case names"),
        std::make_tuple("UT_MIN_GPUS"         , minGpus       , "Minimum number of GPUs to use"),
        std::make_tuple("UT_MAX_GPUS"         , maxGpus       , "Maximum number of GPUs to use"),
        std::make_tuple("UT_POW2_GPUS"        , onlyPow2Gpus  , "Only allow power-of-2 # of GPUs"),
        std::make_tuple("UT_PROCESS_MASK"     , processMask   , "Whether to run single/multi process"),
        std::make_tuple("UT_VERBOSE"          , verbose       , "Show verbose unit test output"),
        std::make_tuple("UT_REDOPS"           , -1            , "List of reduction ops to test"),
        std::make_tuple("UT_DATATYPES"        , -1            , "List of datatypes to test"),
        std::make_tuple("UT_MAX_RANKS_PER_GPU", maxRanksPerGpu, "Maximum number of ranks using the same GPU"),
        std::make_tuple("UT_PRINT_VALUES"     , printValues   , "Print array values (-1 for all)"),
        std::make_tuple("UT_SHOW_TIMING"      , showTiming    , "Show timing table"),
        std::make_tuple("UT_INTERACTIVE"      , useInteractive, "Run in interactive mode"),
        std::make_tuple("UT_TIMEOUT_US"       , timeoutUs     , "Timeout limit for collective calls in us"),
        std::make_tuple("UT_MULTITHREAD"      , useMultithreading, "Multi-thread single-process ranks"),
      };

    printf("================================================================================\n");
    printf(" Environment variables:\n");
    for (auto p : supported)
    {
      printf(" - %-20s %-42s (%3d) %s\n", std::get<0>(p).c_str(), std::get<2>(p).c_str(), std::get<1>(p),
             getenv(std::get<0>(p).c_str()) ? getenv(std::get<0>(p).c_str()) : "<unset>");
    }
    printf("================================================================================\n");
  }
}
