// HookWinEvent.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "HookWinEvent.h"
#include "Timer.h"
#include "Log.h"
#include <OleAcc.h>
#include <stdio.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


// Global variable.
HWINEVENTHOOK g_hook;
ULONGLONG lastActiveTick;
const int WINDOWTITLE_LENGTH =200;
TCHAR windowTitle[WINDOWTITLE_LENGTH]= _T("SICAV - TEST CASA DE VALORES (Remote)");
DWORD pid;
// Callback function that handles events.
//
void CALLBACK HandleWinEvent(HWINEVENTHOOK hook, DWORD event, HWND hwnd,
    LONG idObject, LONG idChild,
    DWORD dwEventThread, DWORD dwmsEventTime)
{

    IAccessible* pAcc = NULL;
    //VARIANT varChild;
    /*if (idObject != OBJID_CURSOR)
        return;*/
    HWND hwndFore=GetForegroundWindow();
    TCHAR title[200];
    int len = GetWindowText(hwndFore,title,200);
    
    DWORD nowPid;
    GetWindowThreadProcessId(hwndFore, &nowPid);
    if (len > 0 && _tcscmp(title, windowTitle) == 0)
        pid=nowPid;

    if(nowPid==pid)
    {
        lastActiveTick = GetTickCount64();
        char buffer[500];
        sprintf_s(buffer, "%lu", event);
        OutputDebugStringA(buffer);
        /*FILE* f;
        fopen_s(&f, "msgs.txt", "a");
        if (f != 0) {
            fprintf(f, "%lu ", event);
            fclose(f);
        }*/
        /*FILE* f;
        fopen_s(&f, "log.txt", "a");
        if (f != 0) {
            fwprintf(f, L"Active");
            fclose(f);
        }*/
    }
    else
        OutputDebugStringA("o");
    /*HRESULT hr = AccessibleObjectFromEvent(hwnd, idObject, idChild, &pAcc, &varChild);
    if ((hr == S_OK) && (pAcc != NULL))
    {
        BSTR bstrName;
        pAcc->get_accName(varChild, &bstrName);
        if (event > 0)
        {
            //printf("%d,", event);
        }
        //printf("%S\n", bstrName);
        SysFreeString(bstrName);
        pAcc->Release();
    }*/
}
HWND tgtHwnd;
CHAR IniFile[100] = "hook.ini";

void ReadToNewLine(TCHAR* str)
{
    for (int chi = 0; str[chi]; chi++)
        if (str[chi] == _T('\r') || str[chi] == _T('\n'))
            str[chi] = 0;
}

// Initializes COM and sets up the event hook.
//
void InitializeMSAA()
{
    HRESULT hr = CoInitialize(NULL);

    
    TCHAR temp[500];
    FILE* fIni;
    fopen_s(&fIni, IniFile, "r");
    if (fIni != 0) {
        _fgetts(temp,500,fIni);
        _fgetts(temp,500,fIni);
        _fgetts(windowTitle,WINDOWTITLE_LENGTH,fIni);
        fclose(fIni);
    }
    ReadToNewLine(windowTitle);

    HWND tgtHwnd = FindWindow(NULL, windowTitle);
    unsigned long pid;
    DWORD tid = GetWindowThreadProcessId(tgtHwnd, &pid);
    g_hook = SetWinEventHook(
        8, 9,  // Range of events (4 to 5).
        NULL,                                          // Handle to DLL.
        HandleWinEvent,                                // The callback.
        0, 0,              // Process and thread IDs of interest (0 = all)
        WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS); // Flags.
    FILE* f;
    errno_t err = fopen_s(&f, "log.txt", "a");
    if (f != 0) {
        fprintf(f, "afterhook %d\n",g_hook!=0?1:0);
        fclose(f);
    }

}

// Unhooks the event and shuts down COM.
//
void ShutdownMSAA()
{
    UnhookWinEvent(g_hook);
    CoUninitialize();
}

const UINT_PTR IDT_TIMER = 0x1;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    // TODO: Place code here.
    DeleteFile(_T("log.txt"));
    WCHAR dir[500];
    GetCurrentDirectory(500, dir);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_HOOKWINEVENT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_HOOKWINEVENT));

    MSG msg;
    lastActiveTick = 0;
    InitializeMSAA();
    UINT_PTR timer = SetTimer(0, IDT_TIMER, 1000, (TIMERPROC)MyTimerProc);

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (msg.message == WM_USER) {
            bool bTimerKilled = KillTimer(0, IDT_TIMER);
            ShutdownMSAA();
            PostQuitMessage(0);
        }
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HOOKWINEVENT));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_HOOKWINEVENT);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}


HWND hMainWnd;
//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   hMainWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hMainWnd)
   {
      return FALSE;
   }

   //ShowWindow(hWnd, nCmdShow);
   //UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        ShutdownMSAA();
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
