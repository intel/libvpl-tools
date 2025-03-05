/*############################################################################
  # Copyright (C) 2025 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef VAAPI_UTILS_COMMON_H
#define VAAPI_UTILS_COMMON_H

extern const char* MFX_X11_NODE_RENDER;
extern const char* MFX_X11_DRIVER_NAME;
extern const char* MFX_X11_DRIVER_XE_NAME;

extern int open_intel_adapter(const std::string& devicePath);
#endif // VAAPI_UTILS_COMMON_H
