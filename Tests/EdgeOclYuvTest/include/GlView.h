#pragma once

#include "OglImage.h"
#include "OglImagePainter.h"

#include <CL/cl.hpp>
#include "CannyEdge.h"

class GlView
{
public:
    GlView(GLsizei w, GLsizei h, cl::Context& ctxt, cl::CommandQueue& queue);
    ~GlView();

public:
    void draw(uint8_t* pData);
    void resize(GLsizei w, GLsizei h);

private:
    cl::Context& mCtxtCL;
    cl::CommandQueue& mQueueCL;

    Ocl::CannyEdge mCanny;

    Ogl::Yuv420Image mYuvImg;
    Ogl::Image<GL_RED> mEdgeImg;

    float minThresh;
    float maxThresh;

    Ogl::ImagePainter< Ogl::Yuv420Shader, Ogl::Yuv420Image > mYuvPainter;
    Ogl::ImagePainter< Ogl::LumaShader, Ogl::Image<GL_RED> > mLumaPainter;
};
