/*
	SVNService
	Written by Magnus Norddahl

	This code is in the public domain and is provided as-is without
	warranty of any kind, either expressed or implied, including but not
	limited to killing your cat or burning down your PC.
*/

/*
  Adapted for ocaml by ygrek, (c) 2009
*/

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <assert.h>
#include <caml/callback.h>
#include <caml/alloc.h>

#include "service.h"

void call_service_run(void)
{
  static value* closure_f = NULL;
  if (closure_f == NULL)
  {
     closure_f = caml_named_value("service.run");
     assert(NULL != closure_f);
  }
  caml_callback_exn(*closure_f, Val_unit);
}

void call_service_stop(void)
{
  static value* closure_f = NULL;
  if (closure_f == NULL)
  {
     closure_f = caml_named_value("service.stop");
     assert(NULL != closure_f);
  }
  caml_callback_exn(*closure_f, Val_unit);
}

//#pragma comment(lib,"advapi32.lib")

int install_service(int argc, TCHAR **argv);
int remove_service();
int debug_service(int argc, TCHAR **argv);
int run_service(int argc, TCHAR **argv);
void stop_service();
BOOL WINAPI console_ctrl_handler(DWORD ctrl_type);
void service_main(DWORD argc, TCHAR **argv);
BOOL report_status(DWORD current_state, DWORD win32_exitcode, DWORD wait_hint);
void WINAPI service_ctrl_handler(DWORD ctrl_code);

static SERVICE_STATUS service_status;
static SERVICE_STATUS_HANDLE handle_service_status = 0;
static int check_point = 1;
static BOOL debug_mode = FALSE;

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc >= 2 && _tcsicmp(TEXT("-debug"), argv[1]) == 0)
	{
		return debug_service(argc+2, argv+2);
	}
	else if (argc >= 2 && (_tcsicmp(TEXT("-install"), argv[1]) == 0 || _tcsicmp(TEXT("/Install"), argv[1]) == 0))
	{
		return install_service(argc-2, argv+2);
	}
	else if (argc == 2 && (_tcsicmp(TEXT("-remove"), argv[1]) == 0 || _tcsicmp(TEXT("/Uninstall"), argv[1]) == 0 || _tcsicmp(TEXT("/Rollback"), argv[1]) == 0))
	{
		return remove_service();
	}
	else
	{
		_tprintf(TEXT("Service parameters:\r\n"));
		_tprintf(TEXT("  -install                     to install the service\r\n"));
		_tprintf(TEXT("  -remove                      to remove the service\r\n"));
		_tprintf(TEXT("  -debug                       to run as a console application for debugging\r\n"));
		return run_service(argc, argv);
	}
}

int install_service(int argc, TCHAR **argv)
{
	TCHAR exe_filename[MAX_PATH];
	SC_HANDLE handle_manager;
	SC_HANDLE handle_service;
	BOOL result;
  SERVICE_DESCRIPTION description;

	result = GetModuleFileName(0, exe_filename, MAX_PATH);
	if (result == FALSE)
	{
		_tprintf(TEXT("Could not get install service.  Unable to retrieve executable path.\r\n"));
		return 1;
	}
	handle_manager = OpenSCManager(0, 0, SC_MANAGER_ALL_ACCESS);
	if (handle_manager == 0)
	{
		_tprintf(TEXT("Could not get install service.  Failed to open service control manager.\r\n"));
		return 2;
	}
	handle_service = CreateService(
		handle_manager, SVNSERVICE_NAME, SVNSERVICE_DISPLAY_NAME, SERVICE_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START,
		SERVICE_ERROR_NORMAL, exe_filename, 0, 0, 0, 0, 0);
	if (handle_service == 0)
	{
		CloseServiceHandle(handle_manager);
		_tprintf(TEXT("Could not create service in service control manager.\r\n"));
		return 3;
	}

	description.lpDescription = SVNSERVICE_DESCRIPTION;
	result = ChangeServiceConfig2(handle_service, SERVICE_CONFIG_DESCRIPTION, &description);

	CloseServiceHandle(handle_service);
	CloseServiceHandle(handle_manager);
	_tprintf(TEXT("%s installed.\r\n"), SVNSERVICE_NAME);
  return 0;
}

