/*************************************************************************
 * Copyright (c) 2022 Advanced Micro Devices, Inc. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include <gtest/gtest.h>
#include "EnvVars.hpp"

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  RcclUnitTesting::EnvVars ev;
}
