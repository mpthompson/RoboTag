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

    $Id: rvRoboTagFrame.h 28 2010-03-09 23:49:39Z mike $
*/

#ifndef _RV_ROBOTAGFRAME_H_
#define _RV_ROBOTAGFRAME_H_

#include "wx/wx.h"
#include "rvCamera.h"
#include "rvRoboTagProps.h"
#include "rvRoboTagCalibrate.h"

// Declare our main frame class
class rvRoboTagFrame : public wxFrame
{
public:
    // Constructor
    rvRoboTagFrame(const wxString &title);

    // Event handlers
    void OnNew(wxCommandEvent& event);
    void OnOpen(wxCommandEvent& event);
    void OnSave(wxCommandEvent& event);
    void OnSaveAs(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnShowPinProperties(wxCommandEvent& event);
    void OnShowFilterProperties(wxCommandEvent& event);
    void OnShowRoboTagProperties(wxCommandEvent& event);
    void OnShowRoboTagCalibrate(wxCommandEvent& event);

private:

    // Filepath.
    wxString m_filepath;

    // Camera window.
    rvCamera *m_camera;

    // Dialog objects.
    rvRoboTagProps *m_propsDialog;
    rvRoboTagCalibrate *m_calibrateDialog;

    // This class handles events
    DECLARE_EVENT_TABLE()
};

#endif // _RV_ROBOTAGFRAME_H_
