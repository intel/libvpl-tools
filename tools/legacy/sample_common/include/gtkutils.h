/*############################################################################
  # Copyright (C) 2024 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/
#ifndef __GTK_UTILS_H__
#define __GTK_UTILS_H__
#if defined(LIBVA_GTK4_SUPPORT)
    #include <iostream>
    #include <string>
    #include <vector>

    #include <epoxy/egl.h>
    #include <gtkmm.h>
    #include "gtkdata.h"

class GtkPlayer : public Gtk::Window {
public:
    GtkPlayer(int width, int height, bool isFullscreen);
    ~GtkPlayer();
    void frameReady();
    std::unique_ptr<Glib::Dispatcher> m_dispatcher;
    gtk_data_t m_frame_data;

protected:
    Gtk::Box m_VBox{ Gtk::Orientation::VERTICAL, false };

    EGLDisplay m_egl_display;
    PFNEGLCREATEIMAGEPROC eglCreateImage;
    PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES;
    Gtk::GLArea m_GLArea;

    void realize();
    void unrealize();
    bool render(const Glib::RefPtr<Gdk::GLContext>& context);

    void initBuffers();
    void initShaders();
    void initEgl(void);
    void getMonitorDimension();

    bool onKeyPressEvent(guint keyval, guint a, Gdk::ModifierType state);

    int m_target = GL_TEXTURE_2D;
    GLuint m_Vao{ 0 };
    GLuint m_Buffer{ 0 };
    GLuint m_program{ 0 };
    GLuint m_Mvp{ 0 };
    GLuint m_Ebo{ 0 };
    GLuint m_texture;

    Gdk::Rectangle m_mon_dim;
    Gdk::Rectangle m_mon_work_area;
    bool m_isX11 = false;
    int m_height = 0;
    int m_width  = 0;
    bool m_fullscreenMode;
    std::set<guint> m_pressed_keys;
};

#endif /* #if defined(LIBVA_GTK4_SUPPORT) */

#endif /* __GTK_UTILS_H__ */
