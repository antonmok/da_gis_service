
#pragma once
//#include <WinSDKVer.h>
//#define _WIN32_WINNT_WIN7  
//#define _WIN32_WINNT

//#include "all.h"
//#include "pch.h"

#include <iostream>
#include <time.h>

#include "HttpParser.h"

#define UNICODE

// Internal name of the service 
#define SERVICE_NAME             L"NETLINE" 

// Displayed name of the service 
#define SERVICE_DISPLAY_NAME     L"NETLINE CLIENT OLVIA (SAMARA)" 

// Service start options. 
#define SERVICE_START_TYPE       SERVICE_DEMAND_START 

// List of service dependencies - "dep1\0dep2\0\0" 
#define SERVICE_DEPENDENCIES     L"" 

// The name of the account under which the service should run 
#define SERVICE_ACCOUNT          L"NT AUTHORITY\\LocalService" 

// The password to the service account name 
#define SERVICE_PASSWORD         NULL 

SERVICE_STATUS serviceStatus;
SERVICE_STATUS_HANDLE serviceStatusHandle;

//int InitService() { return 0; };

wchar_t servicePath[MAX_PATH];
int InstallService();
int StartClientService();
int RemoveService();

VOID WINAPI MainServiceProc(DWORD, LPWSTR* argv);
VOID WINAPI ControlHandler(DWORD request);

bool filePresent();
int saveLog(char *p);
int addLogMessage(char* p);

wchar_t path_of_log_file[MAX_PATH];
char logStr[1024];

Netline* netline = nullptr;

int main(int argc, char* argv[])
{
	SetErrorMode(SetErrorMode(SEM_NOGPFAULTERRORBOX) | SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);

	HMODULE hModule = GetModuleHandleW(NULL);
	GetModuleFileNameW(hModule, servicePath, MAX_PATH);
	wprintf(L"servicePath = %s\r\n", servicePath);

	//wcscpy(path_of_log_file,L"d:\\log-soap.log");
	memset(path_of_log_file, 0, sizeof(wchar_t)*MAX_PATH);

	for (int i = wcslen(servicePath) - 1; i > 0; i--)
	{
		if (servicePath[i] == L'\\')
		{
			printf("\r\n");
			wcsncpy(path_of_log_file, servicePath, i);
			wcscat(path_of_log_file, L"\\");
			wcscat(path_of_log_file, L"log-netline.log");

			wprintf(L"log txt is: %s[%d]\r\n", path_of_log_file, i);
			break;
		}
	}

	if (netline == nullptr)
		netline = new Netline();
	else
	{
		delete netline;
		netline = new Netline();
	}

	netline->Start(servicePath);

	//return 0;

//#ifdef DEBUG_PROC
//	cls = new COLSO();
//
//	cls->Start(servicePath);
//	cls->thrMain();
//
//	cls->lwsInit();
//
//#endif

	if (argc - 1 == 0)
	{
		addLogMessage((char*)"SERVICE_TABLE_ENTRY");

		SERVICE_TABLE_ENTRY ServiceTable[2];
		ServiceTable[0].lpServiceName = (wchar_t*)SERVICE_NAME;
		ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTIONW)MainServiceProc;// (argc, argv);
		ServiceTable[1].lpServiceName = nullptr;
		ServiceTable[1].lpServiceProc = nullptr;

		if (!StartServiceCtrlDispatcher(ServiceTable))
		{
			DWORD err = GetLastError();
			addLogMessage((char*)"Error: StartServiceCtrlDispatcher");
			printf("ERR=%d\r\n", err);
		}
	}
	else
		if (strcmp(argv[argc - 1],"install") == 0)
		{
			InstallService();
			printf("InstallService\r\n");
		}
		else
			if (strcmp(argv[argc - 1], "remove") == 0)
			{
				RemoveService();
				printf("RemoveService\r\n");
			}
			else
				if (strcmp(argv[argc - 1], "start") == 0)
				{
					StartClientService();
					printf("StartClientService\r\n");
				}

	return 0;
}

