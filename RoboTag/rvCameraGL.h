/*
    Copyright (c) 2010, Michael P. Thompson

    You may distribute under the terms of the GNU General Public License
    as specified in the README.txt file.

    $Id: rvCameraGL.h 28 2010-03-09 23:49:39Z mike $
*/

#ifndef _RV_CAMERA_H_
#define _RV_CAMERA_H_

#include "wx/wx.h"
#include "wx/glcanvas.h"
#include "cv.h"
#include "rvDSCamera.h"
#include "rvGrid.h"

// We only use a single texture to store the image.
#define RVCAMERA_TEXTURES_NUM       1
#define RVCAMERA_TEXTURE_ID         0

// Texture should be large enough to store full-size.
#define RVCAMERA_TEX_WIDTH          1024
#define RVCAMERA_TEX_HEIGHT         1024

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(wxEVT_SHOW_PIN_PROPERTIES, -2)
DECLARE_EVENT_TYPE(wxEVT_SHOW_FILTER_PROPERTIES, -3)
END_DECLARE_EVENT_TYPES()

class rvCamera: public wxGLCanvas
{
public:
    rvCamera(wxWindow *parent, wxWindowID id = wxID_ANY,
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize,
             long style = 0, const wxString& name = _T("rvCamera") );

    virtual ~rvCamera();

    void OnPaint(wxPaintEvent &event);
    void OnSize(wxSizeEvent &event);
    void OnEraseBackground(wxEraseEvent &event);
    void OnEnterWindow(wxMouseEvent &event);
    void OnImageReady(wxCommandEvent &event);
    void OnShowPinProperties(wxCommandEvent &event);
    void OnShowFilterProperties(wxCommandEvent &event);

    void Render();
    void InitGL();
    void InitCamera();

private:
    bool m_init;
    GLuint m_gllist;
    GLuint m_gltexid[RVCAMERA_TEXTURES_NUM];
    rvGrid *m_grid;

    rvDSCamera m_graphManager;

DECLARE_EVENT_TABLE()
};


#endif // _RV_CAMERA_H_