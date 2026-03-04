#ifndef MOILDEV_HPP
#define MOILDEV_HPP

#include <memory>
#include <string>
#include <vector>

struct IMoildevPlugin;

namespace moildev
{
    /**
     * @brief Backend type enumeration
     *
     * Specifies which computation backend to use for image processing.
     */
    enum class Backend
    {
        AUTO,   /**< Automatically select the best available backend (OpenCL if available, else CPU). */
        CPU,    /**< Force usage of the CPU backend implementation. */
        OCL     /**< Force usage of the OpenCL backend implementation. */
    };

    /**
     * @brief Abstract interface for backend implementations.
     *
     * This interface defines the contract that all Moildev backends must follow.
     * It allows the main Moildev class to switch between different implementations
     * (like CPU or OpenCL) transparently.
     */
    class IMoildevBackend
    {
    public:
        /**
         * @brief Virtual destructor.
         */
        virtual ~IMoildevBackend() = default;

        /**
         * @brief Generate AnyPoint map (Method 1).
         *
         * @param mapX Pointer to the X-coordinate map buffer.
         * @param mapY Pointer to the Y-coordinate map buffer.
         * @param alpha The alpha angle (tilt) for the view.
         * @param beta The beta angle (pan) for the view.
         * @param zoom The zoom factor.
         * @return Status code (0 for success).
         */
        virtual int AnyPointM(float *mapX, float *mapY, float alpha, float beta, float zoom) = 0;

        /**
         * @brief Generate AnyPoint map (Method 2).
         *
         * @param mapX Pointer to the X-coordinate map buffer.
         * @param mapY Pointer to the Y-coordinate map buffer.
         * @param alpha The alpha angle (tilt) for the view.
         * @param beta The beta angle (pan) for the view.
         * @param zoom The zoom factor.
         * @return Status code (0 for success).
         */
        virtual int AnyPointM2(float *mapX, float *mapY, float alpha, float beta, float zoom) = 0;

        /**
         * @brief Generate Panorama Tube map.
         *
         * @param mapX Pointer to the X-coordinate map buffer.
         * @param mapY Pointer to the Y-coordinate map buffer.
         * @param alpha_min The minimum alpha angle.
         * @param alpha_max The maximum alpha angle.
         * @return Status code (0 for success).
         */
        virtual int PanoramaTube(float *mapX, float *mapY, float alpha_min, float alpha_max) = 0;

        /**
         * @brief Generate Panorama Car map.
         *
         * @param mapX Pointer to the X-coordinate map buffer.
         * @param mapY Pointer to the Y-coordinate map buffer.
         * @param my_p_alpha_max_vendor Maximum alpha angle specific to vendor.
         * @param p_iC_alpha_degree Alpha degree parameter.
         * @param p_iC_beta_degree Beta degree parameter.
         * @param flip_h Whether to flip horizontally.
         * @param flip_v Whether to flip vertically.
         * @return Status code (0 for success).
         */
        virtual int PanoramaCar(float *mapX, float *mapY, float my_p_alpha_max_vendor,
                               float p_iC_alpha_degree, float p_iC_beta_degree,
                               bool flip_h, bool flip_v) = 0;

        /**
         * @brief Generate Reverse Panorama map.
         *
         * @param mapX Pointer to the X-coordinate map buffer.
         * @param mapY Pointer to the Y-coordinate map buffer.
         * @param alpha_max The maximum alpha angle.
         * @param alpha_degree The alpha degree parameter.
         * @param beta_degree The beta degree parameter.
         * @return Status code (0 for success).
         */
        virtual int PanoramaRev(float *mapX, float *mapY, float alpha_max,
                               float alpha_degree, float beta_degree) = 0;

        /**
         * @brief Convert panorama coordinates to original image coordinates.
         *
         * @param panorama_x X-coordinate in the panorama image.
         * @param panorama_y Y-coordinate in the panorama image.
         * @param alpha_max The maximum alpha angle used for the panorama.
         * @return A pair containing the (x, y) coordinates in the original image.
         */
        virtual std::pair<int, int> PanoramaToOriginal(float panorama_x, float panorama_y, float alpha_max) = 0;

