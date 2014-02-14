
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <stdarg.h>
#include <termios.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>

#include "../utils.h"
#include "../logger.h"

#define PRINT_FLAG(_VALUE_, _FLAG_) {int _T_ = 0; if ((_VALUE_) & (_FLAG_)) printf("%s "#_FLAG_" ", ((_T_)++ ? "|" : "")); }
#define PRINT_FLAG_W_MASK(_VALUE_, _MASK_, _FLAG_) {int _T_ = 0; if (((_VALUE_) & (_MASK_)) == _FLAG_) printf("%s "#_FLAG_" ", ((_T_)++ ? "|" : "")); }

// #define OPEN_FLAGS (O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK)
#define OPEN_FLAGS (O_RDWR | O_NOCTTY)

static void print_file_flags (int flags)
{
	printf(" 0x%X == (", flags);
	PRINT_FLAG_W_MASK(flags, O_ACCMODE, O_RDONLY);
	PRINT_FLAG_W_MASK(flags, O_ACCMODE, O_WRONLY);
	PRINT_FLAG_W_MASK(flags, O_ACCMODE, O_RDWR);
	PRINT_FLAG(flags, O_CREAT);
	PRINT_FLAG(flags, O_EXCL);
	PRINT_FLAG(flags, O_NOCTTY);
	PRINT_FLAG(flags, O_TRUNC);
	PRINT_FLAG(flags, O_APPEND);
	PRINT_FLAG(flags, O_NONBLOCK);
	PRINT_FLAG(flags, O_SYNC);
	PRINT_FLAG(flags, O_ASYNC);
	printf(")");
}

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
	printf("open(%s,", filename);
	print_file_flags(OPEN_FLAGS);
	printf(")");
	fflush(stdout);
	int fd = open(filename, OPEN_FLAGS);
	printf(" = %d\n", fd);
	if (fd < 0)
		l_error("open : Unable to open file \"%s\"\n", filename);
	return fd;
}

void l_close (int fd)
{
        close(fd);
}

#define PRINT_ARR(_CALL_, _BUFF_, _TAB_) _CALL_("%s%s0x%02X", (i ? ", " : ""), (!i || i % 8 ? "" : "\n" _TAB_), (_BUFF_)[i])

int l_read (int filedes, void *buffer, uint32_t size)
{
//	printf("read(%d, 0x%zX, %zu)", filedes, (intptr_t)buffer, size);
//	fflush(stdout);
	int rcvd = read(filedes, buffer, size);
//	printf(" = %d\n", rcvd);
#if 0
	log_printf("rcvd: %d\n", rcvd);
#if 1
	log_printhex(buffer, rcvd);
#else
	for (int i = 0; i < rcvd; i++)
		PRINT_ARR(log_printf, (uint8_t*)buffer, "");
#endif
	log_printf("\n");
#endif
	return rcvd;
}

int l_write (int filedes, const void *buffer, uint32_t size)
{
//	printf("write(%d, 0x%zX, %zu)", filedes, (intptr_t)buffer, size);
//	fflush(stdout);
//	printf("write %d ... \n", size);
	int send = write(filedes, buffer, size);
//	printf("complete\n");
//	printf(" = %d\n", send);
#if 0
	log_printf("send: %d\n", send);
#if 0
	log_printhex(buffer, send);
	log_printf("\n");
#endif
#endif
	return send;
}

