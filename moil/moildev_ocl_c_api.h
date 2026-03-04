#ifndef MOILDEV_OCL_C_API_H
#define MOILDEV_OCL_C_API_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
    #ifdef BUILDING_MOILDEV_OCL
        #define MOILDEV_OCL_API __declspec(dllexport)
    #else
        #define MOILDEV_OCL_API __declspec(dllimport)
    #endif
#else
    #define MOILDEV_OCL_API __attribute__((visibility("default")))
#endif



/**
 * @brief Create a new Moildev OpenCL instance.
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
 * @return Opaque pointer to the Moildev instance, or NULL on failure.
 */
MOILDEV_OCL_API void* moildev_ocl_create(
    float cameraSensorWidth, float cameraSensorHeight,
    float icx, float icy, float ratio,
    float imageWidth, float imageHeight,
    float calibrationRatio,
    float para0, float para1, float para2,
    float para3, float para4, float para5,
    int openCLDeviceID
);

/**
 * @brief Destroy a Moildev OpenCL instance.
 *
 * @param instance Opaque pointer to the Moildev instance.
 */
MOILDEV_OCL_API void moildev_ocl_destroy(void* instance);

/**
 * @brief Generate AnyPointM map.
 *
 * @param instance Opaque pointer to the Moildev instance.
 * @param mapX Pointer to the X-coordinate map buffer.
 * @param mapY Pointer to the Y-coordinate map buffer.
 * @param alpha The alpha angle.
 * @param beta The beta angle.
 * @param zoom The zoom factor.
 * @return 0 on success.
 */
MOILDEV_OCL_API int moildev_ocl_anypointm(
    void* instance,
    float* mapX, float* mapY,
    float alpha, float beta, float zoom
);

/**
 * @brief Generate AnyPointM2 map.
 *
 * @param instance Opaque pointer to the Moildev instance.
 * @param mapX Pointer to the X-coordinate map buffer.
 * @param mapY Pointer to the Y-coordinate map buffer.
 * @param alpha The alpha angle.
 * @param beta The beta angle.
 * @param zoom The zoom factor.
 * @return 0 on success.
 */
MOILDEV_OCL_API int moildev_ocl_anypointm2(
    void* instance,
    float* mapX, float* mapY,
    float alpha, float beta, float zoom
);

/**
 * @brief Generate PanoramaTube map.
 *
 * @param instance Opaque pointer to the Moildev instance.
 * @param mapX Pointer to the X-coordinate map buffer.
 * @param mapY Pointer to the Y-coordinate map buffer.
 * @param alpha_min Minimum alpha angle.
 * @param alpha_max Maximum alpha angle.
 * @return 0 on success.
 */
MOILDEV_OCL_API int moildev_ocl_panoramatube(
    void* instance,
    float* mapX, float* mapY,
    float alpha_min, float alpha_max
);

/**
 * @brief Generate PanoramaCar map.
 *
 * @param instance Opaque pointer to the Moildev instance.
 * @param mapX Pointer to the X-coordinate map buffer.
 * @param mapY Pointer to the Y-coordinate map buffer.
 * @param my_p_alpha_max_vendor Maximum alpha vendor param.
 * @param p_iC_alpha_degree Alpha degree.
 * @param p_iC_beta_degree Beta degree.
 * @param flip_h Flip horizontally.
 * @param flip_v Flip vertically.
 * @return 0 on success.
 */
MOILDEV_OCL_API int moildev_ocl_panoramacar(
    void* instance,
    float* mapX, float* mapY,
    float my_p_alpha_max_vendor,
    float p_iC_alpha_degree,
    float p_iC_beta_degree,
    bool flip_h, bool flip_v
);

/**
 * @brief Generate PanoramaRev map.
 *
 * @param instance Opaque pointer to the Moildev instance.
 * @param mapX Pointer to the X-coordinate map buffer.
 * @param mapY Pointer to the Y-coordinate map buffer.
 * @param alpha_max Maximum alpha angle.
 * @param alpha_degree Alpha degree.
 * @param beta_degree Beta degree.
 * @return 0 on success.
 */
MOILDEV_OCL_API int moildev_ocl_panoramarev(
    void* instance,
    float* mapX, float* mapY,
    float alpha_max,
    float alpha_degree,
    float beta_degree
);

/**
 * @brief Convert panorama coordinates to original fisheye coordinates.
 *
 * @param instance Opaque pointer to the Moildev instance.
 * @param panorama_x X coordinate in panorama.
 * @param panorama_y Y coordinate in panorama.
 * @param alpha_max Maximum alpha angle.
 * @param out_origX Pointer to verify X coordinate output.
 * @param out_origY Pointer to verify Y coordinate output.
 * @return 0 on success, -1 on error.
 */
MOILDEV_OCL_API int moildev_ocl_panoramatooriginal(
    void* instance,
    float panorama_x, 
    float panorama_y, 
    float alpha_max,
    int* out_origX,
    int* out_origY
);

/**
 * @brief Get image width.
 * @param instance Opaque pointer to the Moildev instance.
 * @return Width as a float.
 */
MOILDEV_OCL_API float moildev_ocl_get_image_width(void* instance);

/**
 * @brief Get image height.
 * @param instance Opaque pointer to the Moildev instance.
 * @return Height as a float.
 */
MOILDEV_OCL_API float moildev_ocl_get_image_height(void* instance);

#ifdef __cplusplus
}
#endif

#endif // MOILDEV_OCL_C_API_H