        /**
         * @brief Get the processed image width.
         * @return Width as a float.
         */
        virtual float getImageWidth() const = 0;

        /**
         * @brief Get the processed image height.
         * @return Height as a float.
         */
        virtual float getImageHeight() const = 0;
    };

    /**
     * @brief Unified Moildev wrapper class with dynamic backend selection.
     *
     * This class acts as the main entry point for the Moildev library. It abstracts details
     * of the underlying implementation (CPU or OpenCL) and provides a consistent API.
     * It handles backend loading, initialization, and fallback mechanisms.
     */
    class Moildev
    {
    public:
        /**
         * @brief Constructor for Moildev with automatic backend selection.
         *
         * Initializes the Moildev instance with camera parameters and backend preference.
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
         * @param backend Preferred backend (AUTO, CPU, or OCL). Defaults to AUTO.
         * @param openCLDeviceID ID of the OpenCL device to use (if OCL backend is selected). Defaults to 0.
         */
        Moildev(float cameraSensorWidth, float cameraSensorHeight,
                float icx, float icy, float ratio,
                float imageWidth, float imageHeight,
                float calibrationRatio,
                float para0, float para1, float para2,
                float para3, float para4, float para5,
                Backend backend = Backend::AUTO,
                int openCLDeviceID = 0);

        /**
         * @brief Destructor.
         *
         * Cleans up resources and unloads backend plugins.
         */
        ~Moildev();

        /**
         * @brief Deleted copy constructor.
         * Moildev cannot be copied due to unique ownership of backend resources.
         */
        Moildev(const Moildev&) = delete;

        /**
         * @brief Deleted assignment operator.
         */
        Moildev& operator=(const Moildev&) = delete;

        /**
         * @brief Move constructor.
         * @param other The instance to move from.
         */
        Moildev(Moildev&& other) noexcept;

        /**
         * @brief Move assignment operator.
         * @param other The instance to move from.
         * @return Reference to this instance.
         */
        Moildev& operator=(Moildev&& other) noexcept;

        /**
         * @brief Get the currently active backend type.
         * @return The Backend enum value of the current backend.
         */
        Backend getBackend() const { return activeBackend; }

        /**
         * @brief Get the name of the current backend.
         * @return A string representation of the backend name ("CPU", "OpenCL", or "Unknown").
         */
        std::string getBackendName() const;

        /**
         * @brief Check if OpenCL backend library is available.
         * @return true if OpenCL library can be loaded, false otherwise.
         */
        static bool isOpenCLAvailable();

        /**
         * @brief Check if CPU backend library is available.
         * @return true if CPU library can be loaded, false otherwise.
         */
        static bool isCPUAvailable();

        /**
         * @brief List available OpenCL devices.
         * @return A vector of strings containing the names of available OpenCL devices.
         */
        static std::vector<std::string> getOpenCLDevices();

        /**
         * @brief Generate AnyPoint map (Method 1).
         *
         * Delegates to the active backend implementation.
         *
         * @param mapX Pointer to the X-coordinate map buffer.
         * @param mapY Pointer to the Y-coordinate map buffer.
         * @param alpha The alpha angle (tilt).
         * @param beta The beta angle (pan).
         * @param zoom The zoom factor.
         * @return Status code (0 for success).
         */
        int AnyPointM(float *mapX, float *mapY, float alpha, float beta, float zoom);

        /**
         * @brief Generate AnyPoint map (Method 2).
         *
         * Delegates to the active backend implementation.
         *
         * @param mapX Pointer to the X-coordinate map buffer.
         * @param mapY Pointer to the Y-coordinate map buffer.
         * @param alpha The alpha angle (tilt).
         * @param beta The beta angle (pan).
         * @param zoom The zoom factor.
         * @return Status code (0 for success).
         */
        int AnyPointM2(float *mapX, float *mapY, float alpha, float beta, float zoom);

