#pragma once

#include <vector>
#include <cstdint>
#include <string>

#include "SDL.h"
#ifdef GPU_PRIMITIVES_GL_STANDALONE
#include "SDL_opengl.h"
//#include "SDL_opengl_glext.h"
#endif

namespace frontend {
    namespace ui {
        namespace primitives {
            bool loaded_shader_functions = false;

#ifdef GPU_PRIMITIVES_GL_STANDALONE
            PFNGLCREATESHADERPROC            glCreateShader;
            PFNGLSHADERSOURCEPROC            glShaderSource;
            PFNGLCOMPILESHADERPROC           glCompileShader;
            PFNGLGETSHADERIVPROC             glGetShaderiv;
            PFNGLGETSHADERINFOLOGPROC        glGetShaderInfoLog;
            PFNGLDELETESHADERPROC            glDeleteShader;
            PFNGLATTACHSHADERPROC            glAttachShader;
            PFNGLCREATEPROGRAMPROC           glCreateProgram;
            PFNGLLINKPROGRAMPROC             glLinkProgram;
            PFNGLVALIDATEPROGRAMPROC         glValidateProgram;
            PFNGLGETPROGRAMIVPROC            glGetProgramiv;
            PFNGLGETPROGRAMINFOLOGPROC       glGetProgramInfoLog;
            PFNGLUSEPROGRAMPROC              glUseProgram;
            PFNGLBINDFRAGDATALOCATIONPROC    glBindFragDataLocation;
            PFNGLGETUNIFORMLOCATIONPROC      glGetUniformLocation;
            PFNGLUNIFORM1FPROC               glUniform1f;
            PFNGLUNIFORM2FPROC               glUniform2f;
            PFNGLUNIFORM3FPROC               glUniform3f;
            PFNGLUNIFORM4FPROC               glUniform4f;
            PFNGLUNIFORM1IPROC               glUniform1i;
            PFNGLGENVERTEXARRAYSPROC         glGenVertexArrays;
            PFNGLBINDVERTEXARRAYPROC         glBindVertexArray;
            PFNGLGENBUFFERSPROC              glGenBuffers;
            PFNGLBINDBUFFERPROC              glBindBuffer;
            PFNGLBUFFERDATAPROC              glBufferData;
            PFNGLVERTEXATTRIBPOINTERPROC     glVertexAttribPointer;
            PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
            PFNGLACTIVETEXTUREPROC           glActiveTexture1;
            PFNGLGENFRAMEBUFFERSPROC         glGenFramebuffers;
            PFNGLBINDFRAMEBUFFERPROC         glBindFramebuffer;
            PFNGLFRAMEBUFFERTEXTUREPROC      glFramebufferTexture;
            PFNGLFRAMEBUFFERTEXTURE2DPROC    glFramebufferTexture2D;
            PFNGLDRAWBUFFERSPROC             glDrawBuffers;
            PFNGLCHECKFRAMEBUFFERSTATUSPROC  glCheckFramebufferStatus;
            PFNGLDELETEFRAMEBUFFERSPROC      glDeleteFramebuffers;
            PFNGLDELETEVERTEXARRAYSPROC      glDeleteVertexArrays;
            PFNGLDELETEBUFFERSPROC           glDeleteBuffers;
#endif

