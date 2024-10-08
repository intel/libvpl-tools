/*############################################################################
  # Copyright (C) 2024 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/
#if defined(LIBVA_GTK4_SUPPORT)
    #include "gtkutils.h"
    #include <drm/drm_fourcc.h>
    #include <gdk/wayland/gdkwayland.h>
    #include <gdk/x11/gdkx.h>
    #include <gtk/gtk.h>

GLfloat vertexData[] = {
    // Positions    // Texture Coords
    -1.0f, 1.0f,  0.0f, 0.0f, // Top-left
    -1.0f, -1.0f, 0.0f, 1.0f, // Bottom-left
    1.0f,  -1.0f, 1.0f, 1.0f, // Bottom-right
    1.0f,  1.0f,  1.0f, 0.0f // Top-right
};

GLuint indices[] = { 3, 1, 0, 3, 2, 1 };

// Vertex shader source
const char* vertexShaderSrc = R"(
    #version 300 es
    layout(location = 0) in vec2 position;
    layout(location = 1) in vec2 texcoord;
    out vec2 Texcoord;
    void main()
    {
        Texcoord = texcoord;
        gl_Position = vec4(position, 0.0, 1.0);
    }
)";

// Fragment shader source
const char* fragmentShaderSrc = R"(
    #version 300 es
    precision mediump float;  // Define default precision for float types
    in vec2 Texcoord;
    out vec4 outColor;
    uniform sampler2D tex;

    void main()
    {
        outColor = texture(tex, Texcoord);
    }
)";

static void CheckGLError(const char* statement, const char* fname, int line);

    #define GL_CHECK(x)                           \
        do {                                      \
            x;                                    \
            CheckGLError(#x, __FILE__, __LINE__); \
        } while (0)

static void CheckGLError(const char* statement, const char* fname, int line) {
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "OpenGL error " << err << " at " << fname << ":" << line << " - for "
                  << statement << std::endl;
    }
}

void GtkPlayer::initEgl(void) {
    auto dis = get_display()->gobj();

    if (GDK_IS_X11_DISPLAY(dis)) {
        Display* xDis = GDK_DISPLAY_XDISPLAY(dis);
        m_egl_display = eglGetDisplay((EGLNativeDisplayType)xDis);
    }
    else if (GDK_IS_WAYLAND_DISPLAY(dis)) {
        m_egl_display =
            eglGetDisplay((EGLNativeDisplayType)gdk_wayland_display_get_wl_display(dis));
    }
    else {
        std::cerr << "EGL display is Unknown" << std::endl;
    }

    if (m_egl_display == EGL_NO_DISPLAY) {
        std::cerr << "EGL display is NULL" << std::endl;
    }

    if (!eglInitialize(m_egl_display, nullptr, nullptr)) {
        std::cerr << "Unable to initialize EGL." << std::endl;
    }

    eglCreateImage = (PFNEGLCREATEIMAGEPROC)eglGetProcAddress("eglCreateImage");
    if (!eglCreateImage) {
        std::cerr << "Failed to load eglCreateImage" << std::endl;
    }

    glEGLImageTargetTexture2DOES =
        (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");
    if (!glEGLImageTargetTexture2DOES) {
        std::cerr << "Failed to load glEGLImageTargetTexture2DOES" << std::endl;
    }
}

void GtkPlayer::getMonitorDimension() {
    auto mon           = Gdk::Display::get_default()->get_monitors()->get_object(0);
    auto first_monitor = std::dynamic_pointer_cast<const Gdk::Monitor>(mon);
    first_monitor->get_geometry(m_mon_dim);

    if (GDK_IS_X11_DISPLAY(Gdk::Display::get_default()->gobj())) {
        m_isX11                       = true;
        GdkMonitor* first_monitor_ptr = (GdkMonitor*)first_monitor->gobj();
        gdk_x11_monitor_get_workarea(first_monitor_ptr, m_mon_work_area.gobj());
    }
}

GtkPlayer::GtkPlayer(int width, int height, bool isFullscreen) {
    m_width          = width;
    m_height         = height;
    m_fullscreenMode = isFullscreen;

    // Set the window title
    set_title("GTK Video Player");
    set_size_request(m_width, m_height);

    set_child(m_VBox);
    m_dispatcher = std::make_unique<Glib::Dispatcher>();
    m_dispatcher->connect(sigc::mem_fun(*this, &GtkPlayer::frameReady));

    auto key_press = Gtk::EventControllerKey::create();
    key_press->signal_key_pressed().connect(sigc::mem_fun(*this, &GtkPlayer::onKeyPressEvent),
                                            false);
    add_controller(key_press);

    m_GLArea.set_expand(true);
    m_GLArea.set_size_request(m_width, m_height);
    m_GLArea.set_auto_render(true);
    m_GLArea.signal_realize().connect(sigc::mem_fun(*this, &GtkPlayer::realize));
    m_GLArea.signal_unrealize().connect(sigc::mem_fun(*this, &GtkPlayer::unrealize), false);
    m_GLArea.signal_render().connect(sigc::mem_fun(*this, &GtkPlayer::render), false);
    m_VBox.append(m_GLArea);

    getMonitorDimension();
    initEgl();

    if (false == m_isX11) {
        // Remove the title bar on Wayland
        set_decorated(false);
    }
}

bool GtkPlayer::onKeyPressEvent(guint keyval, guint a, Gdk::ModifierType state) {
    if (keyval == GDK_KEY_Escape) {
        if (is_fullscreen()) {
            m_fullscreenMode = false;
            unfullscreen();
        }
        m_pressed_keys.clear();
    }
    else if (GDK_KEY_Control_L == keyval || GDK_KEY_Control_R == keyval) {
        m_pressed_keys.emplace(keyval);
    }
    else if ((m_pressed_keys.find(GDK_KEY_Control_L) != m_pressed_keys.end() ||
              m_pressed_keys.find(GDK_KEY_Control_R) != m_pressed_keys.end()) &&
             (GDK_KEY_f == keyval || GDK_KEY_F == keyval)) {
        fullscreen();
        present();
        m_pressed_keys.clear();
        m_fullscreenMode = true;
    }

    return false;
}

bool GtkPlayer::render(const Glib::RefPtr<Gdk::GLContext>& /* context */) {
    try {
        m_GLArea.throw_if_error();
        glClearColor(0.5, 1.5, 1.5, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        // Use the shader program
        glUseProgram(m_program);

        // Bind the Vertex Array Object
        glBindVertexArray(m_Vao);

        glBindTexture(GL_TEXTURE_2D, m_texture);

        // Draw the quad
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
        glUseProgram(0);
        glFlush();

        return true;
    }
    catch (const Gdk::GLError& gle) {
        std::cerr << "An error occurred in the render callback of the GLArea" << std::endl;
        std::cerr << gle.domain() << "-" << gle.code() << "-" << gle.what() << std::endl;
        return false;
    }
}

void GtkPlayer::frameReady() {
    if ((m_height != m_frame_data.height || m_width != m_frame_data.width) ||
        (m_isX11 &&
         (m_height < m_mon_work_area.get_height() || m_width < m_mon_work_area.get_width()))) {
        m_height = m_frame_data.height;
        m_width  = m_frame_data.width;

        if (true == m_isX11) {
            if (m_frame_data.height >= m_mon_dim.get_height() &&
                m_frame_data.width >= m_mon_dim.get_width()) {
                // Lock the dimension according to the monitor's dimension
                m_height = m_mon_work_area.get_height();
                m_width  = m_mon_work_area.get_width();
            }
        }

        set_size_request(m_width, m_height);
        queue_resize();
        m_GLArea.set_size_request(m_width, m_height);
        m_GLArea.queue_resize();

        if (m_fullscreenMode) {
            fullscreen();
        }
    }

    glBindTexture(GL_TEXTURE_2D, m_texture);

    EGLAttrib const attribute_list[] = { EGL_WIDTH,
                                         m_frame_data.width,
                                         EGL_HEIGHT,
                                         m_frame_data.height,
                                         EGL_LINUX_DRM_FOURCC_EXT,
                                         DRM_FORMAT_ARGB8888,
                                         EGL_DMA_BUF_PLANE0_FD_EXT,
                                         m_frame_data.fd,
                                         EGL_DMA_BUF_PLANE0_OFFSET_EXT,
                                         m_frame_data.offset[0],
                                         EGL_DMA_BUF_PLANE0_PITCH_EXT,
                                         m_frame_data.stride[0],
                                         EGL_NONE };

    EGLImage image = eglCreateImage(m_egl_display,
                                    nullptr,
                                    EGL_LINUX_DMA_BUF_EXT,
                                    (EGLClientBuffer) nullptr,
                                    attribute_list);
    if (!glIsTexture(m_texture)) {
        std::cerr << "Texture " << m_texture << " is not complete at " << __LINE__ << std::endl;
        return;
    }
    if (image) {
        glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image);
        GL_CHECK(glEGLImageTargetTexture2DOES);

        m_GLArea.queue_draw();
    }
    else {
        std::cerr << "[Err] Image created is Null" << std::endl;
        GL_CHECK(eglCreateImage);
    }
}

