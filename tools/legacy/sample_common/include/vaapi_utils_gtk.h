/*############################################################################
  # Copyright (C) 2024 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef __VAAPI_UTILS_GTK_H__
#define __VAAPI_UTILS_GTK_H__

#if defined(LIBVA_GTK4_SUPPORT)

    #include <sigc++/sigc++.h>
    #include <va/va_x11.h>
    #include "vaapi_utils.h"

class X11GtkVA : public CLibVA {
public:
    X11GtkVA(const std::string& devicePath = "");
    virtual ~X11GtkVA(void);

    MfxLoader::XLib_Proxy& GetX11() {
        return m_x11lib;
    }
    MfxLoader::VA_X11Proxy& GetVAX11() {
        return m_vax11lib;
    }

    void* GetXDisplay(void) {
        return m_display;
    }

protected:
    Display* m_display;
    VAConfigID m_configID;
    VAContextID m_contextID;
    MfxLoader::XLib_Proxy m_x11lib;
    MfxLoader::VA_X11Proxy m_vax11lib;

private:
    void Close();

    DISALLOW_COPY_AND_ASSIGN(X11GtkVA);
};

#endif // #if defined(LIBVA_GTK4_SUPPORT)

#endif // #ifndef __VAAPI_UTILS_GTK_H__
