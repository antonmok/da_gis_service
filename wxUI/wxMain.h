#pragma once

// For compilers that don't support precompilation, include "wx/wx.h"
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#	include "wx/wx.h"
#endif

#include <X11/Xlib.h>
#include <filesystem>

class CWxMonitorApp : public wxApp
{
public:

    CWxMonitorApp() {
        XInitThreads();
    }

	virtual bool OnInit();
    //CWxMonitor inst;
};

class MainFrame: public wxFrame
{
public:
    MainFrame( const wxString &title, const wxPoint &pos, const wxSize &size );

    wxButton* ExitBtn;
    wxButton* SendBtn;

    wxStaticText* statusLb;

    std::filesystem::path _path {"No file opened"};

    void OnExit(wxCommandEvent& event);
    void OnSend(wxCommandEvent& event);
    
    DECLARE_EVENT_TABLE()
};

enum
{
    BUTTON_EXIT = wxID_HIGHEST + 1,
    BUTTON_SEND = wxID_HIGHEST + 2,

    STATIC_STATUS = wxID_HIGHEST + 10,
};

DECLARE_APP(CWxMonitorApp)

