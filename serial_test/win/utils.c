
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <stdarg.h>

#include "../utils.h"
#include "../logger.h"

#define OPEN_FLAGS (O_RDWR)

void l_error (const char* format, ...)
{
	int err = errno;
	const char* errstr = strerror(err);
	va_list ap;
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
	fprintf (stderr, "ERROR: %d - %s\n", err, errstr);
}

void* l_malloc (uint32_t bytes)
{
	void* buffer = malloc(bytes);
	if (!buffer)
		l_error("malloc : can't allocate %d bytes\n", bytes);
	return buffer;
}

int l_open (const char *filename)
{
        char realfilename[128];
        snprintf(realfilename, sizeof(realfilename), "\\\\.\\%s", filename);

        printf("CreateFile(%s, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL)", realfilename);

        int hSerial = (int)CreateFile(realfilename, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        // FILE_FLAG_NO_BUFFERING

        printf(" = %d\n", hSerial);

        if (hSerial == (int)INVALID_HANDLE_VALUE) {
               l_error("open : Unable to open file \"%s\"\n", realfilename);
               return (int)INVALID_HANDLE_VALUE;
        }

        ClearCommError((HANDLE)hSerial, NULL, NULL);

	return hSerial;
}

void l_close (int fd)
{
        if (fd == (int)INVALID_HANDLE_VALUE)
            return;

        // ClearCommError(hSerial, NULL,NULL);

        printf("CloseHandle(%d)\n", fd);

        CloseHandle((HANDLE)fd);
}

#define PRINT_ARR(_CALL_, _BUFF_, _TAB_) _CALL_("%s%s0x%02X", (i ? ", " : ""), (!i || i % 8 ? "" : "\n" _TAB_), (_BUFF_)[i])

int l_read (int filedes, void *buffer, uint32_t size)
{
        uint32_t dwNumberOfBytesToRead;

        if (!ReadFile((HANDLE)filedes, buffer, size, (LPDWORD)&dwNumberOfBytesToRead, NULL)) {
                return -1;
        } else {
                return dwNumberOfBytesToRead;
        }
}

int l_write (int filedes, const void *buffer, uint32_t size)
{
        uint32_t dwNumberOfBytesToWrite;

        if(!WriteFile((HANDLE)filedes, buffer, size, (LPDWORD)&dwNumberOfBytesToWrite, NULL)) {
                return -1;
        } else {
                return dwNumberOfBytesToWrite;
        }
}


void set_term (int fd, int rtscts_flow_control_enable)
{
        DCB dcb = {0};
        dcb.DCBlength = sizeof(DCB);

        if (!GetCommState((HANDLE)fd, &dcb)) {
                printf("GetCommState() FAILED\n");
                return;
        }

        dcb.BaudRate  = CBR_115200;
        dcb.ByteSize  = DATABITS_8;
        dcb.Parity    = NOPARITY;
        dcb.StopBits  = ONESTOPBIT;
        dcb.fBinary   = 1;

        if (rtscts_flow_control_enable) {
                dcb.fOutxCtsFlow = 1;
                dcb.fRtsControl = RTS_CONTROL_ENABLE;
        }

        if (!SetCommState((HANDLE)fd, &dcb)) {
                printf("SetCommState() FAILED\n");
        }
}



int thread_start (thread_t* thread, void (*start_routine) (void))
{
        unsigned long ThreadID;

        thread->thread = CreateThread(
                     NULL,       // default security attributes
                     0,          // default stack size
                     (LPTHREAD_START_ROUTINE) start_routine,
                     0,       // no thread function arguments
                     0,          // default creation flags
                     &ThreadID);

        return !thread->thread;
}

void thread_join (thread_t* thread)
{
        WaitForSingleObject(thread->thread, INFINITE);
        CloseHandle(thread->thread);
}

void mutex_init (mutex_t* mutex)
{
        InitializeCriticalSectionAndSpinCount(&mutex->crit_section, 10);
        mutex->mutex = 1;
}

void mutex_lock (mutex_t* mutex)
{
        while (1) {
                EnterCriticalSection(&mutex->crit_section);
                if (mutex->mutex) {
                        mutex->mutex = 0;
                        LeaveCriticalSection(&mutex->crit_section);
                        break;
                }
                LeaveCriticalSection(&mutex->crit_section);
                Sleep(1);
        }
}

void mutex_unlock (mutex_t* mutex)
{
        EnterCriticalSection(&mutex->crit_section);
        mutex->mutex = 1;
        LeaveCriticalSection(&mutex->crit_section);
}

void cond_init (cond_t* cond)
{
        cond->event = CreateEvent(
                NULL,               // default security attributes
                TRUE,               // manual-reset event
                FALSE,              // initial state is nonsignaled
                NULL              // unnamed event
                );
}

void cond_wait (cond_t* cond)
{
        while (1) {
                uint32_t result = WaitForSingleObject(cond->event, 100);

                if (result == WAIT_OBJECT_0) {
                        ResetEvent(cond->event);
                        break;
                }
        }
}

void cond_notify (cond_t* cond)
{
        SetEvent(cond->event);

}

FILETIME _timer_start;
FILETIME _timer_expire;

void timer_restart (uint32_t count_ms)
{
        SYSTEMTIME timer_start;

        GetLocalTime(&timer_start);

        SystemTimeToFileTime(&timer_start, &_timer_start);

        ULONGLONG now_64 = (((ULONGLONG) _timer_start.dwHighDateTime) << 32) + _timer_start.dwLowDateTime;

        now_64 += 10000 * count_ms;

        _timer_expire.dwLowDateTime  = (DWORD) (now_64 & 0xFFFFFFFF );
        _timer_expire.dwHighDateTime = (DWORD) (now_64 >> 32 );
}

uint32_t timer_elapsed_ms (void)
{
        SYSTEMTIME now;

        GetLocalTime(&now);

        FILETIME _now;

        SystemTimeToFileTime(&now, &_now);

        ULONGLONG now_64 = (((ULONGLONG) _now.dwHighDateTime) << 32) + _now.dwLowDateTime;
        ULONGLONG timer_start_64 = (((ULONGLONG) _timer_start.dwHighDateTime) << 32) + _timer_start.dwLowDateTime;

        ULONGLONG elapsed = now_64 - timer_start_64;

        return elapsed/10000;
}

int timer_expired (void)
{
        SYSTEMTIME now;

        GetLocalTime(&now);

        FILETIME _now;

        SystemTimeToFileTime(&now, &_now);

        if (CompareFileTime(&_now, &_timer_expire) >= 0) { // > 0 means that _now is later than _timer_expire
                return 1;
        } else {
                return 0;
        }
}

tymestamp_t timer_timestamp (void)
{
        tymestamp_t result;
        SYSTEMTIME now;

        GetLocalTime(&now);



        return  result;
}