        void load_shader_functions() {
#ifdef GPU_PRIMITIVES_GL_STANDALONE
                _log(debug, "loading shader functions");
                _log(debug, "glCreateShader=%p", (PFNGLCREATESHADERPROC)SDL_GL_GetProcAddress("glCreateShader");)
                glCreateShader            = (PFNGLCREATESHADERPROC)SDL_GL_GetProcAddress("glCreateShader");
                glShaderSource            = (PFNGLSHADERSOURCEPROC)SDL_GL_GetProcAddress("glShaderSource");
                glCompileShader           = (PFNGLCOMPILESHADERPROC)SDL_GL_GetProcAddress("glCompileShader");
                glGetShaderiv             = (PFNGLGETSHADERIVPROC)SDL_GL_GetProcAddress("glGetShaderiv");
                glGetShaderInfoLog        = (PFNGLGETSHADERINFOLOGPROC)SDL_GL_GetProcAddress("glGetShaderInfoLog");
                glDeleteShader            = (PFNGLDELETESHADERPROC)SDL_GL_GetProcAddress("glDeleteShader");
                glAttachShader            = (PFNGLATTACHSHADERPROC)SDL_GL_GetProcAddress("glAttachShader");
                glCreateProgram           = (PFNGLCREATEPROGRAMPROC)SDL_GL_GetProcAddress("glCreateProgram");
                glLinkProgram             = (PFNGLLINKPROGRAMPROC)SDL_GL_GetProcAddress("glLinkProgram");
                glValidateProgram         = (PFNGLVALIDATEPROGRAMPROC)SDL_GL_GetProcAddress("glValidateProgram");
                glGetProgramiv            = (PFNGLGETPROGRAMIVPROC)SDL_GL_GetProcAddress("glGetProgramiv");
                glGetProgramInfoLog       = (PFNGLGETPROGRAMINFOLOGPROC)SDL_GL_GetProcAddress("glGetProgramInfoLog");
                glUseProgram              = (PFNGLUSEPROGRAMPROC)SDL_GL_GetProcAddress("glUseProgram");
                glBindFragDataLocation    = (PFNGLBINDFRAGDATALOCATIONPROC)SDL_GL_GetProcAddress("glBindFragDataLocation");
                glGetUniformLocation      = (PFNGLGETUNIFORMLOCATIONPROC)SDL_GL_GetProcAddress("glGetUniformLocation");
                glUniform1f               = (PFNGLUNIFORM1FPROC)SDL_GL_GetProcAddress("glUniform1f");
                glUniform2f               = (PFNGLUNIFORM2FPROC)SDL_GL_GetProcAddress("glUniform2f");
                glUniform3f               = (PFNGLUNIFORM3FPROC)SDL_GL_GetProcAddress("glUniform3f");
                glUniform4f               = (PFNGLUNIFORM4FPROC)SDL_GL_GetProcAddress("glUniform4f");
                glUniform1i               = (PFNGLUNIFORM1IPROC)SDL_GL_GetProcAddress("glUniform1i");
                glGenVertexArrays         = (PFNGLGENVERTEXARRAYSPROC)SDL_GL_GetProcAddress("glGenVertexArrays");
                glBindVertexArray         = (PFNGLBINDVERTEXARRAYPROC)SDL_GL_GetProcAddress("glBindVertexArray");
                glGenBuffers              = (PFNGLGENBUFFERSPROC)SDL_GL_GetProcAddress("glGenBuffers");
                glBindBuffer              = (PFNGLBINDBUFFERPROC)SDL_GL_GetProcAddress("glBindBuffer");
                glBufferData              = (PFNGLBUFFERDATAPROC)SDL_GL_GetProcAddress("glBufferData");
                glVertexAttribPointer     = (PFNGLVERTEXATTRIBPOINTERPROC)SDL_GL_GetProcAddress("glVertexAttribPointer");
                glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)SDL_GL_GetProcAddress("glEnableVertexAttribArray");
                glActiveTexture1          = (PFNGLACTIVETEXTUREPROC)SDL_GL_GetProcAddress("glActiveTexture");
                glGenFramebuffers         = (PFNGLGENFRAMEBUFFERSPROC)SDL_GL_GetProcAddress("glGenFramebuffers");
                glBindFramebuffer         = (PFNGLBINDFRAMEBUFFERPROC)SDL_GL_GetProcAddress("glBindFramebuffer");
                glFramebufferTexture      = (PFNGLFRAMEBUFFERTEXTUREPROC)SDL_GL_GetProcAddress("glFramebufferTexture");
                glFramebufferTexture2D    = (PFNGLFRAMEBUFFERTEXTURE2DPROC)SDL_GL_GetProcAddress("glFramebufferTexture2D");
                glDrawBuffers             = (PFNGLDRAWBUFFERSPROC)SDL_GL_GetProcAddress("glDrawBuffers");
                glCheckFramebufferStatus  = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)SDL_GL_GetProcAddress("glCheckFramebufferStatus");
                glDeleteFramebuffers      = (PFNGLDELETEFRAMEBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteFramebuffers");
                glDeleteVertexArrays      = (PFNGLDELETEVERTEXARRAYSPROC)SDL_GL_GetProcAddress("glDeleteVertexArrays");
                glDeleteBuffers           = (PFNGLDELETEBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteBuffers");
#endif

                loaded_shader_functions = true;
            }

            std::vector <uint32_t> buf;

            SDL_Texture* texture = nullptr;

            static const std::string fragment_shader_header =
                "#version 440\n"
                "in vec2 pos;\n"
                "out vec4 out_color;\n"
                "uniform vec3 resolution;\n";
            
            static const std::string vertex_shader_header =
                "#version 120\n"
                "varying out vec2 pos;\n"
                "uniform vec3 resolution;\n"
                "uniform vec2 window_resolution;\n";
            
            static const std::string identity_vertex_source =
                "void main() {\n"
                "    gl_Position = gl_Vertex;\n"
                "    pos = (gl_Position.xy + 1.0) * resolution.xy / 2.0;\n"
                "    pos /= resolution.xy / window_resolution.xy;\n"
                "}";

            static const std::string rounded_rect_filled_source =
                "uniform vec3 color;\n"
                "uniform float radius;\n"
                "float rounded_box_sdf(vec2 size, float radius) {\n"
                "    return length(max(abs(pos.xy - (resolution.xy / 2.0)) - size + radius, 0.0)) - radius;\n"
                "}\n"
                "void main() {\n"
                "    float mradius = radius;\n"
                "    if (mradius == 0.0) mradius = resolution.y / 2.0;\n"
                "    float edge_softness = 1.0;\n"
                "    float d = rounded_box_sdf((resolution.xy / 2.0) - 1.0, mradius);\n"
                "    float smoothed = 1.0 - smoothstep(0.0, edge_softness * 2.0, d);\n"
                "    out_color = mix(vec4(0.0), vec4((color / 255.0), smoothed), smoothed);\n"
                "}";

            struct shader_t {
                SDL_Window* window;

                std::string vert_src, frag_src;

                GLuint program;
                
                int width, height;

                GLuint framebuffer, framebuffer_texture;

                GLuint vao, vbo;

                bool va_created = false, fb_created = false;
            };

            bool shader_has_framebuffer(shader_t* shader) {
                if (!shader) return false;

                return shader->fb_created;
            }

            bool create_shader_framebuffer(shader_t* shader, int width, int height) {
                shader->width = width;
                shader->height = height;

                if (!shader->va_created) {
                    // Generate and bind VAO
                    glGenVertexArrays(1, &shader->vao);
                    glBindVertexArray(shader->vao);

                    // Generate VBO
                    glGenBuffers(1, &shader->vbo);

                    static GLfloat const quad[12] = {
                        -1.f, -1.f, 0.f,
                        -1.f, +1.f, 0.f,
                        +1.f, -1.f, 0.f,
                        +1.f, +1.f, 0.f,
                    };

                    // Bind VBO
                    glBindBuffer(GL_ARRAY_BUFFER, shader->vbo);
                    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
                    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
                    glEnableVertexAttribArray(0);

                    shader->va_created = true;
                }

                glGenFramebuffers(1, &shader->framebuffer);
                glBindFramebuffer(GL_FRAMEBUFFER, shader->framebuffer);

                glGenTextures(1, &shader->framebuffer_texture);
                glBindTexture(GL_TEXTURE_2D, shader->framebuffer_texture);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, shader->width, shader->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

                glBindTexture(GL_TEXTURE_2D, 0);

                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, shader->framebuffer_texture, 0);

                // Set the list of draw buffers.
                GLenum draw_buffers[1] = { GL_COLOR_ATTACHMENT0 };
                glDrawBuffers(1, draw_buffers);

                if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                    SDL_SetError("Shader framebuffer creation failed, status=%u", glCheckFramebufferStatus(GL_FRAMEBUFFER));

                    shader->fb_created = false;

                    return false;
                }

