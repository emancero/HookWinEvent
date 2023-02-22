#include <windows.h>
#include <cstdio>
void Log(LPCSTR file, LPCSTR message) {
    FILE* f;
    fopen_s(&f, file, "a");
    if (f != 0) {
        fprintf(f, "%Ts", message);
        fclose(f);
    }
}