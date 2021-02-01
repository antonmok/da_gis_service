#include <stdio.h>
#include <sstream>
#include <memory>
#include <string>
#include <thread>

#include "wxMain.h"

IMPLEMENT_APP(CWxMonitorApp)

// This is executed upon startup, like 'main()' in non-wxWidgets programs.
bool CWxMonitorApp::OnInit()
{
	MainFrame *frame = new MainFrame(_T("Oracle MP algo player"), wxPoint(790, 520), wxSize(850, 460));
	
	frame->Show(true);
	SetTopWindow(frame);

	// run player backend
	//std::thread playerThread(&CWxMonitor::PlayerThread, playerInst, frame);
	//playerThread.detach();

	//std::thread cvWindowThread(&CWxMonitor::CvWindowThread, playerInst);
	//cvWindowThread.detach();

	//CPlayControl::Instance().push({ PlayControlMessageType::Open, "/home/anton/Projects/Oracle/test.ubd", 0 });

	return true;
}

BEGIN_EVENT_TABLE ( MainFrame, wxFrame )
	EVT_BUTTON (BUTTON_EXIT, MainFrame::OnExit)
	EVT_BUTTON (BUTTON_SEND, MainFrame::OnSend)

END_EVENT_TABLE()

MainFrame::MainFrame(const wxString &title, const wxPoint &pos, const wxSize &size): wxFrame((wxFrame*)NULL,  -1, title, pos, size)
{
	unsigned btnWidth = 95, btnHeight = 40, spacing = 10, row = 1;

	SendBtn = new wxButton(this, BUTTON_SEND, _T("Send request"),
		wxPoint(2 * btnWidth + spacing * 3, spacing),
		wxSize(btnWidth, btnHeight), 0);

	statusLb = new wxStaticText(this, STATIC_STATUS, _T(""),
		wxPoint(spacing, (btnHeight + spacing) * row +  spacing));

	ExitBtn = new wxButton(this, BUTTON_EXIT, _T("Exit"), 
		wxPoint(spacing, (btnHeight + spacing) * row +  spacing), 
		wxSize(btnWidth, btnHeight), 0);
}

void MainFrame::OnSend(wxCommandEvent& event)
{
	(void)event;

	// TODO: send request to server
}

void MainFrame::OnExit(wxCommandEvent& event)
{
	(void)event;

	Close(TRUE);
}

