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

    $Id: rvRoboTagFrame.cpp 28 2010-03-09 23:49:39Z mike $
*/

#include "wx/wx.h"
#include "wx/file.h"
#include "rvRoboTagFrame.h"
#include "rvRoboTagProps.h"

enum
{
    MENU_SHOW_ROBOTAG_PROPERTIES = 100,
    MENU_SHOW_ROBOTAG_CALIBRATE,
    MENU_SHOW_PIN_PROPERTIES,
    MENU_SHOW_FILTER_PROPERTIES
};

// Event table for rvRoboTagFrame
BEGIN_EVENT_TABLE(rvRoboTagFrame, wxFrame)
    EVT_MENU(wxID_NEW, rvRoboTagFrame::OnNew)
    EVT_MENU(wxID_OPEN, rvRoboTagFrame::OnOpen)
    EVT_MENU(wxID_SAVE, rvRoboTagFrame::OnSave)
    EVT_MENU(wxID_SAVEAS, rvRoboTagFrame::OnSaveAs)
    EVT_MENU(wxID_EXIT, rvRoboTagFrame::OnExit)
    EVT_MENU(wxID_ABOUT, rvRoboTagFrame::OnAbout)
    EVT_MENU(MENU_SHOW_ROBOTAG_PROPERTIES, rvRoboTagFrame::OnShowRoboTagProperties)
    EVT_MENU(MENU_SHOW_ROBOTAG_CALIBRATE, rvRoboTagFrame::OnShowRoboTagCalibrate)
    EVT_MENU(MENU_SHOW_PIN_PROPERTIES, rvRoboTagFrame::OnShowPinProperties)
    EVT_MENU(MENU_SHOW_FILTER_PROPERTIES, rvRoboTagFrame::OnShowFilterProperties)
END_EVENT_TABLE()

#include "robotag_icon.xpm"

rvRoboTagFrame::rvRoboTagFrame(const wxString& title)
             : wxFrame(NULL, wxID_ANY, title)
{
    // Clear out the file path.
    m_filepath = wxEmptyString;

    // Set the frame icon
    SetIcon(wxIcon(robotag_icon_xpm));

    // Create a menu bar.
    wxMenuBar *menuBar = new wxMenuBar();

    // Create the menus that should go into the menu bar.
    wxMenu *fileMenu = new wxMenu;
    wxMenu *settingsMenu = new wxMenu;
    wxMenu *helpMenu = new wxMenu;

    // File operations for intrinsics.
    fileMenu->Append(wxID_NEW, _T("&New"));
    fileMenu->Append(wxID_OPEN, _T("&Open..."));
    fileMenu->Append(wxID_SAVE, _T("&Save"));
    fileMenu->Append(wxID_SAVEAS, _T("Save &As..."));
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT, wxT("E&xit\tAlt-X"), wxT("Quit this program"));

    // Append the pin and filter properties menu items.
    settingsMenu->Append(MENU_SHOW_ROBOTAG_PROPERTIES, wxT("RoboTag Properties..."), wxT("Show RoboTag properties"));
    settingsMenu->Append(MENU_SHOW_ROBOTAG_CALIBRATE, wxT("RoboTag Calibrate..."), wxT("Start calibration process"));
    settingsMenu->Append(MENU_SHOW_PIN_PROPERTIES, wxT("Video Stream Properties..."), wxT("Show video stream properties"));
    settingsMenu->Append(MENU_SHOW_FILTER_PROPERTIES, wxT("Video Source Properties..."), wxT("Show video source properties"));

    // The "About" item should be in the help menu
    helpMenu->Append(wxID_ABOUT, wxT("&About...\tF1"), wxT("Show about dialog"));

    // Now append the freshly created menus to the menu bar.
    menuBar->Append(fileMenu, wxT("&File"));
    menuBar->Append(settingsMenu, wxT("&Settings"));
    menuBar->Append(helpMenu, wxT("&Help"));

    // Attach the menu bar to the frame.
    SetMenuBar(menuBar);

    // Create a status bar just for fun
    CreateStatusBar(2);
    SetStatusText(wxT("Camera Frame"));

    // Set the client size.
    SetClientSize(640, 480);

    // Create the camera within this frame.
    m_camera = new rvCamera(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);

    // Initialize the properties dialog.
    m_propsDialog = (rvRoboTagProps *) NULL;
    m_calibrateDialog = (rvRoboTagCalibrate *) NULL;
}


