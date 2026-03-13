#include "shader_render.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

#ifndef TARGET_RENESAS
#include <SDL2/SDL.h>
// Variabel global untuk menahan fokus GPU
static SDL_Window* hidden_gl_win = nullptr;
static SDL_GLContext hidden_gl_ctx = nullptr;
#endif

ShaderRenderer::ShaderRenderer() : shaderProgram(0), inputTexture(0), fbo(0), fboTexture(0), quadVAO(0), quadVBO(0), current_w(0), current_h(0) {}

ShaderRenderer::~ShaderRenderer() {
    if (shaderProgram) glDeleteProgram(shaderProgram);
    if (inputTexture) glDeleteTextures(1, &inputTexture);
    if (fboTexture) glDeleteTextures(1, &fboTexture);
    if (fbo) glDeleteFramebuffers(1, &fbo);
    if (quadVAO) glDeleteVertexArrays(1, &quadVAO);
    if (quadVBO) glDeleteBuffers(1, &quadVBO);
}

uint32_t ShaderRenderer::compileShader(uint32_t type, const std::string& source) {
    uint32_t id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);
    
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (!result) {
        char length[512];
        glGetShaderInfoLog(id, 512, nullptr, length);
        std::cerr << "[GL ERROR] Shader Compilation Failed!\n" << length << std::endl;
        return 0;
    }
    return id;
}

bool ShaderRenderer::init(const std::string& fragPath) {
#ifndef TARGET_RENESAS
    // [PERBAIKAN KUNCI]: Buat Jendela Rahasia & Konteks OpenGL agar tidak direbut LVGL
    hidden_gl_win = SDL_CreateWindow("HiddenGL", 0, 0, 10, 10, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
    if(hidden_gl_win) {
        hidden_gl_ctx = SDL_GL_CreateContext(hidden_gl_win);
        SDL_GL_MakeCurrent(hidden_gl_win, hidden_gl_ctx);
    }

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "[GL ERROR] Failed to initialize GLEW" << std::endl;
        return false;
    }
#endif

    std::ifstream fragFile(fragPath);
    if (!fragFile.is_open()) {
        std::cerr << "[GL ERROR] Tidak bisa membuka file shader: " << fragPath << std::endl;
        return false;
    }
    std::stringstream fragStream;
    fragStream << fragFile.rdbuf();
    std::string fragCode = fragStream.str();

    size_t pos = fragCode.find("#version");
    if(pos != std::string::npos) {
        size_t endPos = fragCode.find('\n', pos);
        fragCode.erase(pos, endPos - pos + 1);
    }

#ifdef TARGET_RENESAS
    std::string finalFragSource = "#version 300 es\nprecision highp float;\n" + fragCode;
    std::string vertexShaderSource = "#version 300 es\nlayout(location=0) in vec2 aPos;\nvoid main(){gl_Position=vec4(aPos,0.0,1.0);}";
#else
    std::string finalFragSource = "#version 330 core\n" + fragCode;
    std::string vertexShaderSource = "#version 330 core\nlayout (location = 0) in vec2 aPos;\nvoid main() { gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0); }";
#endif

    uint32_t vs = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    uint32_t fs = compileShader(GL_FRAGMENT_SHADER, finalFragSource);
    if(vs == 0 || fs == 0) return false;

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);

    float quadVertices[] = { -1.0f,  1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, 1.0f,  1.0f };
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    glGenFramebuffers(1, &fbo);
    glGenTextures(1, &inputTexture);
    glGenTextures(1, &fboTexture);

    std::cout << "[INFO] GPU Shader Berhasil Diinisialisasi!" << std::endl;
    return true;
}

void ShaderRenderer::render(const cv::Mat& raw_frame, int target_w, int target_h,
                            float uMode, float alpha, float beta, float zoom, float alphaMax,
                            std::vector<uint16_t>& out_lvgl_buffer) {
    if (shaderProgram == 0 || target_w <= 0 || target_h <= 0 || raw_frame.empty()) return;

#ifndef TARGET_RENESAS
    if(hidden_gl_win && hidden_gl_ctx) {
        SDL_GL_MakeCurrent(hidden_gl_win, hidden_gl_ctx);
    }
#endif

    if (current_w != target_w || current_h != target_h) {
        glBindTexture(GL_TEXTURE_2D, fboTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, target_w, target_h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture, 0);

        GLenum fb_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (fb_status != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "[GL ERROR] Framebuffer incomplete: " << fb_status << std::endl;
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            return;
        }

        current_w = target_w;
        current_h = target_h;
    }

    // Penting: data 3-channel raw_frame harus alignment 1
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glBindTexture(GL_TEXTURE_2D, inputTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, raw_frame.cols, raw_frame.rows, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, raw_frame.data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, target_w, target_h);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);

    glUniform1f(glGetUniformLocation(shaderProgram, "uMode"), uMode);
    glUniform4f(glGetUniformLocation(shaderProgram, "uControl"), alpha, beta, zoom, alphaMax);
    glUniform2f(glGetUniformLocation(shaderProgram, "uResolution"), (float)target_w, (float)target_h);
    glUniform2f(glGetUniformLocation(shaderProgram, "uMoilSize"), (float)raw_frame.cols, (float)raw_frame.rows);

    float scale_factor = (float)raw_frame.cols / 2592.0f;
    glUniform2f(glGetUniformLocation(shaderProgram, "uMoilCenter"),
                1238.0f * scale_factor, 987.0f * scale_factor);
    glUniform1f(glGetUniformLocation(shaderProgram, "uCpRatio"), scale_factor);

    glUniform1f(glGetUniformLocation(shaderProgram, "p0"), 0.0f);
    glUniform1f(glGetUniformLocation(shaderProgram, "p1"), 0.0f);
    glUniform1f(glGetUniformLocation(shaderProgram, "p2"), -32.973f);
    glUniform1f(glGetUniformLocation(shaderProgram, "p3"), 67.825f);
    glUniform1f(glGetUniformLocation(shaderProgram, "p4"), -41.581f);
    glUniform1f(glGetUniformLocation(shaderProgram, "p5"), 504.7f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTexture);
    glUniform1i(glGetUniformLocation(shaderProgram, "uTexture"), 0);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Penting: baca 3-channel ke cv::Mat tanpa padding OpenGL
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    cv::Mat rgbMat(target_h, target_w, CV_8UC3);
    glReadPixels(0, 0, target_w, target_h, GL_BGR, GL_UNSIGNED_BYTE, rgbMat.data);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    cv::Mat flippedMat, rgb565Mat;
    cv::flip(rgbMat, flippedMat, 0);
    cv::cvtColor(flippedMat, rgb565Mat, cv::COLOR_RGB2BGR565);

    size_t needed = (size_t)target_w * (size_t)target_h;
    if(out_lvgl_buffer.size() != needed) {
        out_lvgl_buffer.resize(needed);
    }

    std::memcpy(out_lvgl_buffer.data(), rgb565Mat.data, needed * sizeof(uint16_t));
}