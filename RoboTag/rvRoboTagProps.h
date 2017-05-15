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

    $Id: rvRoboTagProps.h 28 2010-03-09 23:49:39Z mike $
*/

#ifndef _RV_ROBOTAGPROPS_H_
#define _RV_ROBOTAGPROPS_H_

#include "wx/wx.h"
#include "wx/propdlg.h"
#include "wx/bookctrl.h"
#include "wx/spinctrl.h"
#include "rvCamera.h"
#include "rvGrid.h"

BEGIN_DECLARE_EVENT_TYPES()
END_DECLARE_EVENT_TYPES()

class rvRoboTagProps: public wxPropertySheetDialog
{
    DECLARE_CLASS(rvRoboTagProps)
public:
    rvRoboTagProps(wxWindow* parent, rvCamera* camera);

    // Creates the book control.
    virtual wxBookCtrlBase* CreateBookCtrl();

    wxPanel* CreateGeneralPanel(wxWindow* parent);
    wxPanel* CreateAdaptiveThresholdPanel(wxWindow* parent);
    wxPanel* CreateCannyEdgePanel(wxWindow* parent);
    wxPanel* CreateSuzanEdgePanel(wxWindow* parent);
    wxPanel* CreateDisplayOptionsPanel(wxWindow* parent);

    void OnCameraView(wxCommandEvent& event);
    void OnEdgeMethod(wxCommandEvent& event);
    void OnEdgeDilation(wxSpinEvent& event);
    void OnGaussianBlur(wxSpinEvent& event);
    void OnAdaptiveMethod(wxCommandEvent& event);
    void OnAdaptiveBlockSize(wxSpinEvent& event);
    void OnAdaptiveSubtraction(wxSpinEvent& event);
    void OnDrawSetting(wxCommandEvent& event);

private:

    rvGrid* m_grid;
    rvCamera* m_camera;
    wxRadioBox* m_cameraView;
    wxRadioBox* m_edgeMethod;
    wxSpinCtrl* m_edgeDilation;
    wxSpinCtrl* m_gaussianBlur;
    wxRadioBox* m_adaptiveMethod;
    wxSpinCtrl* m_adaptiveBlockSize;
    wxSpinCtrl* m_adaptiveSubtraction;
    wxCheckBox* m_drawRawContours;
    wxCheckBox* m_drawPolygonContours;
    wxCheckBox* m_drawQuadContours;
    wxCheckBox* m_drawTagCorners;
    wxCheckBox* m_drawTagReferences;
    wxCheckBox* m_drawTagSamples;
    wxCheckBox* m_drawTagIdentifiers;
    wxCheckBox* m_drawCameraPosition;
    wxCheckBox* m_drawTagReprojection;
    wxCheckBox* m_drawObjectReprojection;
    wxCheckBox* m_drawCharacters;

    enum {
        ID_CAMERA_VIEW = 100,
        ID_EDGE_METHOD,
        ID_EDGE_DILATION,
        ID_GAUSSIAN_BLUR,

        ID_ADAPTIVE_METHOD,
        ID_ADAPTIVE_BLOCKSIZE,
        ID_ADAPTIVE_SUBTRACTION,

        ID_DRAW_RAW_CONTOURS,
        ID_DRAW_POLYGON_CONTOURS,
        ID_DRAW_QUAD_CONTOURS,
        ID_DRAW_TAG_REFERENCES,
        ID_DRAW_TAG_SAMPLES,
        ID_DRAW_TAG_CORNERS,
        ID_DRAW_TAG_INDENTIFIERS,
        ID_DRAW_CAMERA_POSITION,
        ID_DRAW_TAG_REPROJECTION,
        ID_DRAW_OBJECT_REPROJECTION,
        ID_DRAW_CHARACTERS
    };

    DECLARE_EVENT_TABLE()
};


#endif // _RV_ROBOTAGPROPS_H_