static void print_iflag (tcflag_t flags)
{
	printf("  .c_iflag = 0x%X == (", flags);
	PRINT_FLAG(flags, IGNBRK);
	PRINT_FLAG(flags, BRKINT);
	PRINT_FLAG(flags, IGNPAR);
	PRINT_FLAG(flags, PARMRK);
	PRINT_FLAG(flags, INPCK);
	PRINT_FLAG(flags, ISTRIP);
	PRINT_FLAG(flags, INLCR);
	PRINT_FLAG(flags, IGNCR);
	PRINT_FLAG(flags, ICRNL);
	PRINT_FLAG(flags, IUCLC);
	PRINT_FLAG(flags, IXON);
	PRINT_FLAG(flags, IXANY);
	PRINT_FLAG(flags, IXOFF);
	PRINT_FLAG(flags, IMAXBEL);
	PRINT_FLAG(flags, IUTF8);
	printf(")\n");
}
static void print_oflag (tcflag_t flags)
{
	printf("  .c_oflag = 0x%X == (", flags);
	PRINT_FLAG(flags, OPOST);
	PRINT_FLAG(flags, OLCUC);
	PRINT_FLAG(flags, ONLCR);
	PRINT_FLAG(flags, OCRNL);
	PRINT_FLAG(flags, ONOCR);
	PRINT_FLAG(flags, ONLRET);
	PRINT_FLAG(flags, OFILL);
	PRINT_FLAG(flags, OFDEL);
	PRINT_FLAG_W_MASK(flags, NLDLY, NL0);
	PRINT_FLAG_W_MASK(flags, NLDLY, NL1);
	PRINT_FLAG_W_MASK(flags, CRDLY, CR0);
	PRINT_FLAG_W_MASK(flags, CRDLY, CR1);
	PRINT_FLAG_W_MASK(flags, CRDLY, CR2);
	PRINT_FLAG_W_MASK(flags, CRDLY, CR3);
	PRINT_FLAG_W_MASK(flags, TABDLY, TAB0);
	PRINT_FLAG_W_MASK(flags, TABDLY, TAB1);
	PRINT_FLAG_W_MASK(flags, TABDLY, TAB2);
	PRINT_FLAG_W_MASK(flags, TABDLY, TAB3);
	PRINT_FLAG_W_MASK(flags, BSDLY, BS0);
	PRINT_FLAG_W_MASK(flags, BSDLY, BS1);
	PRINT_FLAG_W_MASK(flags, FFDLY, FF0);
	PRINT_FLAG_W_MASK(flags, FFDLY, FF1);
	PRINT_FLAG_W_MASK(flags, VTDLY, VT0);
	PRINT_FLAG_W_MASK(flags, VTDLY, VT1);
	printf(")\n");
}

