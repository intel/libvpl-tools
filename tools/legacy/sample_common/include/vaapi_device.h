/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#if defined(LIBVA_DRM_SUPPORT) || defined(LIBVA_X11_SUPPORT) || defined(LIBVA_ANDROID_SUPPORT) || \
    defined(LIBVA_WAYLAND_SUPPORT)
    #include "hw_device.h"
    #include "vaapi_utils_drm.h"
    #include "vaapi_utils_x11.h"
    #if defined(LIBVA_ANDROID_SUPPORT)
        #include "vaapi_utils_android.h"
    #endif
    #ifdef LIBVA_GTK4_SUPPORT
        #include <glibmm/dispatcher.h>
        #include <future>
        #include "gtkdata.h"
        #include "vaapi_utils_gtk.h"
    #endif

CHWDevice* CreateVAAPIDevice(const std::string& devicePath = "", int type = MFX_LIBVA_DRM);

    #if defined(LIBVA_DRM_SUPPORT)
/** VAAPI DRM implementation. */
class CVAAPIDeviceDRM : public CHWDevice {
public:
    CVAAPIDeviceDRM(const std::string& devicePath, int type);
    virtual ~CVAAPIDeviceDRM(void);

    virtual mfxStatus Init(mfxHDL hWindow,
                           mfxU16 nViews,
                           mfxU32 nAdapterNum,
                           bool isFullScreen = false);
    virtual mfxStatus Reset(void) {
        return MFX_ERR_NONE;
    }
    virtual void Close(void) {}

    virtual mfxStatus SetHandle(mfxHandleType type, mfxHDL hdl) {
        return MFX_ERR_UNSUPPORTED;
    }
    virtual mfxStatus GetHandle(mfxHandleType type, mfxHDL* pHdl) {
        if ((MFX_HANDLE_VA_DISPLAY == type) && (NULL != pHdl)) {
            *pHdl = m_DRMLibVA.GetVADisplay();

            return MFX_ERR_NONE;
        }
        return MFX_ERR_UNSUPPORTED;
    }

    virtual mfxStatus RenderFrame(mfxFrameSurface1* pSurface, mfxFrameAllocator* pmfxAlloc);
    virtual void UpdateTitle(double fps) {}
    virtual void SetMondelloInput(bool isMondelloInputEnabled) {}
    virtual void SetDxgiFullScreen() {}

    inline drmRenderer* getRenderer() {
        return m_rndr;
    }

protected:
    DRMLibVA m_DRMLibVA;
    drmRenderer* m_rndr;

private:
    // no copies allowed
    CVAAPIDeviceDRM(const CVAAPIDeviceDRM&);
    void operator=(const CVAAPIDeviceDRM&);
};

    #endif

    #if defined(LIBVA_X11_SUPPORT)

/** VAAPI X11 implementation. */
class CVAAPIDeviceX11 : public CHWDevice {
public:
    CVAAPIDeviceX11(const std::string& devicePath) : m_X11LibVA(devicePath) {
        m_window      = NULL;
        m_nRenderWinX = 0;
        m_nRenderWinY = 0;
        m_nRenderWinW = 0;
        m_nRenderWinH = 0;
        m_bRenderWin  = false;
        #if defined(X11_DRI3_SUPPORT)
        m_xcbconn = NULL;
        #endif
    }
    virtual ~CVAAPIDeviceX11(void);

    virtual mfxStatus Init(mfxHDL hWindow,
                           mfxU16 nViews,
                           mfxU32 nAdapterNum,
                           bool isFullScreen = false);
    virtual mfxStatus Reset(void);
    virtual void Close(void);

    virtual mfxStatus SetHandle(mfxHandleType type, mfxHDL hdl);
    virtual mfxStatus GetHandle(mfxHandleType type, mfxHDL* pHdl);

