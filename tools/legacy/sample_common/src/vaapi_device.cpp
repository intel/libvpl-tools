/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#if defined(LIBVA_DRM_SUPPORT) || defined(LIBVA_X11_SUPPORT) || defined(LIBVA_ANDROID_SUPPORT)
    #ifdef LIBVA_GTK4_SUPPORT
        #include "gtkutils.h"
    #endif
    #include "sample_utils.h"
    #include "vaapi_device.h"
    #if defined(LIBVA_WAYLAND_SUPPORT)
        #include "class_wayland.h"
    #endif

    #if defined(LIBVA_X11_SUPPORT)

        #include <X11/Xlib.h>
        #include <va/va_x11.h>

        #include "vaapi_allocator.h"
        #if defined(X11_DRI3_SUPPORT)
            #include <fcntl.h>
            #include <sys/ioctl.h>

            #define ALIGN(x, y)   (((x) + (y)-1) & -(y))
            #define PAGE_ALIGN(x) ALIGN(x, 4096)
        #endif // X11_DRI3_SUPPORT

        #define VAAPI_GET_X_DISPLAY(_display) (Display*)(_display)
        #define VAAPI_GET_X_WINDOW(_window)   (Window*)(_window)
const char* MFX_DEVICE_NODE_RENDER          = "/dev/dri/renderD";
const char* MFX_DEVICE_DRIVER_NAME          = "i915";
constexpr mfxU32 MFX_DEVICE_DRIVER_NAME_LEN = 4;
constexpr mfxU32 MFX_DEVICE_NODE_INDEX      = 128;
constexpr mfxU32 MFX_DEVICE_MAX_NODES       = 16;

CVAAPIDeviceX11::~CVAAPIDeviceX11(void) {
    Close();
}

mfxStatus CVAAPIDeviceX11::Init(mfxHDL hWindow,
                                mfxU16 nViews,
                                mfxU32 nAdapterNum,
                                bool /* isFullScreen */) {
    mfxStatus mfx_res = MFX_ERR_NONE;
    Window* window    = NULL;

    if (nViews) {
        if (MFX_ERR_NONE == mfx_res) {
            m_window = window = (Window*)malloc(sizeof(Window));
            if (!m_window)
                mfx_res = MFX_ERR_MEMORY_ALLOC;
        }
        if (MFX_ERR_NONE == mfx_res) {
            Display* display              = VAAPI_GET_X_DISPLAY(m_X11LibVA.GetXDisplay());
            MfxLoader::XLib_Proxy& x11lib = m_X11LibVA.GetX11();
            mfxU32 screen_number          = DefaultScreen(display);

            *window = x11lib.XCreateSimpleWindow(display,
                                                 RootWindow(display, screen_number),
                                                 m_bRenderWin ? m_nRenderWinX : 0,
                                                 m_bRenderWin ? m_nRenderWinY : 0,
                                                 100,
                                                 100,
                                                 0,
                                                 0,
                                                 BlackPixel(display, screen_number));

            if (!(*window))
                mfx_res = MFX_ERR_UNKNOWN;
            else {
                x11lib.XMapWindow(display, *window);
                x11lib.XSync(display, False);
            }
        }
    }
        #if defined(X11_DRI3_SUPPORT)
    MfxLoader::X11_Xcb_Proxy& x11xcblib = m_X11LibVA.GetX11XcbX11();

    m_xcbconn = x11xcblib.XGetXCBConnection(VAAPI_GET_X_DISPLAY(m_X11LibVA.GetXDisplay()));
        #endif

    return mfx_res;
}

void CVAAPIDeviceX11::Close(void) {
    if (m_window) {
        Display* display = VAAPI_GET_X_DISPLAY(m_X11LibVA.GetXDisplay());
        Window* window   = VAAPI_GET_X_WINDOW(m_window);

        MfxLoader::XLib_Proxy& x11lib = m_X11LibVA.GetX11();
        x11lib.XDestroyWindow(display, *window);

        free(m_window);
        m_window = NULL;
    }
}

mfxStatus CVAAPIDeviceX11::Reset(void) {
    return MFX_ERR_NONE;
}

mfxStatus CVAAPIDeviceX11::GetHandle(mfxHandleType type, mfxHDL* pHdl) {
    if ((MFX_HANDLE_VA_DISPLAY == type) && (NULL != pHdl)) {
        *pHdl = m_X11LibVA.GetVADisplay();

        return MFX_ERR_NONE;
    }

    return MFX_ERR_UNSUPPORTED;
}

mfxStatus CVAAPIDeviceX11::SetHandle(mfxHandleType type, mfxHDL hdl) {
    return MFX_ERR_UNSUPPORTED;
}

