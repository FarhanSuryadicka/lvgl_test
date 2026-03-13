#pragma once
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>

#ifdef TARGET_RENESAS
    #include <GLES3/gl3.h>
#else
    #include <GL/glew.h>
#endif

class ShaderRenderer {
public:
    ShaderRenderer();
    ~ShaderRenderer();

    bool init(const std::string& fragPath);
    void render(const cv::Mat& raw_frame, int target_w, int target_h, 
                float uMode, float alpha, float beta, float zoom, float alphaMax,
                std::vector<uint16_t>& out_lvgl_buffer);

private:
    uint32_t shaderProgram;
    uint32_t inputTexture;
    uint32_t fbo;
    uint32_t fboTexture;
    uint32_t quadVAO, quadVBO;
    int current_w, current_h;

    uint32_t compileShader(uint32_t type, const std::string& source);
};