    virtual mfxStatus RenderFrame(mfxFrameSurface1* pSurface, mfxFrameAllocator* pmfxAlloc);
    virtual void UpdateTitle(double fps) {}
    virtual void SetMondelloInput(bool isMondelloInputEnabled) {}
    virtual void SetDxgiFullScreen() {}

protected:
    mfxHDL m_window;
    X11LibVA m_X11LibVA;

private:
    bool m_bRenderWin;
    mfxU32 m_nRenderWinX;
    mfxU32 m_nRenderWinY;
    mfxU32 m_nRenderWinW;
    mfxU32 m_nRenderWinH;
        #if defined(X11_DRI3_SUPPORT)
    xcb_connection_t* m_xcbconn;
        #endif
    // no copies allowed
    CVAAPIDeviceX11(const CVAAPIDeviceX11&);
    void operator=(const CVAAPIDeviceX11&);
};

    #endif

    #if defined(LIBVA_WAYLAND_SUPPORT)

class Wayland;

class CVAAPIDeviceWayland : public CHWDevice {
public:
    CVAAPIDeviceWayland(const std::string& devicePath = "") : m_DRMLibVA(devicePath) {
        m_nRenderWinX            = 0;
        m_nRenderWinY            = 0;
        m_nRenderWinW            = 0;
        m_nRenderWinH            = 0;
        m_isMondelloInputEnabled = false;
        m_Wayland                = NULL;
    }
    virtual ~CVAAPIDeviceWayland(void);

    virtual mfxStatus Init(mfxHDL hWindow,
                           mfxU16 nViews,
                           mfxU32 nAdapterNum,
                           bool isFullScreen = false);
    virtual mfxStatus Reset(void) {
        return MFX_ERR_NONE;
    }
    virtual void Close(void);

    virtual mfxStatus SetHandle(mfxHandleType type, mfxHDL hdl) {
        return MFX_ERR_UNSUPPORTED;
    }
    virtual mfxStatus GetHandle(mfxHandleType type, mfxHDL* pHdl) {
        if ((MFX_HANDLE_VA_DISPLAY == type) && (NULL != pHdl)) {
            *pHdl = m_DRMLibVA.GetVADisplay();
            return MFX_ERR_NONE;
        }

        return MFX_ERR_UNSUPPORTED;
    }
    virtual mfxStatus RenderFrame(mfxFrameSurface1* pSurface, mfxFrameAllocator* pmfxAlloc);
    virtual void UpdateTitle(double fps) {}

    virtual void SetMondelloInput(bool isMondelloInputEnabled) {
        m_isMondelloInputEnabled = isMondelloInputEnabled;
    }

    virtual void SetDxgiFullScreen() {}

    Wayland* GetWaylandHandle() {
        return m_Wayland;
    }

protected:
    DRMLibVA m_DRMLibVA;
    MfxLoader::VA_WaylandClientProxy m_WaylandClient;
    Wayland* m_Wayland;

private:
    mfxU32 m_nRenderWinX;
    mfxU32 m_nRenderWinY;
    mfxU32 m_nRenderWinW;
    mfxU32 m_nRenderWinH;

    bool m_isMondelloInputEnabled;

    // no copies allowed
    CVAAPIDeviceWayland(const CVAAPIDeviceWayland&);
    void operator=(const CVAAPIDeviceWayland&);
};

    #endif

    #if defined(LIBVA_ANDROID_SUPPORT)

/** VAAPI Android implementation. */
class CVAAPIDeviceAndroid : public CHWDevice {
public:
    CVAAPIDeviceAndroid(AndroidLibVA* pAndroidLibVA) : m_pAndroidLibVA(pAndroidLibVA) {
        if (!m_pAndroidLibVA) {
            throw std::bad_alloc();
        }
    };
    virtual ~CVAAPIDeviceAndroid(void) {
        Close();
    }

    virtual mfxStatus Init(mfxHDL hWindow,
                           mfxU16 nViews,
                           mfxU32 nAdapterNum,
                           bool isFullScreen = false) {
        return MFX_ERR_NONE;
    }
    virtual mfxStatus Reset(void) {
        return MFX_ERR_NONE;
    }
    virtual void Close(void) {}