#define MASK_CBAUDEX (CBAUDEX | 017)
static void print_cflag (tcflag_t flags)
{
	printf("  .c_cflag = 0x%X == (", flags);
	PRINT_FLAG_W_MASK(flags, CBAUD, B0);
	PRINT_FLAG_W_MASK(flags, CBAUD, B50);
	PRINT_FLAG_W_MASK(flags, CBAUD, B75);
	PRINT_FLAG_W_MASK(flags, CBAUD, B110);
	PRINT_FLAG_W_MASK(flags, CBAUD, B134);
	PRINT_FLAG_W_MASK(flags, CBAUD, B150);
	PRINT_FLAG_W_MASK(flags, CBAUD, B200);
	PRINT_FLAG_W_MASK(flags, CBAUD, B300);
	PRINT_FLAG_W_MASK(flags, CBAUD, B600);
	PRINT_FLAG_W_MASK(flags, CBAUD, B1200);
	PRINT_FLAG_W_MASK(flags, CBAUD, B1800);
	PRINT_FLAG_W_MASK(flags, CBAUD, B2400);
	PRINT_FLAG_W_MASK(flags, CBAUD, B4800);
	PRINT_FLAG_W_MASK(flags, CBAUD, B9600);
	PRINT_FLAG_W_MASK(flags, CBAUD, B19200);
	PRINT_FLAG_W_MASK(flags, CBAUD, B38400);
	PRINT_FLAG_W_MASK(flags, CSIZE, CS5);
	PRINT_FLAG_W_MASK(flags, CSIZE, CS6);
	PRINT_FLAG_W_MASK(flags, CSIZE, CS7);
	PRINT_FLAG_W_MASK(flags, CSIZE, CS8);
	PRINT_FLAG(flags, CSTOPB);
	PRINT_FLAG(flags, CREAD);
	PRINT_FLAG(flags, PARENB);
	PRINT_FLAG(flags, PARODD);
	PRINT_FLAG(flags, HUPCL);
	PRINT_FLAG(flags, CLOCAL);
	PRINT_FLAG_W_MASK(flags, MASK_CBAUDEX, B57600);
	PRINT_FLAG_W_MASK(flags, MASK_CBAUDEX, B115200);
	PRINT_FLAG_W_MASK(flags, MASK_CBAUDEX, B230400);
	PRINT_FLAG_W_MASK(flags, MASK_CBAUDEX, B460800);
	PRINT_FLAG_W_MASK(flags, MASK_CBAUDEX, B500000);
	PRINT_FLAG_W_MASK(flags, MASK_CBAUDEX, B576000);
	PRINT_FLAG_W_MASK(flags, MASK_CBAUDEX, B921600);
	PRINT_FLAG_W_MASK(flags, MASK_CBAUDEX, B1000000);
	PRINT_FLAG_W_MASK(flags, MASK_CBAUDEX, B1152000);
	PRINT_FLAG_W_MASK(flags, MASK_CBAUDEX, B1500000);
	PRINT_FLAG_W_MASK(flags, MASK_CBAUDEX, B2000000);
	PRINT_FLAG_W_MASK(flags, MASK_CBAUDEX, B2500000);
	PRINT_FLAG_W_MASK(flags, MASK_CBAUDEX, B3000000);
	PRINT_FLAG_W_MASK(flags, MASK_CBAUDEX, B3500000);
	PRINT_FLAG_W_MASK(flags, MASK_CBAUDEX, B4000000);
	PRINT_FLAG(flags, CMSPAR);
	PRINT_FLAG(flags, CRTSCTS);
	printf(")\n");
}
static void print_lflag (tcflag_t flags)
{
	printf("  .c_lflag = 0x%X == (", flags);
	PRINT_FLAG(flags, ISIG);
	PRINT_FLAG(flags, ICANON);
	PRINT_FLAG(flags, XCASE);
	PRINT_FLAG(flags, ECHO);
	PRINT_FLAG(flags, ECHOE);
	PRINT_FLAG(flags, ECHOK);
	PRINT_FLAG(flags, ECHONL);
	PRINT_FLAG(flags, NOFLSH);
	PRINT_FLAG(flags, TOSTOP);
	PRINT_FLAG(flags, ECHOCTL);
	PRINT_FLAG(flags, ECHOPRT);
	PRINT_FLAG(flags, ECHOKE);
	PRINT_FLAG(flags, FLUSHO);
	PRINT_FLAG(flags, PENDIN);
	PRINT_FLAG(flags, IEXTEN);
	printf(")\n");
}

static void print_termios (const struct termios *termios_p)
{
	printf("{\n");
	print_iflag(termios_p->c_iflag);
	print_oflag(termios_p->c_oflag);
	print_cflag(termios_p->c_cflag);
	print_lflag(termios_p->c_lflag);
	printf("  .c_line = 0x%X\n", termios_p->c_line);
	printf("  .c_cc = [ ");
	for (int i = 0; i < sizeof(termios_p->c_cc)/sizeof(termios_p->c_cc[0]); i++) {
		PRINT_ARR(printf, termios_p->c_cc, "            ");
	}
	printf(" ]\n");
	printf("  .c_ispeed = 0x%X\n", termios_p->c_ispeed);
	printf("  .c_ospeed = 0x%X\n", termios_p->c_ospeed);
	printf("}\n");
}

static int l_tcgetattr (int fildes, struct termios *termios_p)
{
	printf("tcgetattr(%d, 0x%zX)", fildes, (intptr_t)termios_p);
	int res = tcgetattr(fildes, termios_p);
	printf(" = %d\n", res);
	print_termios(termios_p);
	return res;
}

