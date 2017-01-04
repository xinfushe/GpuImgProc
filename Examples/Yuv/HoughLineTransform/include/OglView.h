#pragma once

#include "OglImage.h"
#include "OglImagePainter.h"
#include "OclCannyEdge.h"
#include "OclHoughLines.h"
#include "OglHoughLinePainter.h"

class OglView
{
public:
    OglView(GLsizei w, GLsizei h, cl::Context& ctxt, cl::CommandQueue& queue);
    ~OglView();

public:
    void draw(uint8_t* pData);
    void resize(GLsizei w, GLsizei h);

private:
    float mMinThresh;
    float mMaxThresh;

    cl::Context& mCtxtCL;
    cl::CommandQueue& mQueueCL;

    Ocl::CannyEdge mCanny;
    Ocl::HoughLines mHoughLines;

    Ogl::Yuv420Image mYuvImg;
    Ogl::Image<GL_RED> mEdgeImg;

    Ocl::DataBuffer<Ocl::HoughData> mHoughData;

    Ogl::HoughLinePainter mHoughLinePainter;
    Ogl::ImagePainter< Ogl::Yuv420Shader, Ogl::Yuv420Image > mYuvPainter;
    Ogl::ImagePainter< Ogl::LumaShader, Ogl::Image<GL_RED> > mLumaPainter;
};
