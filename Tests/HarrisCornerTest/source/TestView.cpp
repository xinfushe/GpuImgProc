
// OptFlowView.cpp : implementation of the OptFlowView class
//

#include "stdafx.h"
#include "TestApp.h"
#include "TestView.h"

#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32.lib")

// OptFlowView
TestView::TestView()
{
}

TestView::~TestView()
{
}

BEGIN_MESSAGE_MAP(TestView, CWnd)
    ON_WM_PAINT()
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_DESTROY()
    ON_WM_TIMER()
    ON_WM_KEYDOWN()
END_MESSAGE_MAP()

// TestView message handlers
BOOL TestView::PreCreateWindow(CREATESTRUCT& cs)
{
    if (!CWnd::PreCreateWindow(cs))
        return FALSE;

    cs.dwExStyle |= WS_EX_CLIENTEDGE;
    cs.style &= ~WS_BORDER;
    cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
        ::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);

    return TRUE;
}

void TestView::OnPaint()
{
    try
    {
        CPaintDC dc(this); // device context for painting
        // TODO: Add your message handler code here
        Ogl::UseWinGlContext use(*mGlCtxt);
        mCamera.read(mFrame);
        mGlView->draw(mFrame.data);
    }

    catch (cl::Error error)
    {
        const char* str = error.what();
        MessageBox(_T("OpenCL Error"));
        exit(0);
    }
    // Do not call CWnd::OnPaint() for painting messages
}

void TestView::createOpenGLContext()
{
    PIXELFORMATDESCRIPTOR pfd;

    memset(&pfd, 0, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 16;

    int pixFmt = ChoosePixelFormat(GetDC()->m_hDC, &pfd);
    SetPixelFormat(GetDC()->m_hDC, pixFmt, &pfd);

    mGlCtxt.reset(new Ogl::WinGlContext(this));
}

void TestView::initGL()
{
    createOpenGLContext();
    Ogl::UseWinGlContext use(*mGlCtxt);

    glewInit();

    try
    {
        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);
        if (platforms.size() == 0)
        {
            AfxMessageBox(_T("No support for OpenCL"));
            exit(0);
        }

        cl_context_properties props[] = 
            { CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
              CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
              CL_CONTEXT_PLATFORM, (cl_context_properties)(platforms[0])(), 0 };

        mClContext.reset(new cl::Context(CL_DEVICE_TYPE_GPU, props));
        std::vector<cl::Device> devices = mClContext->getInfo<CL_CONTEXT_DEVICES>();
        mClQueue.reset(new cl::CommandQueue(*mClContext, devices[0], CL_QUEUE_PROFILING_ENABLE));
    }

    catch (cl::Error error)
    {
        AfxMessageBox(_T("No support for OpenCL"));
        exit(0);
    }

    mCamera.open(0);
    //mCamera.set(cv::CAP_PROP_FRAME_WIDTH, 1920);
    //mCamera.set(cv::CAP_PROP_FRAME_HEIGHT, 1080);
    if (!mCamera.isOpened())
    {
        exit(0);
    }
    mCamera.read(mFrame);
    mGlView.reset(new GlView(mFrame.cols, mFrame.rows, *mClContext, *mClQueue));
}

int TestView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    // TODO:  Add your specialized creation code here
    initGL();

    SetTimer(1, 25, 0);
    return 0;
}


void TestView::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);

    // TODO: Add your message handler code here
    Ogl::UseWinGlContext use(*mGlCtxt);
    mGlView->resize(cx, cy);
}


void TestView::OnDestroy()
{
    CWnd::OnDestroy();

    // TODO: Add your message handler code here
    {
        Ogl::UseWinGlContext use(*mGlCtxt);
        mGlView.reset();
        mClQueue.reset();
        mClContext.reset();
    }
    mGlCtxt.reset();
    mCamera.release();
}


void TestView::OnTimer(UINT_PTR nIDEvent)
{
    // TODO: Add your message handler code here and/or call default
    SendMessage(WM_PAINT);
    CWnd::OnTimer(nIDEvent);
}


void TestView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}
