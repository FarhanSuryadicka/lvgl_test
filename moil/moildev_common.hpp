#ifndef MOILDEV_HPP
#define MOILDEV_HPP

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <optional>
#include <vector>

#ifdef USE_DOUBLE_PRECISION
using Real = float;
#else
using Real = double;
#endif // USE_DOUBLE_PRECISION

#define PI 3.1415926f
#define PCT_UNIT_WIDTH 1.27f
#define PCT_UNIT_HEIGHT 1.27f
#define FOCAL_LENGTH_FOR_ZOOM 250.0f
#define APPROX_FACT 8

struct CameraParams
{
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
};

struct PanoramaCar_precalc_alpha
{
    float vx;
    float vy;
    float az;
    float kxa_x;
    float kxa_y;
    float kxa_z;
    float k_a;
};

struct PanoramaCar_precalc_beta
{
    float ing_sin_beta;
    float ing_cos_beta;
};

struct AnyPointM_params
{
    float icx;
    float icy;
    float dcx;
    float dcy;
    float mAlphaOffset;
    float mBetaOffset;
    float widthCosB;
    float heightCosASinB;
    float flZoomSinASinB;
    float widthSinB;
    float heightCosACosB;
    float flZoomSinACosB;
    float heightSinA;
    float flZoomCosA;
};

struct AnyPointM2_params
{
    float icx;
    float icy;
    float dcx;
    float dcy;
    float thetaX;
    float thetaY;
    float widthCosB;
    float heightSinASinB;
    float flZoomCosASinB;
    float heightCosA;
    float flZoomSinA;
    float widthSinB;
    float heightSinACosB;
    float flZoomCosACosB;
};

struct PanoramaTube_params
{
    float icx;
    float icy;
};

struct PanoramaCar_params
{
    float icx;
    float icy;
    float ic_beta_pivot;
    float kx;
    float ky;
    float kz;
    bool flip_h;
    bool flip_v;
};

struct PanoramaRev_params
{
    float icx;
    float icy;
    float alpha_max;
    float beta_degree;
};

#define ALPHA2IH(alpha, params) \
    (((((((params).para0 * (alpha) + (params).para1) * (alpha) + \
          (params).para2) * (alpha) + (params).para3) * (alpha) + \
          (params).para4) * (alpha) + (params).para5) * (alpha))

#endif // MOILDEV_HPP