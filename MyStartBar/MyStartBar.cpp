#include "MyStartBar.h"

std::vector<RunningProcess> Processes;

bool running = false;


// Enum window callback
BOOL CALLBACK EnumWindowCallback(HWND hWnd, LPARAM lparam)
{
	if (!lparam)
	{
		debug("Mytaskbar.EnumWindowCallback ERROR : lparam is INVALID.");
		return TRUE;
	}
	std::vector<RunningProcess>* processes = (std::vector<RunningProcess>*) lparam;

	int length = GetWindowTextLength(hWnd);
	char buffer[1024];
	ZeroMemory(buffer, 1024);
	DWORD dwProcId = 0;

	// List visible windows with a non-empty title
	if (IsWindowVisible(hWnd) && length != 0)
	{
		RunningProcess process;

		process.WindowHandle = hWnd;
		int wlen = length + 1;
		GetWindowText(hWnd, process.WindowTitle, wlen);
		bool add = true;

		GetWindowThreadProcessId(hWnd, &dwProcId);
		if (dwProcId)
		{
			HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcId);
			if (hProc)
			{
				HMODULE hMod;
				DWORD cbNeeded;
				process.IsMinimized = IsIconic(hWnd);
				if (EnumProcessModules(hProc, &hMod, sizeof(hMod), &cbNeeded))
				{
					GetModuleBaseNameA(hProc, hMod, process.ImageName, MAX_PATH);
				} //if
				CloseHandle(hProc);

				if (equals(process.ImageName, "SystemSettings.exe"))
				{
					add = false;
				}
				if (equals(process.ImageName, "ApplicationFrameHost.exe"))
				{
					add = false;
				}
				if (equals(process.ImageName, "TextInputHost.exe"))
				{
					add = false;
				}
			} //if
		} //if

		if (equals(process.WindowTitle, "Program Manager"))
		{
			add = false;
		}
		if (endswith(process.WindowTitle, " Extreme Tuning Utility"))
		{
			add = false;
		}
		if (add)
		{
			processes->push_back(process);		// COPY, this is NOT BY REF
			sprintf_s(buffer, "Mytaskbar.EnumWindowCallback, Window: hwnd: %p, Title: '%-55s', Executable: '%-55s', Min: '%-5s'\0", process.WindowHandle, process.WindowTitle, process.ImageName, BOOL_TEXT(process.IsMinimized)); debug(buffer);
		}
		else
		{
			sprintf_s(buffer, "Mytaskbar.EnumWindowCallback, Window: hwnd: %p, Title: '%-55s', IGNORE====: '%-55s', Min: '%-5s'\0", process.WindowHandle, process.WindowTitle, process.ImageName, BOOL_TEXT(process.IsMinimized)); debug(buffer);
		}

	}

	return TRUE;
}


void SortProcesses() {
	std::vector<std::string> endOrder = {
		"devenv.exe",
		"unity.exe",
		"p4v.exe",
		"forticlient.exe",
		"mstsc.exe",
		"chrome.exe",
		"notepad.exe",
		"explorer.exe",
	};

	auto comparator = [&endOrder](const RunningProcess& a, const RunningProcess& b) {
		// Convert ImageName to std::string for easier comparison
		std::string nameA(a.ImageName);
		std::string nameB(b.ImageName);

		// Convert to lowercase using _stricmp
		if (_stricmp(nameA.c_str(), "cmd.exe") == 0 && _stricmp(nameB.c_str(), "cmd.exe") != 0) return true;
		if (_stricmp(nameB.c_str(), "cmd.exe") == 0 && _stricmp(nameA.c_str(), "cmd.exe") != 0) return false;

		if (_stricmp(nameA.c_str(), "windowsterminal.exe") == 0 && _stricmp(nameB.c_str(), "windowsterminal.exe") != 0) return true;
		if (_stricmp(nameB.c_str(), "windowsterminal.exe") == 0 && _stricmp(nameA.c_str(), "windowsterminal.exe") != 0) return false;

		if (_stricmp(nameA.c_str(), "mintty.exe") == 0 &&
			_stricmp(nameB.c_str(), "cmd.exe") != 0 &&
			_stricmp(nameB.c_str(), "mintty.exe") != 0) return true;
		if (_stricmp(nameB.c_str(), "mintty.exe") == 0 &&
			_stricmp(nameA.c_str(), "cmd.exe") != 0 &&
			_stricmp(nameA.c_str(), "mintty.exe") != 0) return false;

		// Handle the end-specific EXEs
		auto posA = std::find_if(endOrder.rbegin(), endOrder.rend(),
			[&nameA](const std::string& endName) {
				return _stricmp(nameA.c_str(), endName.c_str()) == 0;
			});
		auto posB = std::find_if(endOrder.rbegin(), endOrder.rend(),
			[&nameB](const std::string& endName) {
				return _stricmp(nameB.c_str(), endName.c_str()) == 0;
			});

		if (posA != endOrder.rend() && posB != endOrder.rend()) {
			return posA < posB;
		}
		else if (posA != endOrder.rend()) {
			return false;
		}
		else if (posB != endOrder.rend()) {
			return true;
		}

		return false;
		};


	// Perform stable sort
	std::stable_sort(Processes.begin(), Processes.end(), comparator);
}

