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

    $Id: rvRoboTagCalibrate.h 28 2010-03-09 23:49:39Z mike $
*/

#ifndef _RV_ROBOTAGCALIBRATE_H_
#define _RV_ROBOTAGCALIBRATE_H_

#include "wx/wx.h"
#include "rvCamera.h"
#include "rvGrid.h"

BEGIN_DECLARE_EVENT_TYPES()
END_DECLARE_EVENT_TYPES()

class rvRoboTagCalibrate: public wxDialog
{
public:
    rvRoboTagCalibrate(wxWindow *parent, rvCamera* camera);

    bool Show(bool show);

    void OnCalibrateAdd(wxCommandEvent& event);
    void OnCalibrateProcess(wxCommandEvent& event);
    void OnCalibrateReset(wxCommandEvent& event);
    // void OnClose(wxCloseEvent& event);

private:

    rvGrid* m_grid;
    rvCamera* m_camera;

    wxStaticText *m_tagCountLabel;

    enum {
        ID_CALIBRATE_ADD = 100,
        ID_CALIBRATE_PROCESS,
        ID_CALIBRATE_RESET,
        ID_CALIBRATE_TAG_COUNT
    };

    DECLARE_EVENT_TABLE()
};


#endif // _RV_ROBOTAGCALIBRATE_H_