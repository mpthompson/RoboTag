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

    $Id: rvRoboTagProps.cpp 28 2010-03-09 23:49:39Z mike $
*/

#include "wx/wx.h"
#include "rvRoboTagProps.h"

IMPLEMENT_CLASS(rvRoboTagProps, wxPropertySheetDialog)

BEGIN_EVENT_TABLE(rvRoboTagProps, wxPropertySheetDialog)
    EVT_RADIOBOX(ID_CAMERA_VIEW, rvRoboTagProps::OnCameraView)
    EVT_RADIOBOX(ID_EDGE_METHOD, rvRoboTagProps::OnEdgeMethod)
    EVT_SPINCTRL(ID_EDGE_DILATION, rvRoboTagProps::OnEdgeDilation)
    EVT_SPINCTRL(ID_GAUSSIAN_BLUR, rvRoboTagProps::OnGaussianBlur)
    EVT_RADIOBOX(ID_ADAPTIVE_METHOD, rvRoboTagProps::OnAdaptiveMethod)
    EVT_SPINCTRL(ID_ADAPTIVE_BLOCKSIZE, rvRoboTagProps::OnAdaptiveBlockSize)
    EVT_SPINCTRL(ID_ADAPTIVE_SUBTRACTION, rvRoboTagProps::OnAdaptiveSubtraction)
    EVT_CHECKBOX(ID_DRAW_RAW_CONTOURS, rvRoboTagProps::OnDrawSetting)
    EVT_CHECKBOX(ID_DRAW_POLYGON_CONTOURS, rvRoboTagProps::OnDrawSetting)
    EVT_CHECKBOX(ID_DRAW_QUAD_CONTOURS, rvRoboTagProps::OnDrawSetting)
    EVT_CHECKBOX(ID_DRAW_TAG_REFERENCES, rvRoboTagProps::OnDrawSetting)
    EVT_CHECKBOX(ID_DRAW_TAG_SAMPLES, rvRoboTagProps::OnDrawSetting)
    EVT_CHECKBOX(ID_DRAW_TAG_CORNERS, rvRoboTagProps::OnDrawSetting)
    EVT_CHECKBOX(ID_DRAW_TAG_INDENTIFIERS, rvRoboTagProps::OnDrawSetting)
    EVT_CHECKBOX(ID_DRAW_CAMERA_POSITION, rvRoboTagProps::OnDrawSetting)
    EVT_CHECKBOX(ID_DRAW_TAG_REPROJECTION, rvRoboTagProps::OnDrawSetting)
    EVT_CHECKBOX(ID_DRAW_OBJECT_REPROJECTION, rvRoboTagProps::OnDrawSetting)
    EVT_CHECKBOX(ID_DRAW_CHARACTERS, rvRoboTagProps::OnDrawSetting)
END_EVENT_TABLE()


rvRoboTagProps::rvRoboTagProps(wxWindow *parent, rvCamera* camera)
{
    // Save the camera object.
    m_camera = camera;

    // Create the property notebook.
    Create(parent, wxID_ANY, wxString(wxT("RoboTag Properties")), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);

    // Create a standard OK button on the bottom of the dialog box.
    CreateButtons(wxOK);

    // Get the notebook that contains the panels we will be creating below.
    wxBookCtrlBase* notebook = GetBookCtrl();

    wxPanel* generalSettings = CreateGeneralPanel(notebook);
    wxPanel* adaptiveThresholdSettings = CreateAdaptiveThresholdPanel(notebook);
    wxPanel* cannyEdgeSettings = CreateCannyEdgePanel(notebook);
    wxPanel* suzanEdgeSettings = CreateSuzanEdgePanel(notebook);
    wxPanel* displayOptionSettings = CreateDisplayOptionsPanel(notebook);

    notebook->AddPage(generalSettings, wxT("General"), true);
    notebook->AddPage(adaptiveThresholdSettings, wxT("Adaptive Threshold"), false);
    notebook->AddPage(cannyEdgeSettings, wxT("Canny Edge"), false);
    notebook->AddPage(suzanEdgeSettings, wxT("Susan Edge"), false);
    notebook->AddPage(displayOptionSettings, wxT("Display Options"), false);

    // Layout the notebook.
    notebook->Layout();

    // Layout the panels.
    LayoutDialog();
}


