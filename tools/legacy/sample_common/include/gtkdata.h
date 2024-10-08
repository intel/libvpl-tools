/*############################################################################
  # Copyright (C) 2024 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/
#ifndef __GTK_DATA_H__
#define __GTK_DATA_H__

typedef struct {
    int fd;
    int width;
    int height;
    int stride[3];
    int num_planes;
    int offset[3];
    uint32_t fourcc;
} gtk_data_t;

#endif // __GTK_DATA_H__
