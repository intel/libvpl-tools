/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#pragma once

#include "vpl/mfxvideo++.h"

/// Base class for hw device
class CHWDevice {
public:
    virtual ~CHWDevice() {}
    /** Initializes device for requested processing.
    @param[in] hWindow Window handle to bundle device to.
    @param[in] nViews Number of views to process.
    @param[in] nAdapterNum Number of adapter to use
    @param[in] isFullScreen Full screen is enabled or not
    */
    virtual mfxStatus Init(mfxHDL hWindow,
                           mfxU16 nViews,
                           mfxU32 nAdapterNum,
                           bool isFullScreen = false) = 0;
    /// Reset device.
    virtual mfxStatus Reset() = 0;
    /// Get handle can be used for MFX session SetHandle calls
    virtual mfxStatus GetHandle(mfxHandleType type, mfxHDL* pHdl) = 0;
    /** Set handle.
    Particular device implementation may require other objects to operate.
    */
    virtual mfxStatus SetHandle(mfxHandleType type, mfxHDL hdl)                             = 0;
    virtual mfxStatus RenderFrame(mfxFrameSurface1* pSurface, mfxFrameAllocator* pmfxAlloc) = 0;
    virtual void UpdateTitle(double fps)                                                    = 0;
    virtual void SetMondelloInput(bool isMondelloInputEnabled)                              = 0;
    virtual void SetDxgiFullScreen()                                                        = 0;
    virtual void Close()                                                                    = 0;
};
