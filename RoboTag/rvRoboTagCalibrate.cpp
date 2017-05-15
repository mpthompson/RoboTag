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

    $Id: rvRoboTagCalibrate.cpp 28 2010-03-09 23:49:39Z mike $
*/

#include "wx/wx.h"
#include "wx/statline.h"
#include "rvRoboTagCalibrate.h"

enum
{
    DIALOGS_MODELESS_BTN
};


BEGIN_EVENT_TABLE(rvRoboTagCalibrate, wxDialog)
    EVT_BUTTON(ID_CALIBRATE_ADD, rvRoboTagCalibrate::OnCalibrateAdd)
    EVT_BUTTON(ID_CALIBRATE_PROCESS, rvRoboTagCalibrate::OnCalibrateProcess)
    EVT_BUTTON(ID_CALIBRATE_RESET, rvRoboTagCalibrate::OnCalibrateReset)
    // EVT_CLOSE(rvRoboTagCalibrate::OnClose)
END_EVENT_TABLE()


rvRoboTagCalibrate::rvRoboTagCalibrate(wxWindow *parent, rvCamera* camera)
        : wxDialog(parent, wxID_ANY, wxString(wxT("RoboTag Calibrate")))
{
    // Save the camera object.
    m_camera = camera;

    wxBoxSizer *vertSizer = new wxBoxSizer(wxVERTICAL);

    // Add the static labels.
    m_tagCountLabel = new wxStaticText(this, ID_CALIBRATE_TAG_COUNT, wxT("0 tags over 0 images for calibration"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
    vertSizer->Add(m_tagCountLabel, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);

    // Horizontal sizer to contain the operation buttons.
    wxStaticBoxSizer *horzSizer = new wxStaticBoxSizer(wxHORIZONTAL, this, wxT("Calibrate Operations"));
    vertSizer->Add(horzSizer, 0, wxGROW | wxALL, 5);

    // Add the operation buttons.
    wxButton *addButton = new wxButton(this, ID_CALIBRATE_ADD, wxT("Add Tags"));
    wxButton *processButton = new wxButton(this, ID_CALIBRATE_PROCESS, wxT("Process Tags"));
    wxButton *resetButton = new wxButton(this, ID_CALIBRATE_RESET, wxT("Reset Tags"));
    horzSizer->Add(addButton, 1, wxEXPAND | wxALL, 5);
    horzSizer->Add(processButton, 1, wxEXPAND | wxALL, 5);
    horzSizer->Add(resetButton, 1, wxEXPAND | wxALL, 5);

    // Add a horizontal line.
    // vertSizer->Add(new wxStaticLine(this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL), 0, wxGROW | wxALL, 5);

    // Add the standard buttons.
    vertSizer->Add(CreateStdDialogButtonSizer(wxOK), 1, wxEXPAND | wxALL, 5);

    SetSizer(vertSizer);
    vertSizer->SetSizeHints(this);
    vertSizer->Fit(this);
}


bool rvRoboTagCalibrate::Show(bool show)
{
    rvGrid* grid = m_camera->GetGrid();

    // Reset the calibration data.
    rvGrid_CalibrateReset(grid);

    // Update the tag and image counts.
    int tagCount = rvGrid_GetCalibrateTagCount(grid);
    int imageCount = rvGrid_GetCalibrateImageCount(grid);
    wxString tagCountString;
    tagCountString.Printf(wxT("%d tags and %d images for calibration"), tagCount, imageCount);
    m_tagCountLabel->SetLabel(tagCountString);

    return wxDialog::Show(show);
}


void rvRoboTagCalibrate::OnCalibrateAdd(wxCommandEvent& WXUNUSED(event))
{
    rvGrid* grid = m_camera->GetGrid();

    // Add the latest calibration.
    rvGrid_CalibrateAdd(grid);

    // Update the tag and image counts.
    int tagCount = rvGrid_GetCalibrateTagCount(grid);
    int imageCount = rvGrid_GetCalibrateImageCount(grid);
    wxString tagCountString;
    tagCountString.Printf(wxT("%d tags and %d images for calibration"), tagCount, imageCount);
    m_tagCountLabel->SetLabel(tagCountString);
}


void rvRoboTagCalibrate::OnCalibrateProcess(wxCommandEvent& WXUNUSED(event))
{
    rvGrid* grid = m_camera->GetGrid();

    // Process the calibration.
    rvGrid_Calibrate(grid);

    // Update the tag and image counts.
    int tagCount = rvGrid_GetCalibrateTagCount(grid);
    int imageCount = rvGrid_GetCalibrateImageCount(grid);
    wxString tagCountString;
    tagCountString.Printf(wxT("%d tags and %d images for calibration"), tagCount, imageCount);
    m_tagCountLabel->SetLabel(tagCountString);
}


void rvRoboTagCalibrate::OnCalibrateReset(wxCommandEvent& WXUNUSED(event))
{
    rvGrid* grid = m_camera->GetGrid();

    // Reset the calibration.
    rvGrid_CalibrateReset(grid);

    // Update the tag and image counts.
    int tagCount = rvGrid_GetCalibrateTagCount(grid);
    int imageCount = rvGrid_GetCalibrateImageCount(grid);
    wxString tagCountString;
    tagCountString.Printf(wxT("%d tags and %d images for calibration"), tagCount, imageCount);
    m_tagCountLabel->SetLabel(tagCountString);
}

#if 0
void rvRoboTagCalibrate::OnClose(wxCloseEvent& event)
{
}
#endif

