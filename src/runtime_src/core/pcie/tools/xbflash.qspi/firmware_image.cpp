// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020-2021 Xilinx, Inc
// Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.

#include <fstream>
#include "firmware_image.h"

firmwareImage::firmwareImage(const char *file) :
    mBuf(nullptr)
{
    std::ifstream in(file, std::ios::binary | std::ios::ate);
    if (!in.is_open())
    {
        this->setstate(failbit);
        std::cout << "Can't open " << file << std::endl;
        return;
    }
    auto bufsize = in.tellg();

    // For non-dsabin file, the entire file is the image.
    mBuf = new char[bufsize];
    in.seekg(0);
    in.read(mBuf, bufsize);
    this->rdbuf()->pubsetbuf(mBuf, bufsize);
}

firmwareImage::~firmwareImage()
{
    delete[] mBuf;
}
