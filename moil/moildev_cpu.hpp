#ifndef MOILDEV_CPU_HPP
#define MOILDEV_CPU_HPP

#include "moildev_common.hpp"
#include <string>


using namespace std;

namespace moildev::cpu
{
    /**
     * @brief CPU Implementation of Moildev.
     *
     * This class implements the Moildev algorithms using standard CPU instructions.
     * It serves as the baseline implementation and fallback when OpenCL is not available.
     */
    class Moildev
    {
    public:
        /**
         * @brief Constructor for CPU Moildev.
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
         */
        Moildev(float cameraSensorWidth, float cameraSensorHeight,
                float icx, float icy, float ratio,
                float imageWidth, float imageHeight,
                float calibrationRatio,
                float para0, float para1, float para2,
                float para3, float para4, float para5);

        /**
         * @brief Destructor.
         */
        ~Moildev();

        /**
         * @brief Generate AnyPoint map (Method 1).
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
         * @brief Generate AnyPoint map (Method 2).
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
         * @brief Generate Panorama Tube map.
         *
         * @param mapX Pointer to the X-coordinate map buffer.
         * @param mapY Pointer to the Y-coordinate map buffer.
         * @param alpha_min Minimum alpha angle.
         * @param alpha_max Maximum alpha angle.
         * @return 0 on success.
         */
        int PanoramaTube(float *mapX, float *mapY, float alpha_min, float alpha_max);

        /**
         * @brief Generate Panorama Car map.
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
         * @brief Generate Reverse Panorama map.
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
        std::vector<int> alphaRhoTable;
        std::vector<int> rhoAlphaTable;
        CameraParams cameraParams;
        void initAlphaRhoTable();
        int getRhoFromAlpha(float alpha);
        float getAlphaFromRho(int rho);
    };
}

#endif // MOILDEV_CPU_HPP