int InstallService()
{
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (!hSCManager) {
		addLogMessage((char*)"Error: Can't open Service Control Manager");
		return -1;
	}

	SC_HANDLE hService = CreateService(
		hSCManager,
		SERVICE_NAME,
		SERVICE_NAME,
		SERVICE_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS,
		SERVICE_AUTO_START,//SERVICE_DEMAND_START,
		SERVICE_ERROR_NORMAL,
		servicePath,
		NULL, NULL, NULL, NULL, NULL
	);

	//	SECURITY_DESCRIPTOR sd;
	//	sd.
	//	SetServiceObjectSecurity(hService,,);

	SERVICE_FAILURE_ACTIONS_FLAG sfaf;
	sfaf.fFailureActionsOnNonCrashFailures = true;
	ChangeServiceConfig2(hService, SERVICE_CONFIG_FAILURE_ACTIONS_FLAG, &sfaf);

	SERVICE_FAILURE_ACTIONS failAct = { 0 };
	SC_ACTION failActions[3];
	failActions[0].Delay = 1000;
	failActions[0].Type = SC_ACTION_RESTART;

	failActions[1].Delay = 1000;
	failActions[1].Type = SC_ACTION_RESTART;

	failActions[2].Delay = 1000;
	failActions[2].Type = SC_ACTION_RESTART;


	failAct.cActions = 3;
	failAct.dwResetPeriod = INFINITE;
	failAct.lpsaActions = failActions;
	failAct.lpCommand = nullptr;
	failAct.lpRebootMsg = nullptr;

	ChangeServiceConfig2(hService, SERVICE_CONFIG_FAILURE_ACTIONS, &failAct);


	if (!hService) {
		int err = GetLastError();
		switch (err) {
		case ERROR_ACCESS_DENIED:
			addLogMessage((char*)"Error: ERROR_ACCESS_DENIED");
			break;
		case ERROR_CIRCULAR_DEPENDENCY:
			addLogMessage((char*)"Error: ERROR_CIRCULAR_DEPENDENCY");
			break;
		case ERROR_DUPLICATE_SERVICE_NAME:
			addLogMessage((char*)"Error: ERROR_DUPLICATE_SERVICE_NAME");
			break;
		case ERROR_INVALID_HANDLE:
			addLogMessage((char*)"Error: ERROR_INVALID_HANDLE");
			break;
		case ERROR_INVALID_NAME:
			addLogMessage((char*)"Error: ERROR_INVALID_NAME");
			break;
		case ERROR_INVALID_PARAMETER:
			addLogMessage((char*)"Error: ERROR_INVALID_PARAMETER");
			break;
		case ERROR_INVALID_SERVICE_ACCOUNT:
			addLogMessage((char*)"Error: ERROR_INVALID_SERVICE_ACCOUNT");
			break;
		case ERROR_SERVICE_EXISTS:
			addLogMessage((char*)"Error: ERROR_SERVICE_EXISTS");
			break;
		default:
			addLogMessage((char*)"Error: Undefined");
		}
		CloseServiceHandle(hSCManager);
		return -1;
	}
	CloseServiceHandle(hService);

	CloseServiceHandle(hSCManager);
	addLogMessage((char*)"Success install service!");
	return 0;
}
int RemoveService()
{
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!hSCManager) {
		addLogMessage((char*)"Error: Can't open Service Control Manager");
		return -1;
	}
	SC_HANDLE hService = OpenService(hSCManager, SERVICE_NAME, SERVICE_STOP | DELETE);
	if (!hService) {
		addLogMessage((char*)"Error: Can't remove service");
		CloseServiceHandle(hSCManager);
		return -1;
	}

	DeleteService(hService);
	CloseServiceHandle(hService);
	CloseServiceHandle(hSCManager);
	addLogMessage((char*)"Success remove service!");
	return 0;
}
int StartClientService()
{
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	SC_HANDLE hService = OpenService(hSCManager, SERVICE_NAME, SERVICE_START);
	if (!StartService(hService, 0, NULL)) {
		CloseServiceHandle(hSCManager);
		addLogMessage((char*)"Error: Can't start service");
		return -1;
	}

	CloseServiceHandle(hService);
	CloseServiceHandle(hSCManager);
	addLogMessage((char*)"StartClientService");
	return 0;
}
void __stdcall ControlHandler(DWORD request)
{
	addLogMessage((char*)"ControlHandler.");
	switch (request)
	{
	case SERVICE_CONTROL_STOP:
		addLogMessage((char*)"Stopped.");
		printf("SERVICE_CONTROL_STOP\r\n");

		//cls->Stop();
		serviceStatus.dwWin32ExitCode = 0;
		serviceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(serviceStatusHandle, &serviceStatus);
		return;

	case SERVICE_CONTROL_SHUTDOWN:
		addLogMessage((char*)"Shutdown.");
		printf("SERVICE_CONTROL_SHUTDOWN\r\n");

		//cls->Stop();
		//cls->Start(servicePath);

		serviceStatus.dwWin32ExitCode = 0;
		serviceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(serviceStatusHandle, &serviceStatus);
		return;

	default:
		break;
	}

	SetServiceStatus(serviceStatusHandle, &serviceStatus);

	return;
}
void __stdcall MainServiceProc(DWORD argc, LPWSTR* argv)
{
	//char pp[MAX_PATH];
	//sprintf(pp,"SuperPuper argc = %d\r\n",argc);
	//addLogMessage(pp);
	//wprintf(L"argv = %s\r\n", argv[0]);

	int error;
	int i = 0;

	//addLogMessage("MainServiceProc");//ServiceMain

	serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	serviceStatus.dwCurrentState = SERVICE_START_PENDING;//SERVICE_RUNNING
	serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;// | SERVICE_ACCEPT_SHUTDOWN;
	serviceStatus.dwWin32ExitCode = 0;
	serviceStatus.dwServiceSpecificExitCode = 0;
	serviceStatus.dwCheckPoint = 0;
	serviceStatus.dwWaitHint = 0;

	LPHANDLER_FUNCTION zzz = (LPHANDLER_FUNCTION)ControlHandler;// (0);

	serviceStatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, zzz);

	if (serviceStatusHandle == (SERVICE_STATUS_HANDLE)0)
	{
		addLogMessage((char*)"serviceStatusHandle == 0");
		return;
	}

	error = 0;// InitService();
	if (error)
	{
		serviceStatus.dwCurrentState = SERVICE_STOPPED;
		serviceStatus.dwWin32ExitCode = -1;
		SetServiceStatus(serviceStatusHandle, &serviceStatus);
		addLogMessage((char*)"InitService error");
		return;
	}

	serviceStatus.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus(serviceStatusHandle, &serviceStatus);

	while (serviceStatus.dwCurrentState == SERVICE_RUNNING)
	{
		//char buffer[255];
		//sprintf_s(buffer, "%u", i);
		//int result = addLogMessage(buffer);

		int result = 0;
		//result = netline->Start(servicePath);
		if (result)
		{
			serviceStatus.dwCurrentState = SERVICE_STOPPED;
			serviceStatus.dwWin32ExitCode = -1;
			SetServiceStatus(serviceStatusHandle, &serviceStatus);
			addLogMessage((char*)"thrMain return error");
			return;
		}

		Sleep(100);
	}

	return;
}
bool filePresent()
{
	struct _stat buffer;
	int res = _wstat(path_of_log_file, &buffer);
	if (res == -1)
	{
		if (errno == ENOENT)
		{
			FILE *f = _wfopen(path_of_log_file, L"w");
			fclose(f);// _wfopen(path_of_log_file [filePresent]
			res = _wstat(path_of_log_file, &buffer);
		}
	}

	if (buffer.st_size > 1024*1024)
	{
		_wremove(path_of_log_file);
		FILE *f = _wfopen(path_of_log_file, L"w");
		fclose(f);// _wfopen(path_of_log_file [filePresent]
		res = _wstat(path_of_log_file, &buffer);
	}

	if (res == 0)
		return true;
	else
		return false;
}
int saveLog(char *p)
{
	FILE *f;

	if (filePresent())
	{
		f = _wfopen(path_of_log_file, L"at");
		size_t size = fwrite(p, 1, strlen(p), f);
		fclose(f);// _wfopen(path_of_log_file [saveLog]
	}

	return 0;
}
int addLogMessage(char* p)
{
	time_t timer;
	time(&timer);
	struct tm * ptm;
	ptm = gmtime(&timer);
	//printf("[%04d-%02d-%02d %02d:%02d:%02d]\r\n", ptm->tm_year + 1900, ptm->tm_mon, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

	sprintf(logStr, "[%04d-%02d-%02d %02d:%02d:%02d] %s[err=%d]\r\n", ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, p, GetLastError());
	saveLog(logStr);
	return 0;
};