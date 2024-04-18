/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#pragma once

#include "sample_defs.h"

#ifndef TOOLS_VERSION_MAJOR
    #define TOOLS_VERSION_MAJOR 0
#endif

#ifndef TOOLS_VERSION_MINOR
    #define TOOLS_VERSION_MINOR 0
#endif

#ifndef TOOLS_VERSION_PATCH
    #define TOOLS_VERSION_PATCH 0
#endif

static std::string GetToolVersion() {
    std::stringstream ss;
    ss << TOOLS_VERSION_MAJOR << "." << TOOLS_VERSION_MINOR << "." << TOOLS_VERSION_PATCH;
    return ss.str();
}
