#ifndef __UTILS_H__
#define __UTULS_H__

#include <stdint.h>
#include <sys/types.h>
#include <stdarg.h>

#ifdef WIN32
#include <windows.h>
#include <winbase.h>
#else
#include <pthread.h>
#endif

void l_error (const char* format, ...);
void* l_malloc (uint32_t bytes);
int l_open (const char *filename);
void l_close (int fd);
int l_read (int filedes, void *buffer, uint32_t size);
int l_write (int filedes, const void *buffer, uint32_t size);
void set_term (int fd, int rtscts_flow_control_enable);


typedef struct thread
{
#ifdef WIN32
        HANDLE thread;
#else
        pthread_t thread;
#endif
} thread_t;

typedef struct mutex
{
#ifdef WIN32
        CRITICAL_SECTION crit_section;
        int mutex;
#else
        pthread_mutex_t mutex;
#endif
} mutex_t;

typedef struct cond
{
#ifdef WIN32
        HANDLE event;
#else
        pthread_mutex_t mutex;
        pthread_cond_t cond;
#endif
} cond_t;


int thread_start (thread_t* thread, void (*start_routine) (void));
void thread_join (thread_t* thread);

void mutex_init (mutex_t* mutex);
void mutex_lock (mutex_t* mutex);
void mutex_unlock (mutex_t* mutex);

void cond_init (cond_t* cond);
void cond_wait (cond_t* cond);
void cond_notify (cond_t* cond);

typedef struct timestamp {
        uint8_t h;
        uint8_t m;
        uint8_t s;
        uint16_t ms;
} tymestamp_t;

void timer_restart (uint32_t count_ms);
uint32_t timer_elapsed_ms (void);
int timer_expired (void);
tymestamp_t timer_timestamp (void);


#endif