void rvRoboTagFrame::OnAbout(wxCommandEvent &event)
{
    wxMessageBox(wxT("Twitch\n")
        wxT("Mike Thompson (c) 2009\n")
        wxT("mpthompson@gmail.com\n\n")
        wxT("Test sandbox for machine vision algorithms."),
        wxT("About RoboTag"),
        wxICON_INFORMATION | wxOK);
}



void rvRoboTagFrame::OnNew(wxCommandEvent& WXUNUSED(event))
{
    // Clear out the file path.
    m_filepath = wxEmptyString;

    // Reset the camera intrinsics.
    rvGrid_ResetIntrinsics(m_camera->GetGrid());
}


void rvRoboTagFrame::OnOpen(wxCommandEvent& WXUNUSED(event))
{
    wxString caption = wxT("Open");
    wxString wildcard = wxT("Camera intrinsic files (*.yml)|*.yml");
    wxString defaultDir = wxT(".");
    wxString defaultFilename = wxEmptyString;

    wxFileDialog dialog(this, caption, defaultDir, defaultFilename, wildcard, wxFD_OPEN);

    if (dialog.ShowModal() == wxID_OK)
    {
        // Set the filepath for the file.
        m_filepath = dialog.GetPath();

        // Does the file exist?
        if (wxFile::Exists(m_filepath))
        {
            // Load the intrinsics.
            rvGrid_LoadIntrinsics(m_camera->GetGrid(), m_filepath.c_str());
        }
        else
        {
            // XXX Report file not found.
        }
    }
}


void rvRoboTagFrame::OnSave(wxCommandEvent& event)
{
    // Do we have a filepath for the file?
    if (m_filepath.IsEmpty())
    {
        // No.  We need to get a filepath and then save.
        OnSaveAs(event);
    }
    else
    {
        // Save the intrinsics.
        rvGrid_SaveIntrinsics(m_camera->GetGrid(), m_filepath.c_str());
    }
}


void rvRoboTagFrame::OnSaveAs(wxCommandEvent& WXUNUSED(event))
{
    wxString caption = wxT("Save As");
    wxString wildcard = wxT("Camera intrinsic files (*.yml)|*.yml");
    wxString defaultDir = wxT(".");
    wxString defaultFilename = wxEmptyString;

    wxFileDialog dialog(this, caption, defaultDir, defaultFilename, wildcard, wxFD_SAVE);

    if (dialog.ShowModal() == wxID_OK)
    {
        // Set the filepath for the file.
        m_filepath = dialog.GetPath();

        // Save the intrinsics.
        rvGrid_SaveIntrinsics(m_camera->GetGrid(), m_filepath.c_str());
    }
}


void rvRoboTagFrame::OnExit(wxCommandEvent &event)
{
    // Destroy the frame
    Close();
}


void rvRoboTagFrame::OnShowPinProperties(wxCommandEvent &event)
{
    wxCommandEvent cmdEvent(wxEVT_SHOW_PIN_PROPERTIES, GetId());
    cmdEvent.SetEventObject(this);
    m_camera->GetEventHandler()->ProcessEvent(cmdEvent);
}


void rvRoboTagFrame::OnShowFilterProperties(wxCommandEvent &event)
{
    wxCommandEvent cmdEvent(wxEVT_SHOW_FILTER_PROPERTIES, GetId());
    cmdEvent.SetEventObject(this);
    m_camera->GetEventHandler()->ProcessEvent(cmdEvent);
}


void rvRoboTagFrame::OnShowRoboTagProperties(wxCommandEvent& event)
{
    // Create the dialog if not yet made.
    if (!m_propsDialog) m_propsDialog = new rvRoboTagProps(this, m_camera);

    // Show the props dialg.
    m_propsDialog->Show(true);
}


void rvRoboTagFrame::OnShowRoboTagCalibrate(wxCommandEvent& event)
{
    // Create the dialog if not yet made.
    if (!m_calibrateDialog) m_calibrateDialog = new rvRoboTagCalibrate(this, m_camera);

    // Show the calibrate dialg.
    m_calibrateDialog->Show(true);
}