static int l_tcsetattr (int fildes, int optional_actions, const struct termios *termios_p)
{
	print_termios(termios_p);
	printf("tcsetattr(%d, 0x%X, 0x%zX)", fildes, optional_actions, (intptr_t)termios_p);
	int res = tcsetattr(fildes, optional_actions, termios_p);
	printf(" = %d\n", res);
	return res;
}


#define PORT_BAUDRATE B115200

void set_term (int fd, int rtscts_flow_control_enable)
{
        struct termios ttystate;

        l_tcgetattr(fd, &ttystate);

        ttystate.c_iflag = 0;
        ttystate.c_oflag = 0;
        ttystate.c_cflag = PORT_BAUDRATE | CS8 | CREAD | CLOCAL;
        if (rtscts_flow_control_enable)
                ttystate.c_cflag |= CRTSCTS;
        ttystate.c_lflag = 0;

        ttystate.c_ispeed = PORT_BAUDRATE;
        ttystate.c_ospeed = PORT_BAUDRATE;

#if 0
        ttystate.c_line = 0;
        cc_t cc_tmp[] = { 0x03, 0x1C, 0x7F, 0x15, 0x04, 0x05, 0x01, 0x00,
        0x11, 0x13, 0x1A, 0x00, 0x12, 0x0F, 0x17, 0x16,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
        memcpy(ttystate.c_cc, cc_tmp, sizeof(ttystate.c_cc));
#endif

        l_tcsetattr(fd, TCSANOW, &ttystate);
}

int thread_start (thread_t* thread, void (*start_routine) (void))
{
        int err = pthread_create(&thread->thread, 0, (void * (*)(void *))start_routine, 0);

        return err;
}

void thread_join (thread_t* thread)
{
        pthread_join(thread->thread, 0);
}

void mutex_init (mutex_t* mutex)
{
        pthread_mutex_init(&mutex->mutex, 0);
}

void mutex_lock (mutex_t* mutex)
{
        pthread_mutex_lock(&mutex->mutex);

}

void mutex_unlock (mutex_t* mutex)
{
        pthread_mutex_unlock(&mutex->mutex);
}

void cond_init (cond_t* cond)
{
        pthread_mutex_init(&cond->mutex, 0);
        pthread_cond_init(&cond->cond, 0);
}

void cond_wait (cond_t* cond)
{
        pthread_mutex_lock(&cond->mutex);
        pthread_cond_wait(&cond->cond, &cond->mutex);
        pthread_mutex_unlock(&cond->mutex);

}

void cond_notify (cond_t* cond)
{
        pthread_cond_signal(&cond->cond);
}



struct timeval timer_start;
struct timeval timer_expire;

void timer_restart (uint32_t count_ms)
{
        gettimeofday(&timer_start, 0);

        struct timeval add = {.tv_sec = count_ms/1000, .tv_usec = 1000*(count_ms%1000)};

        timeradd(&timer_start, &add, &timer_expire);
}

uint32_t timer_elapsed_ms (void)
{
        struct timeval now;

        gettimeofday(&now, 0);

        struct timeval sub;

        timersub(&now, &timer_start, &sub);

        return sub.tv_sec*1000 + sub.tv_usec/1000;
}

int timer_expired (void)
{
        struct timeval now;

        gettimeofday(&now, 0);

        if (timercmp(&now, &timer_expire, >)) {
                return 1;
        } else {
                return 0;
        }
}

tymestamp_t timer_timestamp (void)
{
        tymestamp_t result;

        struct timeval _now;

        gettimeofday(&_now, 0);

        struct tm *now = localtime(&_now.tv_sec);

        result.h = now->tm_hour;
        result.m = now->tm_min;
        result.s = now->tm_sec;
        result.ms = _now.tv_usec/1000;

        return  result;
}