void ReorderTaskbarIcons(void* parm)
{
	char buffer[1024];
	ZeroMemory(buffer, 1024);

	sprintf_s(buffer, "Mytaskbar.ReorderTaskbarIcons Processes: %zi\0", Processes.size()); debug(buffer);

	SortProcesses();
	size_t i = 0;

	// hide all the windows.
	for (const RunningProcess& process : Processes)
	{
		sprintf_s(buffer, "Mytaskbar.ReorderTaskbarIcons !! HIDE !! process: %-3zi of %-3zi: '%s'\0", i, Processes.size(), process.ImageName); debug(buffer);
		i++;
		ShowWindow(process.WindowHandle, SW_HIDE);
		Sleep(500);
		Yield();
	}

	debug("Waiting for a short time...");
	Sleep(1000);
	Yield();

	// enable them in order.
	for (const RunningProcess& process : Processes)
	{
		sprintf_s(buffer, "Mytaskbar.ReorderTaskbarIcons !! SHOW !! process: %-3zi of %-3zi: '%s'\0", i, Processes.size(), process.ImageName); debug(buffer);
		i++;
		ShowWindow(process.WindowHandle, SW_SHOW);
		Sleep(500);
		Yield();
	}

	Sleep(1000);
	Yield();

	running = false;

	_endthread();
}


// Win main!
int WINAPI WinMain(HINSTANCE inst, HINSTANCE prev, LPSTR cmd, int show)
{
	char buffer[1024];
	ZeroMemory(buffer, 1024);
	int rc = 0;

	Processes.clear();
	EnumWindows(EnumWindowCallback, (LPARAM)&Processes);

	HWND hWndTarget = FindWindow("TaskManagerWindow", NULL);
	if (hWndTarget)
	{
		RunningProcess process;
		process.WindowHandle = hWndTarget;
		strcpy_s(process.WindowTitle, "TaskManagerWindow");
		Processes.push_back(process);
	}

	sprintf_s(buffer, "Mytaskbar.WinMain \t Called EnumWindows..., Processes: %I64i\0", Processes.size()); debug(buffer);
	if (Processes.size() < 1)
	{
		debug("Mytaskbar.WinMain ERROR : Could not enumerate any windows.");
		rc = 99;
		goto DONE;
	} //if

	sprintf_s(buffer, "MyWorkspace.WinMain \t Sorting Processes: %I64i\0", Processes.size()); debug(buffer);
	std::sort(std::begin(Processes), std::end(Processes));

	for (int i = 0; i < Processes.size(); i++)
	{
		const RunningProcess& process = Processes[i];
		sprintf_s(buffer, "MyWorkspace.WinMain \t Sorted Processes: %i, process: '%s'\0", i, process.WindowTitle); debug(buffer);
	} //for

	running = true;
	_beginthread(ReorderTaskbarIcons, 0, NULL);

	while (running)
	{
		Sleep(100);
		Yield();
	}

DONE:
	debug("Mytaskbar.WinMain:-- DONE");
	return rc;
}



// Does file exist.
bool FileExists(const char* file)
{
	FILE* f = NULL;
	fopen_s(&f, file, "rb");
	if (f)
	{
		fclose(f);
		return  true;
	} //if

	return false;
}

