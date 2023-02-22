#include <Windows.h>
#include "HttpRequest.h"
#include "HookWinEvent.h"
#include "stdio.h"
POINT ptOld;
const int TMS_TERMINATE = 0x1;
DWORD timerMessage=0x0;
VOID CALLBACK MyTimerProc(
    HWND hwnd,        // handle to window for timer messages 
    UINT message,     // WM_TIMER message 
    UINT idTimer,     // timer identifier 
    DWORD dwTime)     // current system time 
{
    ULONGLONG nowTick = GetTickCount64();
    ULONGLONG lapse = nowTick - lastActiveTick;
    if (lapse > 1000 && lapse < 5000) {
        SendHttpRequest();
        lastActiveTick -= 5000;
    }

    //monitor terminate signal
    FILE* f;
    fopen_s(&f, "kill", "r");
    if (f != 0) {
        DWORD curThreadId;

        curThreadId = GetCurrentThreadId();

        // Send messages to self:
        PostThreadMessage(curThreadId, WM_USER, 1, 2);
    }

    /*RECT rc;
    POINT pt;

    // If the window is minimized, compare the current 
    // cursor position with the one from 10 seconds earlier. 
    // If the cursor position has not changed, move the 
    // cursor to the icon. 

    if (IsIconic(hwnd))
    {
        GetCursorPos(&pt);

        if ((pt.x == ptOld.x) && (pt.y == ptOld.y))
        {
            GetWindowRect(hwnd, &rc);
            SetCursorPos(rc.left, rc.top);
        }
        else
        {
            ptOld.x = pt.x;
            ptOld.y = pt.y;
        }
    }*/
}