mfxStatus CVAAPIDeviceX11::RenderFrame(mfxFrameSurface1* pSurface,
                                       mfxFrameAllocator* /*pmfxAlloc*/) {
    mfxStatus mfx_res = MFX_ERR_NONE;
    vaapiMemId* memId = NULL;

        #if !defined(X11_DRI3_SUPPORT)
    VAStatus va_res = VA_STATUS_SUCCESS;
    VASurfaceID surface;
    Display* display = VAAPI_GET_X_DISPLAY(m_X11LibVA.GetXDisplay());
    Window* window   = VAAPI_GET_X_WINDOW(m_window);

    if (!window || !(*window))
        mfx_res = MFX_ERR_NOT_INITIALIZED;
    // should MFX_ERR_NONE be returned below considering situation as EOS?
    if ((MFX_ERR_NONE == mfx_res) && NULL == pSurface)
        mfx_res = MFX_ERR_NULL_PTR;
    if (MFX_ERR_NONE == mfx_res) {
        memId = (vaapiMemId*)(pSurface->Data.MemId);
        if (!memId || !memId->m_surface)
            mfx_res = MFX_ERR_NULL_PTR;
    }
    if (MFX_ERR_NONE == mfx_res) {
        VADisplay dpy    = m_X11LibVA.GetVADisplay();
        VADisplay rnddpy = m_X11LibVA.GetVADisplay();
        VASurfaceID rndsrf;
        void* ctx;

        surface = *memId->m_surface;

        va_res  = m_X11LibVA.AcquireVASurface(&ctx, dpy, surface, rnddpy, &rndsrf);
        mfx_res = va_to_mfx_status(va_res);
        if (MFX_ERR_NONE != mfx_res)
            return mfx_res;

        MfxLoader::XLib_Proxy& x11lib = m_X11LibVA.GetX11();
        x11lib.XResizeWindow(display, *window, pSurface->Info.CropW, pSurface->Info.CropH);

        MfxLoader::VA_X11Proxy& vax11lib = m_X11LibVA.GetVAX11();
        va_res                           = vax11lib.vaPutSurface(rnddpy,
                                       rndsrf,
                                       *window,
                                       pSurface->Info.CropX,
                                       pSurface->Info.CropY,
                                       pSurface->Info.CropX + pSurface->Info.CropW,
                                       pSurface->Info.CropY + pSurface->Info.CropH,
                                       pSurface->Info.CropX,
                                       pSurface->Info.CropY,
                                       pSurface->Info.CropX + pSurface->Info.CropW,
                                       pSurface->Info.CropY + pSurface->Info.CropH,
                                       NULL,
                                       0,
                                       VA_FRAME_PICTURE);

        mfx_res = va_to_mfx_status(va_res);
        x11lib.XSync(display, False);

        m_X11LibVA.ReleaseVASurface(ctx, dpy, surface, rnddpy, rndsrf);
    }
    return mfx_res;
        #else //\/ X11_DRI3_SUPPORT
    Window* window = VAAPI_GET_X_WINDOW(m_window);
    Window root;
    unsigned int border, depth, stride, size, width, height;
    int fd = 0, bpp = 0, x, y;

    MfxLoader::Xcb_Proxy& xcblib               = m_X11LibVA.GetXcbX11();
    MfxLoader::XLib_Proxy& x11lib              = m_X11LibVA.GetX11();
    MfxLoader::Xcbpresent_Proxy& xcbpresentlib = m_X11LibVA.GetXcbpresentX11();
    MfxLoader::XCB_Dri3_Proxy& dri3lib         = m_X11LibVA.GetXCBDri3X11();

    if (!window || !(*window))
        mfx_res = MFX_ERR_NOT_INITIALIZED;
    // should MFX_ERR_NONE be returned below considering situation as EOS?
    if ((MFX_ERR_NONE == mfx_res) && NULL == pSurface)
        mfx_res = MFX_ERR_NULL_PTR;
    if (MFX_ERR_NONE == mfx_res) {
        memId = (vaapiMemId*)(pSurface->Data.MemId);
        if (!memId || !memId->m_surface)
            mfx_res = MFX_ERR_NULL_PTR;
    }

    if (memId && memId->m_buffer_info.mem_type != VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME) {
        printf("Memory type invalid!\n");
        return MFX_ERR_UNSUPPORTED;
    }

    if (MFX_ERR_NONE == mfx_res) {
        x11lib.XResizeWindow(VAAPI_GET_X_DISPLAY(m_X11LibVA.GetXDisplay()),
                             *window,
                             pSurface->Info.CropW,
                             pSurface->Info.CropH);
        x11lib.XGetGeometry(VAAPI_GET_X_DISPLAY(m_X11LibVA.GetXDisplay()),
                            *window,
                            &root,
                            &x,
                            &y,
                            &width,
                            &height,
                            &border,
                            &depth);

        switch (depth) {
            case 8:
                bpp = 8;
                break;
            case 15:
            case 16:
                bpp = 16;
                break;
            case 24:
            case 32:
                bpp = 32;
                break;
            default:
                printf("Invalid depth\n");
        }

        width  = pSurface->Info.CropX + pSurface->Info.CropW;
        height = pSurface->Info.CropY + pSurface->Info.CropH;

        stride = memId->m_image.pitches[0];
        size   = PAGE_ALIGN(stride * height);

        fd = dup(memId->m_buffer_info.handle);
        if (!fd) {
            printf("Invalid fd\n");
            return MFX_ERR_NOT_INITIALIZED;
        }

        xcb_pixmap_t pixmap = xcblib.xcb_generate_id(m_xcbconn);
        xcb_void_cookie_t cookie;
        xcb_generic_error_t* error;

        cookie = dri3lib.xcb_dri3_pixmap_from_buffer_checked(m_xcbconn,
                                                             pixmap,
                                                             root,
                                                             size,
                                                             width,
                                                             height,
                                                             stride,
                                                             depth,
                                                             bpp,
                                                             fd);
        if ((error = xcblib.xcb_request_check(m_xcbconn, cookie))) {
            printf(
                "Failed to create xcb pixmap from the %s surface: try another color format (e.g. RGB4)\n",
                ColorFormatToStr(pSurface->Info.FourCC));
            free(error);
            close(fd);
            return MFX_ERR_INVALID_HANDLE;
        }

        cookie = xcbpresentlib.xcb_present_pixmap_checked(m_xcbconn,
                                                          *window,
                                                          pixmap,
                                                          0,
                                                          0,
                                                          0,
                                                          0,
                                                          0,
                                                          None,
                                                          None,
                                                          None,
                                                          XCB_PRESENT_OPTION_NONE,
                                                          0,
                                                          0,
                                                          0,
                                                          0,
                                                          NULL);
        if ((error = xcblib.xcb_request_check(m_xcbconn, cookie))) {
            printf("Failed to present pixmap\n");
            free(error);
            close(fd);
            return MFX_ERR_UNKNOWN;
        }

        xcblib.xcb_free_pixmap(m_xcbconn, pixmap);
        xcblib.xcb_flush(m_xcbconn);
        close(fd);
    }

    return mfx_res;

        #endif // X11_DRI3_SUPPORT
}
    #endif

    #if defined(LIBVA_WAYLAND_SUPPORT)
        #include "wayland-drm-client-protocol.h"