// Get current ms
const long long CurrentTimeMS()
{
	chrono::system_clock::time_point currentTime = chrono::system_clock::now();

	long long transformed = currentTime.time_since_epoch().count() / 1000000;
	long long millis = transformed % 1000;

	return millis;
}





// Format date
void FormatDate(char* buffer)
{
	buffer[0] = '\0';
	time_t timer;
	tm tm_info;

	timer = time(NULL);
	localtime_s(&tm_info, &timer);

	strftime(buffer, 26, "%Y-%m-%d %H:%M:%S\0", &tm_info);
}





// Debug string to console
void debug(const char* msg)
{
	char buffer[27];
	ZeroMemory(buffer, 27);
	FormatDate(buffer);

	printf("%s - %s\n", buffer, msg); fflush(stdout);

}



void split(const char* splitString, const char splitChar, std::vector<string>& strings)
{

	char logbuffer[1024];
	ZeroMemory(logbuffer, 1024);

	size_t bufferidx = 0;
	char buffer[1024];
	ZeroMemory(buffer, 1024);

	int len = (int)strnlen_s(splitString, 1024);
	for (int i = 0; i < len; i++)
	{
		if (splitString[i] == splitChar)
		{
			//sprintf_s(logbuffer, "Split \t : Adding: '%s'\0", buffer); debug(logbuffer);
			if (strnlen_s(buffer, 1024) > 0) strings.push_back(string(buffer));
			bufferidx = 0;
			ZeroMemory(buffer, 1024);
			continue;
		}
		buffer[bufferidx++] = splitString[i];
	}
	if (strnlen_s(buffer, 1024) > 0) strings.push_back(string(buffer));

}



// Endswith string
bool endswith(const char* string, const char* search)
{
	char logbuffer[1024];
	ZeroMemory(logbuffer, 1024);

	int stringlen = (int)strnlen_s(string, 1024);
	int searchlen = (int)strnlen_s(search, 1024);
	//sprintf_s(logbuffer, "endswith: \t stringlen: %i, searchlen: %i\0", stringlen, searchlen); debug(logbuffer);

	if (stringlen < searchlen) return false;
	const char* buffer = string + (stringlen - searchlen);

	bool match = strcmp(buffer, search) == 0;
	//sprintf_s(logbuffer, "endswith: \t\t search len: %i, buffer: '%s', match: '%s'\0", searchlen, buffer, BOOL_TEXT(match)); debug(logbuffer);


	return match;
}




// Starts with string
bool startswith(const char* string, const char* search)
{
	char logbuffer[1024];
	ZeroMemory(logbuffer, 1024);

	char buffer[1024];
	ZeroMemory(buffer, 1024);

	// copy string, into buffer, for length of search
	int len = (int)strnlen_s(search, 1024);
	//sprintf_s(logbuffer, "startswith: \t search len: %i\0", len); debug(logbuffer);

	CopyMemory(buffer, string, len);
	//sprintf_s(logbuffer, "startswith: \t search len: %i, buffer: '%s'\0", len, buffer); debug(logbuffer);

	bool match = strcmp(buffer, search) == 0;
	//sprintf_s(logbuffer, "startswith: \t\t search len: %i, buffer: '%s', match: '%s'\0", len, buffer, BOOL_TEXT(match)); debug(logbuffer);

	return match;
}



// is equal (CI)?
bool equals(const char* string1, const char* string2)
{
	char buffer[1024];
	ZeroMemory(buffer, 1024);

	char str1[1024];
	char str2[1024];
	ZeroMemory(str1, 1024);
	ZeroMemory(str2, 1024);

	int len = (int)strnlen_s(string1, 1024);
	int len2 = (int)strnlen_s(string2, 1024);

	if (len != len2) return false;

	CopyMemory(str1, string1, len);
	CopyMemory(str2, string2, len2);

	_strlwr_s(str1, len + 1);
	_strlwr_s(str2, len2 + 1);

	bool match = strcmp(str1, str2) == 0;
	//sprintf_s(buffer, "equals: \t\t search len: %i, str1: '%s', str2: '%s', match: '%s'\0", len, str1, str2, BOOL_TEXT(match)); debug(buffer);

	return match;

}
