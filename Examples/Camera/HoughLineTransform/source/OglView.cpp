#include "OglView.h"
#include "OglImageFormat.h"

#define THRESHOLD_CHANGE 0.001f

OglView::OglView(GLsizei w, GLsizei h, cl::Context& ctxt, cl::CommandQueue& queue)
    :mSize((size_t)(w/4)),
     mMinThresh((float)(20.0/256.0)),
     mMaxThresh((float)(70.0/256.0)),
     mCtxtCL(ctxt),
     mQueueCL(queue),
     mBgrImg(w, h, GL_RGB, GL_UNSIGNED_BYTE),
     mGrayImg(w, h, GL_R32F, GL_FLOAT),
     mEdgeImg(w, h, GL_R32F, GL_FLOAT),
     mCanny(mCtxtCL),
     mHoughLines(mCtxtCL),
     mHoughData(mCtxtCL, CL_MEM_READ_WRITE, 1000),
     mHoughLinePainter(mCtxtCL, 1000)
{
}

OglView::~OglView()
{
}

void OglView::draw(uint8_t* pData)
{
    mBgrImg.load(pData);
    Ogl::ImageFormat::convert(mGrayImg, mBgrImg);

    cl::ImageGL inpImgGL(mCtxtCL, CL_MEM_READ_ONLY, GL_TEXTURE_2D, 0, mGrayImg.texture());
    cl::ImageGL outImgGL(mCtxtCL, CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, mEdgeImg.texture());

    size_t time = mCanny.process(mQueueCL, inpImgGL, outImgGL, mMinThresh, mMaxThresh);    
    
    size_t lines = 0;
    time += mHoughLines.process(mQueueCL, outImgGL, mSize, mHoughData, lines);

    mRgbaPainter.draw(mBgrImg);
    //mGrayPainter.draw(mEdgeImg);
    mHoughLinePainter.draw(mQueueCL, mHoughData, lines, mEdgeImg.width(), mEdgeImg.height());

    //Ogl::IGeometry::Rect vp = { mBgrImg.width()/2, 0, mBgrImg.width()/2, mBgrImg.height()/2 };
    //mGrayPainter.draw(vp, mEdgeImg);
}

void OglView::resize(GLsizei w, GLsizei h)
{
    glViewport(0, 0, w, h);
}

void OglView::minThresholdUp()
{
    float thresh = mMinThresh + THRESHOLD_CHANGE;
    if (thresh < mMaxThresh)
    {
        mMinThresh = thresh;
    }
}

void OglView::minThresholdDown()
{
    float thresh = mMinThresh - THRESHOLD_CHANGE;
    if (thresh > 0.0)
    {
        mMinThresh = thresh;
    }
}

void OglView::maxThresholdUp()
{
    float thresh = mMaxThresh + THRESHOLD_CHANGE;
    if (thresh < 1.0)
    {
        mMaxThresh = thresh;
    }
}

void OglView::maxThresholdDown()
{
    float thresh = mMaxThresh - THRESHOLD_CHANGE;
    if (thresh > mMinThresh)
    {
        mMaxThresh = thresh;
    }
}

void OglView::minLineSizeUp()
{
    mSize += 10;
}

void OglView::minLineSizeDown()
{
    if (mSize > 20)
    {
        mSize -= 10;
    }
}
