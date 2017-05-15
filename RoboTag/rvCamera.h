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

    $Id: rvCamera.h 28 2010-03-09 23:49:39Z mike $
*/

#ifndef _RV_CAMERA_H_
#define _RV_CAMERA_H_

#include "wx/wx.h"
#include "cv.h"
#include "highgui.h"
#include "rvDSCamera.h"
#include "rvGrid.h"

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(wxEVT_SHOW_PIN_PROPERTIES, -2)
DECLARE_EVENT_TYPE(wxEVT_SHOW_FILTER_PROPERTIES, -3)
END_DECLARE_EVENT_TYPES()

class rvCamera: public wxWindow
{
public:
    rvCamera(wxWindow *parent, wxWindowID id = wxID_ANY,
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize,
             long style = 0, const wxString& name = _T("rvCamera") );

    virtual ~rvCamera();

    rvGrid* GetGrid();

    void Draw(wxDC& dc);

    void OnPaint(wxPaintEvent &event);
    void OnSize(wxSizeEvent &event);
    void OnEraseBackground(wxEraseEvent &event);
    void OnEnterWindow(wxMouseEvent &event);
    void OnImageReady(wxCommandEvent &event);
    void OnShowPinProperties(wxCommandEvent &event);
    void OnShowFilterProperties(wxCommandEvent &event);

private:
    int m_width;
    int m_height;
    rvGrid *m_grid;
    rvDSCamera m_graphManager;
    IplImage *m_flippedImage;

DECLARE_EVENT_TABLE()
};


#endif // _RV_CAMERA_H_