/*
    Copyright (C) 2010, Michael P. Thompson

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License Version 2 as 
    specified in the README.txt file or as published by the Free Software 
    Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

    $Id: rvCamera.cpp 28 2010-03-09 23:49:39Z mike $
*/

#include "wx/wx.h"
#include "rvCamera.h"
#include "rvRoboTagApp.h"

DEFINE_EVENT_TYPE(wxEVT_SHOW_PIN_PROPERTIES)
DEFINE_EVENT_TYPE(wxEVT_SHOW_FILTER_PROPERTIES)

BEGIN_EVENT_TABLE(rvCamera, wxWindow)
    EVT_SIZE(rvCamera::OnSize)
    EVT_PAINT(rvCamera::OnPaint)
    EVT_ERASE_BACKGROUND(rvCamera::OnEraseBackground)
    EVT_COMMAND(wxID_ANY, wxEVT_CAMERA_IMAGE_READY, rvCamera::OnImageReady)
    EVT_COMMAND(wxID_ANY, wxEVT_SHOW_PIN_PROPERTIES, rvCamera::OnShowPinProperties)
    EVT_COMMAND(wxID_ANY, wxEVT_SHOW_FILTER_PROPERTIES, rvCamera::OnShowFilterProperties)
END_EVENT_TABLE()

rvCamera::rvCamera(wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
    : wxWindow(parent, id, pos, size, style | wxFULL_REPAINT_ON_RESIZE , name),
      m_graphManager()
{
    // Get the canvas width/height
    m_width = size.GetWidth( );
    m_height = size.GetHeight( );

    // Start the camera.
    m_graphManager.BuildGraph(640, 480, 30.0);
    m_graphManager.EnableMemoryBuffer(3);
    m_graphManager.SetImageHandler(this);
    m_graphManager.Run();

    // Create a new grid object.
    m_grid = rvGrid_New(cvSize(640, 480), IPL_ORIGIN_BL);

    // Set the camera matrix and distortion coeffs.
    rvGrid_LoadIntrinsics(m_grid, wxGetApp().GetAppDir() << "\\Intrinsics.yml");

    // Create a flipped image buffer.  This buffer is needed the camera  and OpenCV
    // deal with the image with a different orgin and RGB order than wxWidgets.
    m_flippedImage = cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 3);
}

rvCamera::~rvCamera()
{
    // Release the flipped image buffer.
    cvReleaseImage(&m_flippedImage);

    // Free the grid object.
    rvGrid_Free(m_grid);

    // Release the graph manager resources.
    m_graphManager.Stop();
    m_graphManager.DisableMemoryBuffer();
    m_graphManager.ReleaseGraph();
}

rvGrid* rvCamera::GetGrid()
{
    return m_grid;
}

void rvCamera::Draw(wxDC& dc)
{
    // Check if dc available.
    if (dc.IsOk())
    {
        int x,y,w,h;

        // Get the clipping box.
        dc.GetClippingBox(&x, &y, &w, &h);

        // Check out the image.
        IplImage image;
        if (m_graphManager.CheckoutIplImage(&image))
        {
            int step;
            CvSize roiSize;
            unsigned char *rawData;

            // Process the image to determine the position.
            rvGrid_ProcessImage(m_grid, &image);

            // Flip the image and swap the red and blue channels.
            cvConvertImage(&image, m_flippedImage, CV_CVTIMG_FLIP | CV_CVTIMG_SWAP_RB);

            // Get raw data from the image.
            cvGetRawData(m_flippedImage, &rawData, &step, &roiSize);

            // Convert data from raw image data to a native wxImage.
            wxImage nativeImage = wxImage(640, 480, rawData, TRUE);

            // Do we need to scale the image?
            if (roiSize.width != m_width || roiSize.height != m_height)
            {
                // Rescale the image.
                nativeImage.Rescale(m_width, m_height, wxIMAGE_QUALITY_NORMAL);
            }

            // Convert to image to bitmap so we can draw it to a window.
            wxBitmap bitmap = wxBitmap(nativeImage);

            // Draw the bitmap.
            dc.DrawBitmap(bitmap, x, y);

            // We are finished with the image.
            m_graphManager.CheckinIplImage(&image);
        }
    }
}


void rvCamera::OnPaint(wxPaintEvent& WXUNUSED(event))
{
    wxPaintDC dc(this);
    Draw(dc);
}

void rvCamera::OnSize(wxSizeEvent &event)
{
    // Update the window widght and height.
    m_width = event.GetSize().GetWidth();
    m_height = event.GetSize().GetHeight();
}

void rvCamera::OnEraseBackground(wxEraseEvent &WXUNUSED(event))
{
    // Do nothing, to avoid flashing.  We probably only want to
    // avoid erasing the background when we are capturing data.
}

void rvCamera::OnImageReady(wxCommandEvent &WXUNUSED(event))
{
    // Force an update of the window.
    Refresh(false, NULL);
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