        /**
         * @brief Generate Panorama Tube map.
         *
         * @param mapX Pointer to the X-coordinate map buffer.
         * @param mapY Pointer to the Y-coordinate map buffer.
         * @param alpha_min The minimum alpha angle.
         * @param alpha_max The maximum alpha angle.
         * @return Status code (0 for success).
         */
        int PanoramaTube(float *mapX, float *mapY, float alpha_min, float alpha_max);

        /**
         * @brief Generate Panorama Car map.
         *
         * @param mapX Pointer to the X-coordinate map buffer.
         * @param mapY Pointer to the Y-coordinate map buffer.
         * @param my_p_alpha_max_vendor Maximum alpha angle specific to vendor.
         * @param p_iC_alpha_degree Alpha degree parameter.
         * @param p_iC_beta_degree Beta degree parameter.
         * @param flip_h Whether to flip horizontally.
         * @param flip_v Whether to flip vertically.
         * @return Status code (0 for success).
         */
        int PanoramaCar(float *mapX, float *mapY, float my_p_alpha_max_vendor,
                       float p_iC_alpha_degree, float p_iC_beta_degree,
                       bool flip_h = false, bool flip_v = false);

        /**
         * @brief Generate Reverse Panorama map.
         *
         * @param mapX Pointer to the X-coordinate map buffer.
         * @param mapY Pointer to the Y-coordinate map buffer.
         * @param alpha_max The maximum alpha angle.
         * @param alpha_degree The alpha degree parameter.
         * @param beta_degree The beta degree parameter.
         * @return Status code (0 for success).
         */
        int PanoramaRev(float *mapX, float *mapY, float alpha_max,
                       float alpha_degree, float beta_degree);

        /**
         * @brief Convert panorama coordinates back to original image coordinates.
         *
         * @param panorama_x X-coordinate in the panorama image.
         * @param panorama_y Y-coordinate in the panorama image.
         * @param alpha_max The maximum alpha angle used.
         * @return A pair of (x, y) coordinates in the original image.
         */
        std::pair<int, int> PanoramaToOriginal(float panorama_x, float panorama_y, float alpha_max);

        /**
         * @brief Get the configured image width.
         * @return Width in pixels.
         */
        float getImageWidth() const;

        /**
         * @brief Get the configured image height.
         * @return Height in pixels.
         */
        float getImageHeight() const;

    private:
        std::unique_ptr<IMoildevBackend> backend;
        Backend activeBackend;

        void* oclLibHandle; 
        void* cpuLibHandle; 
        IMoildevPlugin* oclPlugin;
        IMoildevPlugin* cpuPlugin;

        // Stored parameters for fallback re-initialization
        struct InitParams {
            float cameraSensorWidth;
            float cameraSensorHeight;
            float icx;
            float icy;
            float ratio;
            float imageWidth;
            float imageHeight;
            float calibrationRatio;
            float para0;
            float para1;
            float para2;
            float para3;
            float para4;
            float para5;
        } m_params;
        int m_openCLDeviceID;

        void switchToCPU();

        void initializeBackend(float cameraSensorWidth, float cameraSensorHeight,
                              float icx, float icy, float ratio,
                              float imageWidth, float imageHeight,
                              float calibrationRatio,
                              float para0, float para1, float para2,
                              float para3, float para4, float para5,
                              Backend backendChoice,
                              int openCLDeviceID);

        std::unique_ptr<IMoildevBackend> createCPUBackend(
            float cameraSensorWidth, float cameraSensorHeight,
            float icx, float icy, float ratio,
            float imageWidth, float imageHeight,
            float calibrationRatio,
            float para0, float para1, float para2,
            float para3, float para4, float para5);

        std::unique_ptr<IMoildevBackend> createOpenCLBackend(
            float cameraSensorWidth, float cameraSensorHeight,
            float icx, float icy, float ratio,
            float imageWidth, float imageHeight,
            float calibrationRatio,
            float para0, float para1, float para2,
            float para3, float para4, float para5,
            int openCLDeviceID);
    };

} // namespace moildev

#endif // MOILDEV_HPP