                glBindFramebuffer(GL_FRAMEBUFFER, 0);

                shader->fb_created = true;

                return true;
            }

            void delete_shader_framebuffer(shader_t* shader) {
                glDeleteFramebuffers(1, &shader->framebuffer);
                glDeleteTextures(1, &shader->framebuffer_texture);

                shader->fb_created = false;
            }

            void reload_shader_framebuffer(shader_t* shader, int width, int height) {
                delete_shader_framebuffer(shader);
                create_shader_framebuffer(shader, width, height);
            }

            bool compile_shader(GLuint shader, const char* source, GLint* log_size = nullptr) {
                GLint result;

                glShaderSource(shader, 1, &source, NULL);
                glCompileShader(shader);

                glGetShaderiv(shader, GL_COMPILE_STATUS, &result);

                if (log_size)
                    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, log_size);

                return result != 0;
            }

            shader_t* create_shader(SDL_Window* window, std::string vertex_shader_src, std::string fragment_shader_src, bool append_headers = true) {
                _log(debug, "load_shader_functions()");
                if (!loaded_shader_functions) load_shader_functions();

                _log(debug, "glCreateShader()");
                GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER),
                       frag_shader = glCreateShader(GL_FRAGMENT_SHADER);

                shader_t* shader = new shader_t();

                _log(debug, "shader = new shader_t()");
                shader->window = window;

                if (append_headers) {
                    vertex_shader_src = vertex_shader_header + vertex_shader_src;
                    fragment_shader_src = fragment_shader_header + fragment_shader_src;
                }

                _log(debug, "append_header()");
                const char* vert_src_ptr = vertex_shader_src.c_str();
                const char* frag_src_ptr = fragment_shader_src.c_str();

                GLint log_size = 0;

                if (!compile_shader(vert_shader, vert_src_ptr, &log_size)) {
                    SDL_SetError("Vertex shader compilation failed");

                    delete shader;
                    return nullptr;
                }

                if (!compile_shader(frag_shader, frag_src_ptr, &log_size)) {
                    std::vector <char> log;

                    log.resize(log_size);

                    glGetShaderInfoLog(frag_shader, log_size, &log_size, log.data());

                    SDL_SetError("%s", log.data());

                    delete shader;
                    return nullptr;
                }

                shader->program = glCreateProgram();
                glAttachShader(shader->program, vert_shader);
                glAttachShader(shader->program, frag_shader);
                glLinkProgram(shader->program);

                GLint result;

                glValidateProgram(shader->program);
                glGetProgramiv(shader->program, GL_LINK_STATUS, &result);
                glGetProgramiv(shader->program, GL_INFO_LOG_LENGTH, &log_size);

                if (!result) {
                    SDL_SetError("Shader link failed");

                    delete shader;
                    return nullptr;
                }
                
                glDeleteShader(vert_shader);
                glDeleteShader(frag_shader);

                return shader;
            }

            shader_t* rounded_rect_filled_shader;

            void init(SDL_Window* window) {
                rounded_rect_filled_shader = create_shader(window, identity_vertex_source, rounded_rect_filled_source);
            }

            void setup_render(SDL_Renderer* renderer, shader_t* shader, int width, int height) {
                if ((width * height) != buf.size()) {
                    buf.resize(width * height);

                    if (texture) SDL_DestroyTexture(texture);

                    texture = SDL_CreateTexture(
                        renderer,
                        SDL_PIXELFORMAT_ABGR8888,
                        SDL_TEXTUREACCESS_STREAMING,
                        width, height
                    );
                }
                
                if (!shader_has_framebuffer(shader)) {
                    create_shader_framebuffer(shader, width, height);
                } else {
                    reload_shader_framebuffer(shader, width, height);
                }
                
                glBindFramebuffer(GL_FRAMEBUFFER, shader->framebuffer);

                int window_width = 0, window_height = 0;

                SDL_GetWindowSize(shader->window, &window_width, &window_height);

                glUseProgram(shader->program);

                glUniform3f(glGetUniformLocation(shader->program, "resolution"), (double)width, (double)height, 0.0f);
                glUniform2f(glGetUniformLocation(shader->program, "window_resolution"), (double)window_width, (double)window_height);
            }

            void end_render(shader_t* shader, int width, int height) {
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

                glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buf.data());

                glBindFramebuffer(GL_FRAMEBUFFER, 0);
            }

            void copy_buf_to_renderer(SDL_Renderer* renderer, int x, int y, int width, int height) {
                SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

                SDL_UpdateTexture(
                    texture,
                    NULL,
                    buf.data(),
                    width * sizeof(uint32_t)
                );

                //SDL_SetTextureScaleMode(texture, SDL_ScaleModeLinear);

                SDL_Rect rect;

                rect.x = x;
                rect.y = y;
                rect.w = width;
                rect.h = height;

                SDL_RenderCopyEx(renderer, texture, NULL, &rect, 0.0, NULL, SDL_FLIP_NONE);

                SDL_DestroyTexture(texture);
            }

            void render_rounded_rectangle_filled(SDL_Renderer* renderer, int x, int y, int width, int height, int r, int g, int b, double radius) {
                setup_render(renderer, rounded_rect_filled_shader, width, height);

                glUniform3f(
                    glGetUniformLocation(rounded_rect_filled_shader->program, "color"),
                    (double)r, (double)g, (double)b
                );

                glUniform1f(glGetUniformLocation(rounded_rect_filled_shader->program, "radius"), radius);

                end_render(rounded_rect_filled_shader, width, height);

                copy_buf_to_renderer(renderer, x, y, width, height);
            }
        };
    }
}