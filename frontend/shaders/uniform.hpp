#pragma once

#ifdef GEEBLY_SHADERS_GL_STANDALONE
#include "gl.hpp"
#endif

#include <type_traits>

namespace shaders {
    class base_uniform_t {
    public:
        virtual void update() = 0;
        virtual std::string name() = 0;
    };

    template <class T, short D> class uniform_t : public base_uniform_t {
        T* variables[D];
        std::string name;
        GLuint program;

    public:
        uniform_t(T* variables[D], std::string name, GLuint program) {
            static_assert((D > 0) && (D < 5), "Uniform dimension should be between 1 and 4");
            static_assert(
                (std::is_same_v(T, float)) ||
                (std::is_same_v(T, int)) ||
                (std::is_same_v(T, unsigned int)),
                "Uniform type should be either float, int or unsigned int"
            );

            this->variables = variables;
            this->name = name;
            this->program = program;
        }

        std::string name() override {
            return this->name;
        }

        void update(int program_override = -1) override {
            GLint location;

            if (program_override == -1) {
                location = glGetUniformLocation(program, name);
            } else {
                location = glGetUniformLocation(program_override, name);
            }

            if constexpr (D == 1) {
                if constexpr (std::is_same_v(T, float       )) { glUniform1f(location, *variable[0]); }
                if constexpr (std::is_same_v(T, int         )) { glUniform1i(location, *variable[0]); }
                if constexpr (std::is_same_v(T, unsigned int)) { glUniform1ui(location, *variable[0]); }
            }
            if constexpr (D == 2) {
                if constexpr (std::is_same_v(T, float       )) { glUniform2f(location, *variable[0], *variable[1]); }
                if constexpr (std::is_same_v(T, int         )) { glUniform2i(location, *variable[0], *variable[1]); }
                if constexpr (std::is_same_v(T, unsigned int)) { glUniform2ui(location, *variable[0], *variable[1]); }
            }
            if constexpr (D == 3) {
                if constexpr (std::is_same_v(T, float       )) { glUniform3f(location, *variable[0], *variable[1], *variable[2]); }
                if constexpr (std::is_same_v(T, int         )) { glUniform3i(location, *variable[0], *variable[1], *variable[2]); }
                if constexpr (std::is_same_v(T, unsigned int)) { glUniform3ui(location, *variable[0], *variable[1], *variable[2]); }
            }
            if constexpr (D == 4) {
                if constexpr (std::is_same_v(T, float       )) { glUniform4f(location, *variable[0], *variable[1], *variable[2], *variable[3]); }
                if constexpr (std::is_same_v(T, int         )) { glUniform4i(location, *variable[0], *variable[1], *variable[2], *variable[3]); }
                if constexpr (std::is_same_v(T, unsigned int)) { glUniform4ui(location, *variable[0], *variable[1], *variable[2], *variable[3]); }
            }
        }
    };
}