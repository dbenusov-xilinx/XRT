// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2022 Xilinx, Inc
// Copyright (C) 2023 Advanced Micro Devices, Inc. All rights reserved.

#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
// XRT includes
#include "experimental/xrt_system.h"
#include "xrt/xrt_device.h"
#include "xrt/xrt_kernel.h"
#include "xrt/xrt_bo.h"
static const int COUNT = 1024;

static std::string ps_kernel_path = "/lib/firmware/xilinx/ps_kernels/";

static void printHelp() {
    std::cout << "usage: %s <options>\n";
    std::cout << "  -p <path>\n";
    std::cout << "  -d <device> \n";
    std::cout << "  -s <supported>\n";
    std::cout << "  -h <help>\n";
}

static int
validate_binary_file(const std::string& binaryfile, bool print = false)
{
    std::ifstream infile(binaryfile);
    if (!infile.good()) {
        if (print)
            std::cout << "\nNOT SUPPORTED" << std::endl;
        return EOPNOTSUPP;
    } else {
        if (print)
            std::cout << "\nSUPPORTED" << std::endl;
        return EXIT_SUCCESS;
    }
}

static int
load_dependencies(  xrt::device& device,
                    const std::string& test_path,
                    const std::string& ps_kernel_name,
                    std::map<std::string, xrt::uuid>& uuid_map)
{
    std::string dependency_name = "test_dependencies.json";
    std::string dependency_json = ps_kernel_path + dependency_name;

    try {
        boost::property_tree::ptree load_ptree_root;
        boost::property_tree::read_json(dependency_json, load_ptree_root);

        auto ps_kernels = load_ptree_root.get_child("ps_kernel_mappings");
        for (const auto& ps_kernel : ps_kernels) {
            boost::property_tree::ptree ps_kernel_pt = ps_kernel.second;
            if (!boost::equals(ps_kernel_name, ps_kernel_pt.get<std::string>("name")))
                continue;

            auto dependencies = ps_kernel_pt.get_child("dependencies");
            for (const auto& dependency : dependencies) {
                std::string dependency_name = dependency.second.get_value<std::string>();
                std::string binaryfile = test_path + dependency_name;
                auto retVal = validate_binary_file(binaryfile);
                if (retVal != EXIT_SUCCESS)
                    return retVal;
                auto uuid = device.load_xclbin(binaryfile);
                uuid_map.emplace(dependency_name, uuid);
            }
        }
    } catch (const std::exception& e) {
        std::string msg("ERROR: Bad JSON format detected while marshaling dependency metadata (");
        msg += e.what();
        msg += ").";
        std::cerr << msg << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
    std::string dev_id = "0";
    std::string test_path;
    std::string b_file = "ps_validate.xclbin";
    bool flag_s = false;
    for (int i = 1; i < argc; i++) {
        if ((strcmp(argv[i], "-p") == 0) || (strcmp(argv[i], "--path") == 0)) {
            test_path = argv[i + 1];
        } else if ((strcmp(argv[i], "-d") == 0) || (strcmp(argv[i], "--device") == 0)) {
            dev_id = argv[i + 1];
        } else if ((strcmp(argv[i], "-s") == 0) || (strcmp(argv[i], "--supported") == 0)) {
            flag_s = true;
        } else if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0)) {
            printHelp();
            return 1;
        }
    }

    if (test_path.empty()) {
        std::cout << "ERROR : please provide the platform test path to -p option\n";
        return EXIT_FAILURE;
    }

    auto num_devices = xrt::system::enumerate_devices();

    auto device = xrt::device {dev_id};

    std::map<std::string, xrt::uuid> uuid_map;
    auto dependencyLoad = load_dependencies(device, test_path, b_file, uuid_map);
    if (dependencyLoad != EXIT_SUCCESS)
        return dependencyLoad;

    // Load ps kernel onto device
    std::string binaryfile = ps_kernel_path + b_file;
    auto retVal = validate_binary_file(binaryfile, flag_s);
    if (flag_s || retVal != EXIT_SUCCESS)
        return retVal;

    auto uuid = device.load_xclbin(binaryfile);
    auto hello_world = xrt::kernel(device, uuid.get(), "hello_world");
    const size_t DATA_SIZE = COUNT * sizeof(int);
    auto bo0 = xrt::bo(device, DATA_SIZE, hello_world.group_id(0));
    auto bo1 = xrt::bo(device, DATA_SIZE, hello_world.group_id(1));
    auto bo0_map = bo0.map<int*>();
    auto bo1_map = bo1.map<int*>();
    std::fill(bo0_map, bo0_map + COUNT, 0);
    std::fill(bo1_map, bo1_map + COUNT, 0);

    // Fill our data sets with pattern
    int bufReference[COUNT];
    bo0_map[0] = 'h';
    bo0_map[1] = 'e';
    bo0_map[2] = 'l';
    bo0_map[3] = 'l';
    bo0_map[4] = 'o';
    for (int i = 5; i < COUNT; ++i) {
      bo0_map[i] = 0;
      bo1_map[i] = i;
      bufReference[i] = i;
    }
    
    bo0.sync(XCL_BO_SYNC_BO_TO_DEVICE, DATA_SIZE, 0);
    bo1.sync(XCL_BO_SYNC_BO_TO_DEVICE, DATA_SIZE, 0);
    
    auto run = hello_world(bo0, bo1, COUNT);
    run.wait();
    
    //Get the output;
    bo1.sync(XCL_BO_SYNC_BO_FROM_DEVICE, DATA_SIZE, 0);
    
    // Validate our results
    if (std::memcmp(bo1_map, bo0_map, DATA_SIZE)) {
      for (int i=0;i< COUNT;i++) {
	std::cout << "bo0[" << i << "] = " << bo0_map[i] << ", bo1[" << i << "] = " << bo1_map[i] << std::endl;
      }
      throw std::runtime_error("Value read back does not match reference");
      return EXIT_FAILURE;
    }
    std::cout << "TEST PASSED\n";
    return 0;
}
