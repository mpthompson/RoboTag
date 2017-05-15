/*
    Copyright (c) 2010, Michael P. Thompson

    You may distribute under the terms of the GNU General Public License
    as specified in the README.txt file.

    $Id: rvCameraGL.cpp 28 2010-03-09 23:49:39Z mike $
*/

#include "wx/wx.h"
#include "rvCamera.h"
#include "rvRoboTagApp.h"

DEFINE_EVENT_TYPE(wxEVT_SHOW_PIN_PROPERTIES)
DEFINE_EVENT_TYPE(wxEVT_SHOW_FILTER_PROPERTIES)

BEGIN_EVENT_TABLE(rvCamera, wxGLCanvas)
    EVT_SIZE(rvCamera::OnSize)
    EVT_PAINT(rvCamera::OnPaint)
    EVT_ERASE_BACKGROUND(rvCamera::OnEraseBackground)
    EVT_ENTER_WINDOW(rvCamera::OnEnterWindow)
    EVT_COMMAND(wxID_ANY, wxEVT_CAMERA_IMAGE_READY, rvCamera::OnImageReady)
    EVT_COMMAND(wxID_ANY, wxEVT_SHOW_PIN_PROPERTIES, rvCamera::OnShowPinProperties)
    EVT_COMMAND(wxID_ANY, wxEVT_SHOW_FILTER_PROPERTIES, rvCamera::OnShowFilterProperties)
END_EVENT_TABLE()

rvCamera::rvCamera(wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
    : wxGLCanvas(parent, (wxGLCanvas*) NULL, id, pos, size, style | wxFULL_REPAINT_ON_RESIZE , name ),
      m_graphManager()
{
    m_init = false;
    m_gllist = 0;

    // Create a new grid object.
    m_grid = rvGrid_New(cvSize(640, 480), IPL_ORIGIN_BL);

    // Initialize what grid items to draw.
    rvGrid_DrawTagCorners(m_grid, 1);
    rvGrid_DrawTagSamples(m_grid, 1);
    rvGrid_DrawTagReferences(m_grid, 1);
    rvGrid_DrawTagIdentifiers(m_grid, 1);
    rvGrid_DrawCameraPosition(m_grid, 1);
    rvGrid_DrawTagReprojection(m_grid, 1);
    rvGrid_DrawObjectReprojection(m_grid, 1);

    // Set the camera matrix and distortion coeffs.
    rvGrid_LoadIntrinsics(m_grid, wxGetApp().GetAppDir() << "\\Intrinsics.yml");
}

rvCamera::~rvCamera()
{
    // Free the grid object.
    rvGrid_Free(m_grid);

    // Delete GL resources.
    if (m_gllist) glDeleteLists(m_gllist, 1);
    glDeleteTextures(RVCAMERA_TEXTURES_NUM, m_gltexid);

    // Release the graph manager resources.
    m_graphManager.Stop();
    m_graphManager.DisableMemoryBuffer();
    m_graphManager.ReleaseGraph();
}

void rvCamera::Render()
{
    wxPaintDC dc(this);

    // Obtain the context that is associated with this canvas.
    if (!GetContext()) return;

    // Have subsequent OpenGL calls modify the OpenGL state of the implicit rendering context.
    SetCurrent();

    // Init OpenGL once, but after SetCurrent
    if (!m_init)
    {
        InitGL();
        InitCamera();
        m_init = true;
    }

#if 0
    // Check out the image.
    IplImage image;
    if (m_graphManager.CheckoutIplImage(&image))
    {
        // Process the image to determine the position.
        rvGrid_ProcessImage(m_grid, &image);

        // Bind the texture name to the appropriate texture target.
        glBindTexture(GL_TEXTURE_2D, m_gltexid[RVCAMERA_TEXTURE_ID]);

        // Specify the pixel image as two-dimensional texture subimage.
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 640, 480, GL_BGR_EXT, GL_UNSIGNED_BYTE, (void*) image.imageData);

        // We are finished with the image.
        m_graphManager.CheckinIplImage(&image);
    }
#endif

#if 1
    // Check out the image.
    IplImage image;
    if (m_graphManager.CheckoutIplImage(&image))
    {
        // Initialize the output image.
        IplImage *outputImage = cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 1);
        outputImage->origin = IPL_ORIGIN_BL;

        // Blur the camera image into the output image.
        // cvSmooth(&image, outputImage, CV_BLUR, 15, 0, 0.0, 0.0);
        cvCvtColor(&image, outputImage, CV_BGR2GRAY);

        // Bind the texture name to the appropriate texture target.
        glBindTexture(GL_TEXTURE_2D, m_gltexid[RVCAMERA_TEXTURE_ID]);

        // Specify the pixel image as two-dimensional texture subimage.
        // glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 640, 480, GL_BGR_EXT, GL_UNSIGNED_BYTE, (void*) outputImage->imageData);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 640, 480, GL_LUMINANCE, GL_UNSIGNED_BYTE, (void*) outputImage->imageData);

        // Release the output image.
        cvReleaseImage(&outputImage);

        // We are finished with the image.
        m_graphManager.CheckinIplImage(&image);
    }