CVAAPIDeviceWayland::~CVAAPIDeviceWayland(void) {
    Close();
    m_WaylandClient.WaylandDestroy((MfxLoader::Wayland*)m_Wayland);
}

mfxStatus CVAAPIDeviceWayland::Init(mfxHDL hWindow,
                                    mfxU16 nViews,
                                    mfxU32 nAdapterNum,
                                    bool /* isFullScreen */) {
    mfxStatus mfx_res = MFX_ERR_NONE;

    if (nViews) {
        m_Wayland = (Wayland*)m_WaylandClient.WaylandCreate();
        if (!m_Wayland->InitDisplay()) {
            return MFX_ERR_DEVICE_FAILED;
        }

        if (NULL == m_Wayland->GetDisplay()) {
            mfx_res = MFX_ERR_UNKNOWN;
            return mfx_res;
        }
        if (-1 == m_Wayland->DisplayRoundtrip()) {
            mfx_res = MFX_ERR_UNKNOWN;
            return mfx_res;
        }
        if (!m_Wayland->CreateSurface()) {
            mfx_res = MFX_ERR_UNKNOWN;
            return mfx_res;
        }
    }
    return mfx_res;
}

mfxStatus CVAAPIDeviceWayland::RenderFrame(mfxFrameSurface1* pSurface,
                                           mfxFrameAllocator* /*pmfxAlloc*/) {
    uint32_t drm_format = 0;
    int offsets[3], pitches[3];
    mfxStatus mfx_res             = MFX_ERR_NONE;
    vaapiMemId* memId             = NULL;
    struct wl_buffer* m_wl_buffer = NULL;
    if (NULL == pSurface) {
        mfx_res = MFX_ERR_UNKNOWN;
        return mfx_res;
    }
    m_Wayland->Sync();
    memId = (vaapiMemId*)(pSurface->Data.MemId);

    if (pSurface->Info.FourCC == MFX_FOURCC_NV12) {
        drm_format = WL_DRM_FORMAT_NV12;
    }
    else if (pSurface->Info.FourCC == MFX_FOURCC_RGB4) {
        drm_format = WL_DRM_FORMAT_ARGB8888;

        if (m_isMondelloInputEnabled) {
            drm_format = WL_DRM_FORMAT_XBGR8888;
        }
    }

    offsets[0]  = memId->m_image.offsets[0];
    offsets[1]  = memId->m_image.offsets[1];
    offsets[2]  = memId->m_image.offsets[2];
    pitches[0]  = memId->m_image.pitches[0];
    pitches[1]  = memId->m_image.pitches[1];
    pitches[2]  = memId->m_image.pitches[2];
    m_wl_buffer = m_Wayland->CreatePrimeBuffer(memId->m_buffer_info.handle,
                                               pSurface->Info.CropW,
                                               pSurface->Info.CropH,
                                               drm_format,
                                               offsets,
                                               pitches);
    if (NULL == m_wl_buffer) {
        printf("\nCan't wrap flink to wl_buffer\n");
        mfx_res = MFX_ERR_UNKNOWN;
        return mfx_res;
    }

    m_Wayland->RenderBuffer(m_wl_buffer, pSurface);

    return mfx_res;
}