    virtual mfxStatus SetHandle(mfxHandleType type, mfxHDL hdl) {
        return MFX_ERR_UNSUPPORTED;
    }
    virtual mfxStatus GetHandle(mfxHandleType type, mfxHDL* pHdl) {
        if ((MFX_HANDLE_VA_DISPLAY == type) && (NULL != pHdl)) {
            if (m_pAndroidLibVA)
                *pHdl = m_pAndroidLibVA->GetVADisplay();
            return MFX_ERR_NONE;
        }

        return MFX_ERR_UNSUPPORTED;
    }

    virtual mfxStatus RenderFrame(mfxFrameSurface1* pSurface, mfxFrameAllocator* pmfxAlloc) {
        return MFX_ERR_NONE;
    }
    virtual void UpdateTitle(double fps) {}
    virtual void SetMondelloInput(bool isMondelloInputEnabled) {}
    virtual void SetDxgiFullScreen() {}

protected:
    AndroidLibVA* m_pAndroidLibVA;
};
    #endif
    #if defined(LIBVA_GTK4_SUPPORT)
class CVAAPIDeviceGTK : public CHWDevice {
public:
    CVAAPIDeviceGTK(const std::string& devicePath) {
        m_nRenderWinX = 0;
        m_nRenderWinY = 0;
        m_nRenderWinW = 0;
        m_nRenderWinH = 0;
        m_bRenderWin  = false;
        m_device_path = devicePath;

        if (isWayland()) {
            m_DRMLibVA = new DRMLibVA(devicePath);
        }
        else {
            m_GtkLibVA = new X11GtkVA(devicePath);
        }
    }

    virtual ~CVAAPIDeviceGTK(void) {
        delete m_GtkLibVA;
        delete m_DRMLibVA;
    }

    virtual mfxStatus Init(mfxHDL hWindow, mfxU16 nViews, mfxU32 nAdapterNum, bool isFullScreen);
    virtual mfxStatus Reset(void) {
        return MFX_ERR_NONE;
    }
    virtual void Close(void) {}

    virtual mfxStatus SetHandle(mfxHandleType type, mfxHDL hdl) {
        return MFX_ERR_UNSUPPORTED;
    }
    virtual mfxStatus GetHandle(mfxHandleType type, mfxHDL* pHdl);

    virtual mfxStatus RenderFrame(mfxFrameSurface1* pSurface, mfxFrameAllocator* pmfxAlloc);

    virtual void UpdateTitle(double fps) {}
    virtual void SetMondelloInput(bool isMondelloInputEnabled) {}
    virtual void SetDxgiFullScreen() {}

    bool GetInitDone() {
        return m_initComplete.get();
    }

    bool isWayland() {
        return std::getenv("WAYLAND_DISPLAY") != nullptr;
    }

private:
    bool m_bRenderWin;
    mfxU32 m_nRenderWinX;
    mfxU32 m_nRenderWinY;
    mfxU32 m_nRenderWinW;
    mfxU32 m_nRenderWinH;
    std::string m_device_path;
    std::atomic<bool> m_ForceStop = false;
    std::promise<bool> m_initPromise;
    std::future<bool> m_initComplete;

protected:
    X11GtkVA* m_GtkLibVA;
    DRMLibVA* m_DRMLibVA;
    virtual void SetForceStop() {
        m_ForceStop = true;
    }
    void gtkMain(bool isFullScreen, std::promise<bool>&& initPromise);
    Glib::Dispatcher* m_dispatcher_ptr;
    gtk_data_t* m_frame_data_ptr;
    std::thread* m_gtk_thread;
};
    #endif
#endif //#if defined(LIBVA_DRM_SUPPORT) || defined(LIBVA_X11_SUPPORT) || defined(LIBVA_ANDROID_SUPPORT)