GtkPlayer::~GtkPlayer() {
    // Unrealize will perform all the cleanup
}

void GtkPlayer::initShaders() {
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertexShaderSrc, nullptr);
    glCompileShader(vertex_shader);

    GLint success;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(vertex_shader, 512, nullptr, info_log);
        std::cerr << info_log << std::endl;
    }

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragmentShaderSrc, nullptr);
    glCompileShader(fragment_shader);

    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(fragment_shader, 512, nullptr, info_log);
        std::cerr << "FRAGMENT ERROR " << info_log << std::endl;
    }

    m_program = glCreateProgram();
    glAttachShader(m_program, vertex_shader);
    glAttachShader(m_program, fragment_shader);
    glLinkProgram(m_program);

    glGetProgramiv(m_program, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(vertex_shader, 512, nullptr, info_log);
        std::cerr << "FAILED TO LINK PROGRAM" << info_log << std::endl;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

void GtkPlayer::initBuffers() {
    glGenVertexArrays(1, &m_Vao);
    glBindVertexArray(m_Vao);

    glGenBuffers(1, &m_Buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_Buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

    glGenBuffers(1, &m_Ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Attributes
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          4 * sizeof(GLfloat),
                          (void*)(2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Initialize Texture
    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void GtkPlayer::realize() {
    m_GLArea.make_current();
    try {
        m_GLArea.throw_if_error();
        initBuffers();
        initShaders();
    }
    catch (const Gdk::GLError& gle) {
        std::cerr << "An error occured making the context current during realize:" << std::endl;
        std::cerr << gle.domain() << "-" << gle.code() << "-" << gle.what() << std::endl;
    }
}

void GtkPlayer::unrealize() {
    m_GLArea.make_current();
    try {
        m_GLArea.throw_if_error();

        // Delete buffers and program
        glDeleteBuffers(1, &m_Buffer);
        glDeleteBuffers(1, &m_Ebo);
        glDeleteProgram(m_program);
        glDeleteTextures(1, &m_texture);
    }
    catch (const Gdk::GLError& gle) {
        std::cerr << "An error occured making the context current during unrealize" << std::endl;
        std::cerr << gle.domain() << "-" << gle.code() << "-" << gle.what() << std::endl;
    }
}

#endif /* #if defined(LIBVA_GTK4_SUPPORT) */
