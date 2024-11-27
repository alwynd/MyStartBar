#pragma once

#ifndef _MYTASKBAR_H_
#define _MYTASKBAR_H_


#include <windows.h>
#include <tchar.h>
#include <iostream>

#include <fstream>
#include <ctime>
#include <mutex>

#include <stdio.h>
#include <time.h>
#include <vector>
#include <map>
#include <string>
#include <algorithm>

#include <process.h>    /* _beginthread, _endthread */
#include <cstring> // For strcasecmp



#include <Psapi.h>
#pragma comment(lib, "Kernel32.lib")
#pragma comment(lib, "Psapi.lib")

#ifndef DLLEXPORT
#define DLLEXPORT __declspec(dllexport)
#endif

#ifndef DLLIMPORT
#define DLLIMPORT __declspec(dllimport)
#endif

#define BOOL_TEXT(x) (x ? "true" : "false")

using namespace std;

const long long CurrentTimeMS();							// Get current ms
void FormatDate(char* buffer);								// Format date
void debug(const char* msg);								// Debug string to console


BOOL CALLBACK EnumWindowCallback(HWND hWnd, LPARAM lparam);	// Enum window callback


// A running process.
struct RunningProcess
{
	RunningProcess()
	{
		WindowHandle = NULL;
		ZeroMemory(WindowTitle, sizeof(char) * 1024);
		ZeroMemory(ImageName, sizeof(char) * 1024);
		IsMinimized = false;
	};

	bool operator < (const RunningProcess& a) const
	{
		std::string aWindowTitle(WindowTitle);
		std::string bWindowTitle(a.WindowTitle);

		return (aWindowTitle < bWindowTitle);
	}


	HWND WindowHandle;
	char WindowTitle[1024];
	char ImageName[1024];
	bool IsMinimized;
};


bool FileExists(const char* file);								// Does file exist.
void split(const char* splitString, const char splitChar, std::vector<string>& strings);
bool startswith(const char* string, const char* search);	// Startswith string
bool endswith(const char* string, const char* search);		// Endswith string
bool equals(const char* string1, const char* string2);		// is equal (CI)?





#endif