void CVAAPIDeviceWayland::Close(void) {
    m_Wayland->FreeSurface();
}

CHWDevice* CreateVAAPIDevice(void) {
    return new CVAAPIDeviceWayland();
}

    #endif // LIBVA_WAYLAND_SUPPORT

    #if defined(LIBVA_DRM_SUPPORT)

CVAAPIDeviceDRM::CVAAPIDeviceDRM(const std::string& devicePath, int type)
        : m_DRMLibVA(devicePath, type),
          m_rndr(NULL) {}

CVAAPIDeviceDRM::~CVAAPIDeviceDRM(void) {
    MSDK_SAFE_DELETE(m_rndr);
}

mfxStatus CVAAPIDeviceDRM::Init(mfxHDL hWindow,
                                mfxU16 nViews,
                                mfxU32 nAdapterNum,
                                bool /* isFullScreen */) {
    if (0 == nViews) {
        return MFX_ERR_NONE;
    }
    if (1 == nViews) {
        if (m_DRMLibVA.getBackendType() == MFX_LIBVA_DRM_RENDERNODE) {
            return MFX_ERR_NONE;
        }
        mfxI32* monitorType = (mfxI32*)hWindow;
        if (!monitorType)
            return MFX_ERR_INVALID_VIDEO_PARAM;
        try {
            m_rndr = new drmRenderer(m_DRMLibVA.getFD(), *monitorType);
        }
        catch (...) {
            printf("vaapi_device: failed to initialize drmrender\n");
            return MFX_ERR_UNKNOWN;
        }
        return MFX_ERR_NONE;
    }
    return MFX_ERR_UNSUPPORTED;
}

mfxStatus CVAAPIDeviceDRM::RenderFrame(mfxFrameSurface1* pSurface, mfxFrameAllocator* pmfxAlloc) {
    return (m_rndr) ? m_rndr->render(pSurface) : MFX_ERR_NONE;
}

    #endif

    #if defined(LIBVA_DRM_SUPPORT) || defined(LIBVA_X11_SUPPORT) || defined(LIBVA_WAYLAND_SUPPORT)

CHWDevice* CreateVAAPIDevice(const std::string& devicePath, int type) {
    CHWDevice* device = NULL;

    switch (type) {
        case MFX_LIBVA_DRM_RENDERNODE:
        case MFX_LIBVA_DRM_MODESET:
        #if defined(LIBVA_DRM_SUPPORT)
            try {
                device = new CVAAPIDeviceDRM(devicePath, type);
            }
            catch (std::exception&) {
            }
        #endif
            break;

        case MFX_LIBVA_X11:
        #if defined(LIBVA_X11_SUPPORT)
            try {
                device = new CVAAPIDeviceX11(devicePath);
            }
            catch (std::exception&) {
            }
        #endif
            break;
        case MFX_LIBVA_WAYLAND:
        #if defined(LIBVA_WAYLAND_SUPPORT)
            device = new CVAAPIDeviceWayland(devicePath);
        #endif
            break;
        case MFX_LIBVA_GTK:
        #if defined(LIBVA_GTK4_SUPPORT)
            device = new CVAAPIDeviceGTK(devicePath);
        #endif
            break;
        case MFX_LIBVA_AUTO:
        #if defined(LIBVA_X11_SUPPORT)
            try {
                device = new CVAAPIDeviceX11(devicePath);
            }
            catch (std::exception&) {
            }
        #endif
        #if defined(LIBVA_DRM_SUPPORT)
            if (!device) {
                try {
                    device = new CVAAPIDeviceDRM(devicePath, type);
                }
                catch (std::exception&) {
                }
            }
        #endif
            break;
    } // switch(type)

    return device;
}

        #if defined(LIBVA_GTK4_SUPPORT)