#endif

    // Clear the screen and the depth buffer.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    float u_rt = (float) 640 / RVCAMERA_TEX_WIDTH;
    float v_rt = (float) 480 / RVCAMERA_TEX_HEIGHT;

    glDisable(GL_DEPTH_TEST);
    glBindTexture(GL_TEXTURE_2D, m_gltexid[RVCAMERA_TEXTURE_ID]);
    glBegin(GL_QUADS);
        // video backplate
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  0.0f);
        glTexCoord2f(u_rt, 0.0f); glVertex3f( 1.0f, -1.0f,  0.0f);
        glTexCoord2f(u_rt, v_rt); glVertex3f( 1.0f,  1.0f,  0.0f);
        glTexCoord2f(0.0f, v_rt); glVertex3f(-1.0f,  1.0f,  0.0f);
    glEnd();
    glEnable(GL_DEPTH_TEST);

    glPopMatrix();

    // Swap The Buffers To Become Our Rendering Visible
    SwapBuffers();
}

void rvCamera::OnPaint(wxPaintEvent& WXUNUSED(event))
{
    Render();
}

void rvCamera::OnSize(wxSizeEvent &event)
{
    int w, h;

    // This is necessary to update the context on some platforms.
    wxGLCanvas::OnSize(event);

    // Obtain the context that is associated with this canvas.
    if (!GetContext()) return;

    // Have subsequent OpenGL calls modify the OpenGL state of the implicit rendering context.
    SetCurrent();

    // Get the client window size.
    GetClientSize(&w, &h);

    // Set the new viewport size.
    glViewport(0, 0, w, h);
}

void rvCamera::OnEraseBackground(wxEraseEvent &WXUNUSED(event))
{
    // Do nothing, to avoid flashing.
}

void rvCamera::OnEnterWindow(wxMouseEvent& WXUNUSED(event))
{
    SetFocus();
}

void rvCamera::OnImageReady(wxCommandEvent &WXUNUSED(event))
{
    // Force an update of the window.
    Refresh(false);
}

void rvCamera::OnShowPinProperties(wxCommandEvent &WXUNUSED(event))
{
    // Show the pin properties.
    m_graphManager.ShowPinProperties((HWND) GetHWND());
}

void rvCamera::OnShowFilterProperties(wxCommandEvent &WXUNUSED(event))
{
    // Show the filter properties.
    m_graphManager.ShowFilterProperties((HWND) GetHWND());
}

void rvCamera::InitGL()
{
    long imageSize;
    GLubyte *imageData;

    // Have subsequent OpenGL calls modify the OpenGL state of the implicit rendering context.
    SetCurrent();

    // Enable smoot shading.
    glShadeModel(GL_SMOOTH);

    // Black background.
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);

    // Depth buffer setup.
    glClearDepth(1.0f);

    // Enable depth testing and the type of depth testing.
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Set up the textures.
    imageSize = 4 * sizeof(GLubyte) * RVCAMERA_TEX_WIDTH * RVCAMERA_TEX_HEIGHT;

    // Allocate and initialize the image buffer.
    imageData = (GLubyte*) malloc(imageSize);
    FillMemory(imageData, imageSize, 0x7f);

    // Generate the texture ID list.
    glGenTextures(RVCAMERA_TEXTURES_NUM, m_gltexid);

    // Typical texture generation using data from an image.
    glBindTexture(GL_TEXTURE_2D, m_gltexid[RVCAMERA_TEXTURE_ID]);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, RVCAMERA_TEX_WIDTH, RVCAMERA_TEX_HEIGHT, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, imageData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Finished with the image buffer so free it.
    free(imageData);

    // Enable texture mapping.
    glEnable(GL_TEXTURE_2D);

    // Pixel Storage Mode To Byte Alignment
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Cull Polygons
    glEnable(GL_CULL_FACE);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

void rvCamera::InitCamera()
{
    m_graphManager.BuildGraph(640, 480, 30.0);
    m_graphManager.EnableMemoryBuffer(3);
    m_graphManager.SetImageHandler(this);
    m_graphManager.Run();
}



