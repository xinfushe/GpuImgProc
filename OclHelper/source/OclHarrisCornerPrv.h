#pragma once

#include "OclCompact.h"
#include "OclDataBuffer.h"

namespace Ocl
{

class HarrisCornerPrv
{
public:
    HarrisCornerPrv(const cl::Context& ctxt);
    ~HarrisCornerPrv();

public:
    size_t process(const cl::CommandQueue& queue, const cl::Image2D& img, DataBuffer<cl_int2>& corners, float value, size_t& count);
    size_t process(const cl::CommandQueue& queue, const cl::ImageGL& inImage, DataBuffer<cl_int2>& corners, float value, size_t& count);

private:
    void init();
    void checkLocalGroupSizes();
    void createIntImages(const cl::Image& inpImg);
    size_t eigen(const cl::CommandQueue& queue, const cl::Image& inpImg, cl::Image& outImg);
    size_t gradient(const cl::CommandQueue& queue, const cl::Image& inpImg, cl::Image& outImg);
    size_t suppress(const cl::CommandQueue& queue, const cl::Image& inpImg, cl::Image& outImg, float value);

private:
    size_t mWidth;
    size_t mHeight;
    size_t mLocSizeX;
    size_t mLocSizeY;
    const cl::Context& mContext;

    cl::Program mPgm;

    cl::Kernel mGradKernel;
    cl::Kernel mEigenKernel;
    cl::Kernel mCornerKernel;

    std::unique_ptr<cl::Image2D> mIxIyImg;
    std::unique_ptr<cl::Image2D> mEigenImg;
    std::unique_ptr<cl::Image2D> mCornerImg;

    Ocl::Compact mCompact;

    static const char sSource[];
};

};