int remove_service()
{
	SC_HANDLE handle_manager;
	SC_HANDLE handle_service;
	SERVICE_STATUS status;

	handle_manager = OpenSCManager(0, 0, SC_MANAGER_ALL_ACCESS);
	if (handle_manager == 0)
	{
		_tprintf(TEXT("Could not remove service.  Failed to open service control manager.\r\n"));
		return 1;
	}
	handle_service = OpenService(handle_manager, SVNSERVICE_NAME, SERVICE_ALL_ACCESS);
	if (handle_service == 0)
	{
		CloseServiceHandle(handle_manager);
		_tprintf(TEXT("Could not open service in service control manager.\r\n"));
		return 2;
	}
	memset(&status, 0, sizeof(SERVICE_STATUS));
	if (ControlService(handle_service, SERVICE_CONTROL_STOP, &status))
	{
		_tprintf(TEXT("Stopping %s.."), SVNSERVICE_NAME);
		Sleep(1000);
		while (QueryServiceStatus(handle_service, &status))
		{
			if (status.dwCurrentState != SERVICE_STOP_PENDING)
				break;
			_tprintf(TEXT("."));
			Sleep(1000);
		}
		_tprintf(TEXT("\r\n"));
		if (status.dwCurrentState == SERVICE_STOPPED)
			_tprintf(TEXT("%s stopped.\r\n"), SVNSERVICE_NAME);
		else
			_tprintf(TEXT("%s failed to stop.\r\n"), SVNSERVICE_NAME);
	}
	if (DeleteService(handle_service))
		_tprintf(TEXT("%s removed.\r\n"), SVNSERVICE_NAME);
	else
		_tprintf(TEXT("Unable to remove service.  DeleteService failed.\r\n"));
	CloseServiceHandle(handle_service);
	CloseServiceHandle(handle_manager);
	return 0;
}

int debug_service(int argc, TCHAR **argv)
{
    debug_mode = TRUE;
	SetConsoleCtrlHandler(console_ctrl_handler, TRUE);
	service_main((DWORD) argc, argv);
	return 0;
}

int run_service(int argc, TCHAR **argv)
{
	SERVICE_TABLE_ENTRY dispatch_table[] =
	{
		{ SVNSERVICE_NAME, (LPSERVICE_MAIN_FUNCTION) service_main },
		{ 0, 0 }
	};

	BOOL start_service = StartServiceCtrlDispatcher(dispatch_table);
	if (start_service == FALSE)
		return 1;
	return 0;
}

BOOL WINAPI console_ctrl_handler(DWORD ctrl_type)
{
	// We use Ctrl+C or Ctrl+Break to simulate SERVICE_CONTROL_STOP in
	// debug mode.
	switch (ctrl_type)
	{
	case CTRL_BREAK_EVENT:
	case CTRL_C_EVENT:
		stop_service();
		return TRUE;
	}
	return FALSE;
}

void service_main(DWORD argc, TCHAR **argv)
{
    char* z = NULL;

    memset(&service_status, 0, sizeof(SERVICE_STATUS));
    service_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    service_status.dwServiceSpecificExitCode = 0;
    if (debug_mode == FALSE)
      handle_service_status = RegisterServiceCtrlHandler(SVNSERVICE_NAME, service_ctrl_handler);

    report_status(SERVICE_START_PENDING, NO_ERROR, 2000);

    caml_startup(&z);

    report_status(SERVICE_RUNNING, NO_ERROR, 0);

    call_service_run();

    report_status(SERVICE_STOPPED, NO_ERROR, 2000);
}

void stop_service()
{
	if (debug_mode == TRUE)
		_tprintf(TEXT("Stopping service\r\n"));

	report_status(SERVICE_STOP_PENDING, NO_ERROR, 1000);

  call_service_stop();
}

BOOL report_status(DWORD current_state, DWORD win32_exitcode, DWORD wait_hint)
{
	if (debug_mode == TRUE)
		return TRUE;
	if (current_state != SERVICE_START_PENDING)
		service_status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	service_status.dwCurrentState = current_state;
	service_status.dwWin32ExitCode = win32_exitcode;
	service_status.dwWaitHint = wait_hint;
	switch (current_state)
	{
	case SERVICE_RUNNING:
	case SERVICE_STOPPED:
		break;
	default:
		service_status.dwCheckPoint = check_point++;
	}
	return SetServiceStatus(handle_service_status, &service_status);
}

void WINAPI service_ctrl_handler(DWORD ctrl_code)
{
	if (ctrl_code == SERVICE_CONTROL_STOP)
	{
		stop_service();
	}
	else
	{
		report_status(service_status.dwCurrentState, NO_ERROR, 0);
	}
}
