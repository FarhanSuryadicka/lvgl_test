#ifndef MOILDEV_OCL_HPP
#define MOILDEV_OCL_HPP

#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_TARGET_OPENCL_VERSION 300

#include "moildev_common.hpp"
#include <CL/opencl.hpp>

using namespace std;

namespace moildev::ocl
{

    /**
     * @brief OpenCL Implementation of Moildev.
     *
     * This class implements the Moildev algorithms using OpenCL for GPU acceleration.
     * It allows for significantly faster map generation on supported hardware.
     */
    class Moildev
    {
    public:
        /**
         * @brief Constructor for OpenCL Moildev.
         *
         * @param cameraSensorWidth sensor width in mm.
         * @param cameraSensorHeight sensor height in mm.
         * @param icx image center x coordinate.
         * @param icy image center y coordinate.
         * @param ratio pixel aspect ratio.
         * @param imageWidth width of the input image.
         * @param imageHeight height of the input image.
         * @param calibrationRatio calibration ratio.
         * @param para0 calibration parameter 0.
         * @param para1 calibration parameter 1.
         * @param para2 calibration parameter 2.
         * @param para3 calibration parameter 3.
         * @param para4 calibration parameter 4.
         * @param para5 calibration parameter 5.
         * @param openCLDeviceID The ID of the OpenCL device to use.
         */
        Moildev(float cameraSensorWidth, float cameraSensorHeight,
                float icx, float icy, float ratio,
                float imageWidth, float imageHeight,
                float calibrationRatio,
                float para0, float para1, float para2,
                float para3, float para4, float para5,
                int openCLDeviceID = 0);

        /**
         * @brief Destructor.
         */
        ~Moildev();

        /**
         * @brief Get list of available OpenCL devices.
         *
         * @return Vector of device names.
         */
        static std::vector<std::string> getOpenCLDevices();

        /**
         * @brief Generate AnyPoint map (Method 1) using OpenCL.
         *
         * @param mapX Pointer to the X-coordinate map buffer.
         * @param mapY Pointer to the Y-coordinate map buffer.
         * @param alpha The alpha angle.
         * @param beta The beta angle.
         * @param zoom The zoom factor.
         * @return 0 on success.
         */
        int AnyPointM(float *mapX, float *mapY, float alpha, float beta, float zoom);

        /**
         * @brief Generate AnyPoint map (Method 2) using OpenCL.
         *
         * @param mapX Pointer to the X-coordinate map buffer.
         * @param mapY Pointer to the Y-coordinate map buffer.
         * @param alpha The alpha angle.
         * @param beta The beta angle.
         * @param zoom The zoom factor.
         * @return 0 on success.
         */
        int AnyPointM2(float *mapX, float *mapY, float alpha, float beta, float zoom);

        /**
         * @brief Generate Panorama Tube map using OpenCL.
         *
         * @param mapX Pointer to the X-coordinate map buffer.
         * @param mapY Pointer to the Y-coordinate map buffer.
         * @param alpha_min Minimum alpha angle.
         * @param alpha_max Maximum alpha angle.
         * @return 0 on success.
         */
        int PanoramaTube(float *mapX, float *mapY, float alpha_min, float alpha_max);

        /**
         * @brief Generate Panorama Car map using OpenCL.
         *
         * @param mapX Pointer to the X-coordinate map buffer.
         * @param mapY Pointer to the Y-coordinate map buffer.
         * @param my_p_alpha_max_vendor Maximum alpha vendor param.
         * @param p_iC_alpha_degree Alpha degree.
         * @param p_iC_beta_degree Beta degree.
         * @param flip_h Flip horizontally.
         * @param flip_v Flip vertically.
         * @return 0 on success.
         */
        int PanoramaCar(float *mapX, float *mapY, float my_p_alpha_max_vendor, float p_iC_alpha_degree, float p_iC_beta_degree, bool flip_h = false, bool flip_v = false);

        /**
         * @brief Generate Reverse Panorama map using OpenCL.
         *
         * @param mapX Pointer to the X-coordinate map buffer.
         * @param mapY Pointer to the Y-coordinate map buffer.
         * @param alpha_max Maximum alpha angle.
         * @param alpha_degree Alpha degree.
         * @param beta_degree Beta degree.
         * @return 0 on success.
         */
        int PanoramaRev(float *mapX, float *mapY, float alpha_max, float alpha_degree, float beta_degree);

        /**
         * @brief Convert panorama coordinates to original image coordinates.
         *
         * @param panorama_x X coordinate in panorama.
         * @param panorama_y Y coordinate in panorama.
         * @param alpha_max Maixmum alpha angle.
         * @return Pair of (x, y) coordinates in original image.
         */
        std::pair<int, int> PanoramaToOriginal(float panorama_x, float panorama_y, float alpha_max);

        /**
         * @brief Get image width.
         * @return Width.
         */
        float getImageWidth() const { return cameraParams.imageWidth; }

        /**
         * @brief Get image height.
         * @return Height.
         */
        float getImageHeight() const { return cameraParams.imageHeight; }

    private:
        int N;
        int openCLDeviceID;
        bool hostUnifiedMemory;
        std::vector<int> alphaRhoTable;
        std::vector<int> rhoAlphaTable;
        CameraParams cameraParams;

        cl::Context context;
        cl::Device device;
        cl::CommandQueue queue;
        cl::Program program;

        cl::Kernel kernel_anypointm;
        cl::Kernel kernel_anypointm2;
        cl::Kernel kernel_panoramatube;
        cl::Kernel kernel_panoramacar;
        cl::Kernel kernel_panoramarev;

        cl::Buffer dev_mapX;
        cl::Buffer dev_mapY;
        cl::Buffer dev_config;

        std::optional<cl::Buffer> dev_anypointm_params;
        std::optional<cl::Buffer> dev_anypointm2_params;
        std::optional<cl::Buffer> dev_panoramatube_params;
        std::optional<cl::Buffer> dev_panoramatube_precalc_alpha;
        std::optional<cl::Buffer> dev_panoramatube_precalc_beta;
        std::optional<cl::Buffer> dev_panoramacar_params;
        std::optional<cl::Buffer> dev_panoramacar_precalc_alpha;
        std::optional<cl::Buffer> dev_panoramacar_precalc_beta;
        std::optional<cl::Buffer> dev_panoramarev_params;
        std::optional<cl::Buffer> dev_panoramarev_rhoalphatab;
        std::optional<cl::Buffer> dev_panoramarev_panoMapX;
        std::optional<cl::Buffer> dev_panoramarev_panoMapY;

        void initOpenCL();
        void initBuffers();
        void initAlphaRhoTable();
        int getRhoFromAlpha(float alpha);
        float getAlphaFromRho(int rho);
    };
}

#endif // MOILDEV_OCL_HPP