void CVAAPIDeviceGTK::gtkMain(bool isFullScreen, std::promise<bool>&& initPromise) {
    Glib::RefPtr<Gtk::Application> gtk_app = Gtk::Application::create();
    auto window                            = new GtkPlayer(200, 200, isFullScreen);
    m_dispatcher_ptr                       = window->m_dispatcher.get();
    m_frame_data_ptr                       = &window->m_frame_data;

    gtk_app->signal_activate().connect([&] {
        gtk_app->add_window(*window);
        window->show();
    });
    gtk_app->signal_window_removed().connect([&](Gtk::Window* window) {
        m_ForceStop = true;
        gtk_app->remove_window(*window);
        delete window;
    });
    gtk_app->signal_shutdown().connect([&]() {
        std::vector<Gtk::Window*> window_list = gtk_app->get_windows();
        for (auto win : window_list) {
            win->close();
        }
    });

    initPromise.set_value(true);
    gtk_app->run(0, nullptr);
}

mfxStatus CVAAPIDeviceGTK::Init(mfxHDL hWindow,
                                mfxU16 /* nViews */,
                                mfxU32 /* nAdapterNum */,
                                bool isFullScreen) {
    mfxStatus mfx_res = MFX_ERR_NONE;
    m_initComplete    = m_initPromise.get_future();
    m_gtk_thread =
        new std::thread(&CVAAPIDeviceGTK::gtkMain, this, isFullScreen, std::move(m_initPromise));
    m_gtk_thread->detach();
    return mfx_res;
}

mfxStatus CVAAPIDeviceGTK::GetHandle(mfxHandleType type, mfxHDL* pHdl) {
    if ((MFX_HANDLE_VA_DISPLAY == type) && (NULL != pHdl)) {
        if (isWayland()) {
            *pHdl = m_DRMLibVA->GetVADisplay();
        }
        else {
            *pHdl = m_GtkLibVA->GetVADisplay();
        }
        return MFX_ERR_NONE;
    }

    return MFX_ERR_UNSUPPORTED;
}

mfxStatus CVAAPIDeviceGTK::RenderFrame(mfxFrameSurface1* pSurface, mfxFrameAllocator* pmfxAlloc) {
    mfxStatus mfx_res   = MFX_ERR_NONE;
    mfxFrameInfo* pInfo = &pSurface->Info;
    int width           = pInfo->CropX + pInfo->CropW;
    int height          = pInfo->CropY + pInfo->CropH;

    if (m_ForceStop) {
        return MFX_ERR_DEVICE_FAILED;
    }

    vaapiMemId* memId = nullptr;
    memId             = (vaapiMemId*)(pSurface->Data.MemId);
    if (!memId || !memId->m_surface) {
        mfx_res = MFX_ERR_NULL_PTR;
    }

    m_frame_data_ptr->stride[0]  = memId->m_image.pitches[0];
    m_frame_data_ptr->stride[1]  = memId->m_image.pitches[1];
    m_frame_data_ptr->stride[2]  = memId->m_image.pitches[2];
    m_frame_data_ptr->offset[0]  = memId->m_image.offsets[0];
    m_frame_data_ptr->offset[1]  = memId->m_image.offsets[1];
    m_frame_data_ptr->offset[2]  = memId->m_image.offsets[2];
    m_frame_data_ptr->num_planes = memId->m_image.num_planes;
    m_frame_data_ptr->fourcc     = memId->m_image.format.fourcc;
    m_frame_data_ptr->height     = height;
    m_frame_data_ptr->width      = width;
    m_frame_data_ptr->fd         = (int)memId->m_buffer_info.handle;
    m_dispatcher_ptr->emit();

    return mfx_res;
}
        #endif
    #elif defined(LIBVA_ANDROID_SUPPORT)

static AndroidLibVA g_LibVA;
CHWDevice* CreateVAAPIDevice(const std::string& devicePath, int type) {
    return new CVAAPIDeviceAndroid(&g_LibVA);
}

    #endif

#endif //#if defined(LIBVA_DRM_SUPPORT) || defined(LIBVA_X11_SUPPORT) || defined(LIBVA_ANDROID_SUPPORT)
