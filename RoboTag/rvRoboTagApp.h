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

    $Id: rvRoboTagApp.h 28 2010-03-09 23:49:39Z mike $
*/

#ifndef _RV_ROBOTAGAPP_H_
#define _RV_ROBOTAGAPP_H_

#include "wx/wx.h"

// Declare the application class
class rvRoboTagApp : public wxApp
{
public:
    // Called on application startup
    virtual bool OnInit();
    virtual int OnExit();

    // Application level methods.
    wxString GetAppDir();

private:
    wxString m_appDir;
};

// Implements rvRoboTagApp& GetApp()
DECLARE_APP(rvRoboTagApp)

#endif // _RV_ROBOTAGAPP_H_
