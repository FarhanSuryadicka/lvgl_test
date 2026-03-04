#ifndef MOILDEV_PLUGIN_HPP
#define MOILDEV_PLUGIN_HPP

#include <vector>
#include <string>

#ifdef _WIN32
    #ifdef BUILDING_MOILDEV_PLUGIN
        #define MOILDEV_PLUGIN_API __declspec(dllexport)
    #else
        #define MOILDEV_PLUGIN_API __declspec(dllimport)
    #endif
#else
    #define MOILDEV_PLUGIN_API __attribute__((visibility("default")))
#endif

/**
 * @brief Plugin interface for Moildev backends.
 *
 * Each backend (CPU, OpenCL) implements this interface and exports
 * moildev_create_plugin() and moildev_destroy_plugin() functions.
 */
struct IMoildevPlugin {
    // Instance lifecycle
    /**
     * @brief Create a new backend instance.
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
     * @param openCLDeviceID The ID of the OpenCL device to use (OpenCL backend only).
     * @return Void pointer to the created instance.
     */
    virtual void* create(
        float cameraSensorWidth, float cameraSensorHeight,
        float icx, float icy, float ratio,
        float imageWidth, float imageHeight,
        float calibrationRatio,
        float para0, float para1, float para2,
        float para3, float para4, float para5,
        int openCLDeviceID = 0) = 0;

    /**
     * @brief Destroy a backend instance.
     * @param instance The instance pointer to destroy.
     */
    virtual void destroy(void* instance) = 0;

    // Map generation functions
    /**
     * @brief Generate AnyPointM map.
     *
     * @param instance The backend instance.
     * @param mapX X-coordinate map buffer.
     * @param mapY Y-coordinate map buffer.
     * @param alpha Alpha angle.
     * @param beta Beta angle.
     * @param zoom Zoom factor.
     * @return 0 on success.
     */
    virtual int AnyPointM(void* instance, float* mapX, float* mapY,
                          float alpha, float beta, float zoom) = 0;

    /**
     * @brief Generate AnyPointM2 map.
     *
     * @param instance The backend instance.
     * @param mapX X-coordinate map buffer.
     * @param mapY Y-coordinate map buffer.
     * @param alpha Alpha angle.
     * @param beta Beta angle.
     * @param zoom Zoom factor.
     * @return 0 on success.
     */
    virtual int AnyPointM2(void* instance, float* mapX, float* mapY,
                           float alpha, float beta, float zoom) = 0;

    /**
     * @brief Generate PanoramaTube map.
     *
     * @param instance The backend instance.
     * @param mapX X-coordinate map buffer.
     * @param mapY Y-coordinate map buffer.
     * @param alpha_min Minimum alpha angle.
     * @param alpha_max Maximum alpha angle.
     * @return 0 on success.
     */
    virtual int PanoramaTube(void* instance, float* mapX, float* mapY,
                             float alpha_min, float alpha_max) = 0;

    /**
     * @brief Generate PanoramaCar map.
     *
     * @param instance The backend instance.
     * @param mapX X-coordinate map buffer.
     * @param mapY Y-coordinate map buffer.
     * @param p_alpha_max Maximum alpha angle.
     * @param p_iC_alpha Alpha degree.
     * @param p_iC_beta Beta degree.
     * @param flip_h Flip horizontally.
     * @param flip_v Flip vertically.
     * @return 0 on success.
     */
    virtual int PanoramaCar(void* instance, float* mapX, float* mapY,
                            float p_alpha_max, float p_iC_alpha, float p_iC_beta,
                            bool flip_h, bool flip_v) = 0;

    /**
     * @brief Generate PanoramaRev map.
     *
     * @param instance The backend instance.
     * @param mapX X-coordinate map buffer.
     * @param mapY Y-coordinate map buffer.
     * @param alpha_max Maximum alpha angle.
     * @param alpha_degree Alpha degree.
     * @param beta_degree Beta degree.
     * @return 0 on success.
     */
    virtual int PanoramaRev(void* instance, float* mapX, float* mapY,
                            float alpha_max, float alpha_degree, float beta_degree) = 0;

    /**
     * @brief Convert panorama coordinates to original image coordinates.
     *
     * @param instance The backend instance.
     * @param panorama_x X coordinate in panorama.
     * @param panorama_y Y coordinate in panorama.
     * @param alpha_max Maximum alpha angle.
     * @return Pair of (x, y) coordinates in original image.
     */
    virtual std::pair<int, int> PanoramaToOriginal(void* instance, float panorama_x, float panorama_y,
                                                   float alpha_max) = 0;

    // Getters
    /**
     * @brief Get image width.
     * @param instance The backend instance.
     * @return Width.
     */
    virtual float getImageWidth(void* instance) const = 0;

    /**
     * @brief Get image height.
     * @param instance The backend instance.
     * @return Height.
     */
    virtual float getImageHeight(void* instance) const = 0;

    // OpenCL specific (optional - default implementations for CPU)
    /**
     * @brief Get available OpenCL devices.
     * @return List of device names. (Empty for CPU backend).
     */
    virtual std::vector<std::string> getOpenCLDevices() { return {}; }

    virtual ~IMoildevPlugin() = default;
};

// Plugin entry points - only these 2 functions need extern "C"
/**
 * @brief Factory function to create a plugin instance.
 * @return Pointer to the plugin interface.
 */
extern "C" MOILDEV_PLUGIN_API IMoildevPlugin* moildev_create_plugin();

/**
 * @brief Function to destroy the plugin instance.
 * @param plugin The plugin interface pointer to destroy.
 */
extern "C" MOILDEV_PLUGIN_API void moildev_destroy_plugin(IMoildevPlugin* plugin);

#endif // MOILDEV_PLUGIN_HPP
