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

    $Id: rvRoboTagApp.cpp 28 2010-03-09 23:49:39Z mike $
*/

#include "wx/wx.h"
#include "rvRoboTagApp.h"
#include "rvRoboTagFrame.h"

// Give wxWidgets the means to create a rvRoboTagApp object
IMPLEMENT_APP(rvRoboTagApp)

// Find the absolute path where this application has been run from.
// argv0 is wxTheApp->argv[0]
// cwd is the current working directory (at startup)
// appVariableName is the name of a variable containing the directory
// for this app, e.g. MYAPPDIR. This is checked first.
static wxString wxFindAppPath(const wxString& argv0, const wxString& cwd, const wxString& appVariableName)
{
    wxString str;

    // Try appVariableName
    if (!appVariableName.IsEmpty())
    {
        str = wxGetenv(appVariableName);
        if (!str.IsEmpty()) return str;
    }

#if defined(__WXMAC__) && !defined(__DARWIN__)
    // On Mac, the current directory is the relevant one when
    // the application starts.
    return cwd;
#endif

    if (wxIsAbsolutePath(argv0))
    {
        return wxPathOnly(argv0);
    }
    else
    {
        // Is it a relative path?
        wxString currentDir(cwd);
        if (currentDir.Last() != wxFILE_SEP_PATH) currentDir += wxFILE_SEP_PATH;

        str = currentDir + argv0;
        if (wxFileExists(str)) return wxPathOnly(str);
    }

    // OK, it's neither an absolute path nor a relative path.
    // Search PATH.
    wxPathList pathList;
    pathList.AddEnvList(wxT("PATH"));
    str = pathList.FindAbsoluteValidPath(argv0);
    if (!str.IsEmpty())
        return wxPathOnly(str);

    // Failed
    return wxEmptyString;
}

// Called on application initialization.
bool rvRoboTagApp::OnInit()
{
    // Get the application directory.
    m_appDir = wxFindAppPath(argv[0], wxGetCwd(), wxT("MYAPPDIR"));

    // Create the main application window.
    rvRoboTagFrame *frame = new rvRoboTagFrame(wxT("Robot Vision Application"));

    // Show the application window.
    frame->Show(true);

    // Start the event loop.
    return true;
}

// Called on application exit.
int rvRoboTagApp::OnExit()
{
    return wxApp::OnExit();
}

// Get the application path.
wxString rvRoboTagApp::GetAppDir()
{
    return m_appDir;
}