// We override this method to specify a multi-line notebook control.
wxBookCtrlBase* rvRoboTagProps::CreateBookCtrl()
{
    return new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_MULTILINE);
}


wxPanel* rvRoboTagProps::CreateGeneralPanel(wxWindow* parent)
{
    rvGrid* grid = m_camera->GetGrid();
    wxPanel* panel = new wxPanel(parent, wxID_ANY);
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* item0 = new wxBoxSizer(wxVERTICAL);

    // Configure the camera views control.
    wxString cameraViews[3];
    cameraViews[0] = wxT("&Color");
    cameraViews[1] = wxT("&Gray");
    cameraViews[2] = wxT("&Edge");

    m_cameraView = new wxRadioBox(panel, ID_CAMERA_VIEW, wxT("Camera &View:"), wxDefaultPosition, wxDefaultSize, 3, cameraViews, 1, wxRA_SPECIFY_ROWS);
    item0->Add(m_cameraView, 0, wxGROW | wxALL, 5);
    m_cameraView->SetSelection(rvGrid_GetDisplay(grid));

    // Configure edge detection method.
    wxString edgeMethods[3];
    edgeMethods[0] = wxT("&Adaptive Threshold");
    edgeMethods[1] = wxT("&Canny Edge");
    edgeMethods[2] = wxT("&Suzan Edge");

    m_edgeMethod = new wxRadioBox(panel, ID_EDGE_METHOD, wxT("&Edge Detection Method:"), wxDefaultPosition, wxDefaultSize, 3, edgeMethods, 1, wxRA_SPECIFY_ROWS);
    item0->Add(m_edgeMethod, 0, wxGROW | wxALL, 5);
    m_edgeMethod->SetSelection(rvGrid_GetEdgeMethod(grid));

    // Configure the edge dilation control.
    wxStaticBox* staticBox1 = new wxStaticBox(panel, wxID_ANY, wxT("Edge &Dilation:"));
    wxBoxSizer* itemSizer1 = new wxStaticBoxSizer(staticBox1, wxHORIZONTAL);

    m_edgeDilation = new wxSpinCtrl(panel, ID_EDGE_DILATION, wxEmptyString, wxDefaultPosition, wxSize(80, wxDefaultCoord), wxSP_ARROW_KEYS, 0, 20, rvGrid_GetEdgeDilation(grid));
    itemSizer1->Add(m_edgeDilation, 0, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    itemSizer1->Add(new wxStaticText(panel, wxID_STATIC, wxT("pixels")), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    item0->Add(itemSizer1, 0, wxGROW | wxLEFT | wxRIGHT, 5);

    // Configure the guassian blur control.
    wxStaticBox* staticBox2 = new wxStaticBox(panel, wxID_ANY, wxT("Gaussian &Blur:"));
    wxBoxSizer* itemSizer2 = new wxStaticBoxSizer(staticBox2, wxHORIZONTAL);

    m_gaussianBlur = new wxSpinCtrl(panel, ID_GAUSSIAN_BLUR, wxEmptyString, wxDefaultPosition, wxSize(80, wxDefaultCoord), wxSP_ARROW_KEYS, 0, 30, rvGrid_GetGaussianBlur(grid));
    itemSizer2->Add(m_gaussianBlur, 0, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    itemSizer2->Add(new wxStaticText(panel, wxID_STATIC, wxT("pixels")), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    item0->Add(itemSizer2, 0, wxGROW | wxLEFT | wxRIGHT, 5);

    // Layout the panel.
    topSizer->Add(item0, 1, wxGROW | wxALIGN_CENTRE | wxALL, 5);
    panel->SetSizer(topSizer);
    topSizer->Fit(panel);

    return panel;
}


void rvRoboTagProps::OnCameraView(wxCommandEvent& WXUNUSED(event))
{
    rvGrid* grid = m_camera->GetGrid();

    // Set the display.
    rvGrid_SetDisplay(grid, m_cameraView->GetSelection());
    m_cameraView->SetSelection(rvGrid_GetDisplay(grid));
}


void rvRoboTagProps::OnEdgeMethod(wxCommandEvent& WXUNUSED(event))
{
    rvGrid* grid = m_camera->GetGrid();

    // Set the edge detection method.
    rvGrid_SetEdgeMethod(grid, m_edgeMethod->GetSelection());
    m_edgeMethod->SetSelection(rvGrid_GetEdgeMethod(grid));
}


void rvRoboTagProps::OnEdgeDilation(wxSpinEvent& WXUNUSED(event))
{
    rvGrid* grid = m_camera->GetGrid();

    // Set the edge dilation.
    rvGrid_SetEdgeDilation(grid, m_edgeDilation->GetValue());
    m_edgeDilation->SetValue(rvGrid_GetEdgeDilation(grid));
}


void rvRoboTagProps::OnGaussianBlur(wxSpinEvent& WXUNUSED(event))
{
    rvGrid* grid = m_camera->GetGrid();

    // Set the gaussian blur.
    rvGrid_SetGaussianBlur(grid, m_gaussianBlur->GetValue());
    m_gaussianBlur->SetValue(rvGrid_GetGaussianBlur(grid));
}


wxPanel* rvRoboTagProps::CreateAdaptiveThresholdPanel(wxWindow* parent)
{
    rvGrid* grid = m_camera->GetGrid();
    wxPanel* panel = new wxPanel(parent, wxID_ANY);
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* item0 = new wxBoxSizer(wxVERTICAL);

    // Configure edge detection method.
    wxString adaptiveMethods[3];
    adaptiveMethods[0] = wxT("&Mean");
    adaptiveMethods[1] = wxT("&Gaussian");

    m_adaptiveMethod = new wxRadioBox(panel, ID_ADAPTIVE_METHOD, wxT("&Method:"), wxDefaultPosition, wxDefaultSize, 2, adaptiveMethods, 1, wxRA_SPECIFY_ROWS);
    item0->Add(m_adaptiveMethod, 0, wxGROW | wxALL, 5);
    m_adaptiveMethod->SetSelection(rvGrid_GetAdaptiveMethod(grid));

    // Configure the adaptive block size.
    wxStaticBox* staticBox1 = new wxStaticBox(panel, wxID_ANY, wxT("Block Size:"));
    wxBoxSizer* itemSizer1 = new wxStaticBoxSizer(staticBox1, wxHORIZONTAL);

    m_adaptiveBlockSize = new wxSpinCtrl(panel, ID_ADAPTIVE_BLOCKSIZE, wxEmptyString, wxDefaultPosition, wxSize(80, wxDefaultCoord), wxSP_ARROW_KEYS, 1, 255, rvGrid_GetAdaptiveBlockSize(grid));
    itemSizer1->Add(m_adaptiveBlockSize, 0, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    item0->Add(itemSizer1, 0, wxGROW | wxLEFT | wxRIGHT, 5);

    // Configure the adaptive subtraction.
    wxStaticBox* staticBox2 = new wxStaticBox(panel, wxID_ANY, wxT("Subtraction:"));
    wxBoxSizer* itemSizer2 = new wxStaticBoxSizer(staticBox2, wxHORIZONTAL);

    m_adaptiveSubtraction = new wxSpinCtrl(panel, ID_ADAPTIVE_SUBTRACTION, wxEmptyString, wxDefaultPosition, wxSize(80, wxDefaultCoord), wxSP_ARROW_KEYS, -255, 255, rvGrid_GetAdaptiveSubtraction(grid));
    itemSizer2->Add(m_adaptiveSubtraction, 0, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    item0->Add(itemSizer2, 0, wxGROW | wxLEFT | wxRIGHT, 5);

    // Layout the panel.
    topSizer->Add(item0, 1, wxGROW | wxALIGN_CENTRE | wxALL, 5);
    panel->SetSizer(topSizer);
    topSizer->Fit(panel);

    return panel;
}


void rvRoboTagProps::OnAdaptiveMethod(wxCommandEvent& WXUNUSED(event))
{
    rvGrid* grid = m_camera->GetGrid();

    // Set the adaptive threshold method.
    rvGrid_SetAdaptiveMethod(grid, m_adaptiveMethod->GetSelection());
    m_adaptiveMethod->SetSelection(rvGrid_GetAdaptiveMethod(grid));
}


void rvRoboTagProps::OnAdaptiveBlockSize(wxSpinEvent& WXUNUSED(event))
{
    rvGrid* grid = m_camera->GetGrid();

    // Set the adaptive block size value.
    rvGrid_SetAdaptiveBlockSize(grid, m_adaptiveBlockSize->GetValue());
    m_adaptiveBlockSize->SetValue(rvGrid_GetAdaptiveBlockSize(grid));
}


void rvRoboTagProps::OnAdaptiveSubtraction(wxSpinEvent& WXUNUSED(event))
{
    rvGrid* grid = m_camera->GetGrid();

    // Set the adaptive subtraction value.
    rvGrid_SetAdaptiveSubtraction(grid, m_adaptiveSubtraction->GetValue());
    m_adaptiveSubtraction->SetValue(rvGrid_GetAdaptiveSubtraction(grid));
}


wxPanel* rvRoboTagProps::CreateCannyEdgePanel(wxWindow* parent)
{
    rvGrid* grid = m_camera->GetGrid();
    wxPanel* panel = new wxPanel(parent, wxID_ANY);
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* item0 = new wxBoxSizer(wxVERTICAL);

    // Layout the panel.
    topSizer->Add(item0, 1, wxGROW | wxALIGN_CENTRE | wxALL, 5);
    panel->SetSizer(topSizer);
    topSizer->Fit(panel);

    return panel;
}


wxPanel* rvRoboTagProps::CreateSuzanEdgePanel(wxWindow* parent)
{
    rvGrid* grid = m_camera->GetGrid();
    wxPanel* panel = new wxPanel(parent, wxID_ANY);
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* item0 = new wxBoxSizer(wxVERTICAL);

    // Layout the panel.
    topSizer->Add(item0, 1, wxGROW | wxALIGN_CENTRE | wxALL, 5);
    panel->SetSizer(topSizer);
    topSizer->Fit(panel);

    return panel;
}


wxPanel* rvRoboTagProps::CreateDisplayOptionsPanel(wxWindow* parent)
{
    rvGrid* grid = m_camera->GetGrid();
    wxPanel* panel = new wxPanel(parent, wxID_ANY);
    wxBoxSizer* item0 = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* itemSizer = NULL;

    // Draw raw contours.
    itemSizer = new wxBoxSizer(wxHORIZONTAL);
    m_drawRawContours = new wxCheckBox(panel, ID_DRAW_RAW_CONTOURS, wxT("Draw raw contours"), wxDefaultPosition, wxDefaultSize);
    itemSizer->Add(m_drawRawContours, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    item0->Add(itemSizer, 0, wxGROW | wxALL, 0);
    m_drawRawContours->SetValue(rvGrid_GetDrawRawContours(grid));

    // Draw polygon contours.
    itemSizer = new wxBoxSizer(wxHORIZONTAL);
    m_drawPolygonContours = new wxCheckBox(panel, ID_DRAW_POLYGON_CONTOURS, wxT("Draw polygon contours"), wxDefaultPosition, wxDefaultSize);
    itemSizer->Add(m_drawPolygonContours, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    item0->Add(itemSizer, 0, wxGROW | wxALL, 0);
    m_drawPolygonContours->SetValue(rvGrid_GetDrawPolygonContours(grid));

    // Draw quad contours.
    itemSizer = new wxBoxSizer(wxHORIZONTAL);
    m_drawQuadContours = new wxCheckBox(panel, ID_DRAW_QUAD_CONTOURS, wxT("Draw quad contours"), wxDefaultPosition, wxDefaultSize);
    itemSizer->Add(m_drawQuadContours, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    item0->Add(itemSizer, 0, wxGROW | wxALL, 0);
    m_drawQuadContours->SetValue(rvGrid_GetDrawQuadContours(grid));

    // Draw tag corners.
    itemSizer = new wxBoxSizer(wxHORIZONTAL);
    m_drawTagCorners = new wxCheckBox(panel, ID_DRAW_TAG_CORNERS, wxT("Draw tag corners"), wxDefaultPosition, wxDefaultSize);
    itemSizer->Add(m_drawTagCorners, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    item0->Add(itemSizer, 0, wxGROW | wxALL, 0);
    m_drawTagCorners->SetValue(rvGrid_GetDrawTagCorners(grid));

    // Draw tag references.
    itemSizer = new wxBoxSizer(wxHORIZONTAL);
    m_drawTagReferences = new wxCheckBox(panel, ID_DRAW_TAG_REFERENCES, wxT("Draw tag references"), wxDefaultPosition, wxDefaultSize);
    itemSizer->Add(m_drawTagReferences, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    item0->Add(itemSizer, 0, wxGROW | wxALL, 0);
    m_drawTagReferences->SetValue(rvGrid_GetDrawTagReferences(grid));

    // Draw tag samples.
    itemSizer = new wxBoxSizer(wxHORIZONTAL);
    m_drawTagSamples = new wxCheckBox(panel, ID_DRAW_TAG_SAMPLES, wxT("Draw tag samples"), wxDefaultPosition, wxDefaultSize);
    itemSizer->Add(m_drawTagSamples, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    item0->Add(itemSizer, 0, wxGROW | wxALL, 0);
    m_drawTagSamples->SetValue(rvGrid_GetDrawTagSamples(grid));

    // Draw tag identifiers.
    itemSizer = new wxBoxSizer(wxHORIZONTAL);
    m_drawTagIdentifiers = new wxCheckBox(panel, ID_DRAW_TAG_INDENTIFIERS, wxT("Draw tag identifiers"), wxDefaultPosition, wxDefaultSize);
    itemSizer->Add(m_drawTagIdentifiers, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    item0->Add(itemSizer, 0, wxGROW | wxALL, 0);
    m_drawTagIdentifiers->SetValue(rvGrid_GetDrawTagIdentifiers(grid));

    // Draw camera position.
    itemSizer = new wxBoxSizer(wxHORIZONTAL);
    m_drawCameraPosition = new wxCheckBox(panel, ID_DRAW_CAMERA_POSITION, wxT("Draw camera position"), wxDefaultPosition, wxDefaultSize);
    itemSizer->Add(m_drawCameraPosition, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    item0->Add(itemSizer, 0, wxGROW | wxALL, 0);
    m_drawCameraPosition->SetValue(rvGrid_GetDrawCameraPosition(grid));

    // Draw tag reprojection.
    itemSizer = new wxBoxSizer(wxHORIZONTAL);
    m_drawTagReprojection = new wxCheckBox(panel, ID_DRAW_TAG_REPROJECTION, wxT("Draw tag reprojection"), wxDefaultPosition, wxDefaultSize);
    itemSizer->Add(m_drawTagReprojection, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    item0->Add(itemSizer, 0, wxGROW | wxALL, 0);
    m_drawTagReprojection->SetValue(rvGrid_GetDrawTagReprojection(grid));

    // Draw object reprojection.
    itemSizer = new wxBoxSizer(wxHORIZONTAL);
    m_drawObjectReprojection = new wxCheckBox(panel, ID_DRAW_OBJECT_REPROJECTION, wxT("Draw object reprojection"), wxDefaultPosition, wxDefaultSize);
    itemSizer->Add(m_drawObjectReprojection, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    item0->Add(itemSizer, 0, wxGROW | wxALL, 0);
    m_drawObjectReprojection->SetValue(rvGrid_GetDrawObjectReprojection(grid));

    // Draw object reprojection.
    itemSizer = new wxBoxSizer(wxHORIZONTAL);
    m_drawCharacters = new wxCheckBox(panel, ID_DRAW_CHARACTERS, wxT("Draw characters"), wxDefaultPosition, wxDefaultSize);
    itemSizer->Add(m_drawCharacters, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    item0->Add(itemSizer, 0, wxGROW | wxALL, 0);
    m_drawCharacters->SetValue(rvGrid_GetDrawCharacters(grid));

    // Layout the panel.
    topSizer->Add(item0, 1, wxGROW | wxALIGN_CENTRE | wxALL, 5);
    panel->SetSizer(topSizer);
    topSizer->Fit(panel);

    return panel;
}

void rvRoboTagProps::OnDrawSetting(wxCommandEvent& event)
{
    int id = event.GetId();
    rvGrid* grid = m_camera->GetGrid();

    switch (id)
    {
        case ID_DRAW_RAW_CONTOURS:
            rvGrid_SetDrawRawContours(grid, m_drawRawContours->GetValue());
            m_drawRawContours->SetValue(rvGrid_GetDrawRawContours(grid));
            break;

        case ID_DRAW_POLYGON_CONTOURS:
            rvGrid_SetDrawPolygonContours(grid, m_drawPolygonContours->GetValue());
            m_drawPolygonContours->SetValue(rvGrid_GetDrawPolygonContours(grid));
            break;

        case ID_DRAW_QUAD_CONTOURS:
            rvGrid_SetDrawQuadContours(grid, m_drawQuadContours->GetValue());
            m_drawQuadContours->SetValue(rvGrid_GetDrawQuadContours(grid));
            break;

        case ID_DRAW_TAG_REFERENCES:
            rvGrid_SetDrawTagReferences(grid, m_drawTagReferences->GetValue());
            m_drawTagReferences->SetValue(rvGrid_GetDrawTagReferences(grid));
            break;

        case ID_DRAW_TAG_SAMPLES:
            rvGrid_SetDrawTagSamples(grid, m_drawTagSamples->GetValue());
            m_drawTagSamples->SetValue(rvGrid_GetDrawTagSamples(grid));
            break;

        case ID_DRAW_TAG_CORNERS:
            rvGrid_SetDrawTagCorners(grid, m_drawTagCorners->GetValue());
            m_drawTagCorners->SetValue(rvGrid_GetDrawTagCorners(grid));
            break;

        case ID_DRAW_TAG_INDENTIFIERS:
            rvGrid_SetDrawTagIdentifiers(grid, m_drawTagIdentifiers->GetValue());
            m_drawTagIdentifiers->SetValue(rvGrid_GetDrawTagIdentifiers(grid));
            break;

        case ID_DRAW_CAMERA_POSITION:
            rvGrid_SetDrawCameraPosition(grid, m_drawCameraPosition->GetValue());
            m_drawCameraPosition->SetValue(rvGrid_GetDrawCameraPosition(grid));
            break;

        case ID_DRAW_TAG_REPROJECTION:
            rvGrid_SetDrawTagReprojection(grid, m_drawTagReprojection->GetValue());
            m_drawTagReprojection->SetValue(rvGrid_GetDrawTagReprojection(grid));
            break;

        case ID_DRAW_OBJECT_REPROJECTION:
            rvGrid_SetDrawObjectReprojection(grid, m_drawObjectReprojection->GetValue());
            m_drawObjectReprojection->SetValue(rvGrid_GetDrawObjectReprojection(grid));
            break;

        case ID_DRAW_CHARACTERS:
            rvGrid_SetDrawCharacters(grid, m_drawCharacters->GetValue());
            m_drawCharacters->SetValue(rvGrid_GetDrawCharacters(grid));
            break;
    }
}


