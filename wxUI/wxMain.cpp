#include <stdio.h>
#include <sstream>
#include <memory>
#include <thread>

#include "wxMain.h"
#include "websocket_client.h"

IMPLEMENT_APP(CWxMonitorApp)

// This is executed upon startup, like 'main()' in non-wxWidgets programs.
bool CWxMonitorApp::OnInit()
{
	MainFrame *frame = new MainFrame(_T("da_gis_service client"), wxPoint(790, 520), wxSize(850, 460));
	
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
		wxPoint(spacing, spacing),
		wxSize(btnWidth, btnHeight), 0);

	statusLb = new wxStaticText(this, STATIC_STATUS, _T(""),
		wxPoint(spacing, (btnHeight + spacing) * row +  spacing));

	row = 4;
	ExitBtn = new wxButton(this, BUTTON_EXIT, _T("Exit"), 
		wxPoint(spacing, (btnHeight + spacing) * row +  spacing), 
		wxSize(btnWidth, btnHeight), 0);
}

void MainFrame::OnSend(wxCommandEvent& event)
{
	(void)event;

	spawn_client("127.0.0.1", "8082", "Hello world");
}

void MainFrame::OnExit(wxCommandEvent& event)
{
	(void)event;

	Close(TRUE);
}

