
#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <stddef.h>
#include <time.h>
#include <stdarg.h>

#include "logger.h"
#include "utils.h"


typedef struct log_queue_node log_queue_node_t;
struct log_queue_node {
	char* text;
	uint32_t text_size;
	log_queue_node_t* next;
};

static struct log_queue {
	log_queue_node_t* first;
	log_queue_node_t* last;
	uint32_t count;
	const char* filename;
	int fd;
	thread_t thread;
	mutex_t lock;
	cond_t cond;
	int enabled;
	int stopped;
	int stdout_flag;
} lq = {
	.first = 0,
	.last = 0,
	.count = 0,
	.filename = 0,
	.fd = -1,
	.enabled = 0,
	.stopped = 0
};



char* bin2hex (const void* bin, uint32_t bin_size, uint32_t* hex_size, int line_len)
{
	char* result = 0;

	if (bin) {
		uint32_t size = bin_size * 2 + ((line_len > 0) ? bin_size/line_len : 0);
		result = malloc(size+1);
		if (result) {
			if (hex_size)
				*hex_size = size;
			char* ptr = result;
			const uint8_t* binptr = bin;
			uint32_t linecnt = 0;
			while (bin_size) {
				if ((line_len > 0) && linecnt == line_len) {
					*ptr++ = '\n';
					linecnt = 0;
				}
				uint8_t b = *binptr++;
				uint8_t h = b >> 4;
				uint8_t l = b & 0x0F;
				*ptr++ = (h > 9) ? h + 55 : h + 48;
				*ptr++ = (l > 9) ? l + 55 : l + 48;
				linecnt++;
				bin_size--;
			}
			if (line_len > 0)
				*ptr++ = '\n';
			*ptr = 0;
		}
	}

	return result;
}

/**
 * \return 1 if file opened, 0 - otherwise
 */
static int __log_open (const char* filename, LOG_START_TYPE_T start_type, int stdout_flag)
{
	int res = 0;
	if (!lq.filename) {
		switch (start_type) {
		case LOG_APPEND:
			lq.fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0666);
			break;
		case LOG_TRUNC:
			lq.fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
			break;
		}
		if (lq.fd > 0) {
			lq.filename = filename;
			res = 1;
		}
	}
	if (stdout_flag) {
		lq.stdout_flag = stdout_flag;
		res = 1;
	}
	return res;
}

int log_push_msg (char* text, uint32_t text_size)
{
	int res = 0;
	mutex_lock(&lq.lock);
	if (lq.enabled) {
		log_queue_node_t* node = malloc(sizeof(*node));
		if (node) {
			node->text = text;
			node->text_size = text_size;
			node->next = 0;
			if (lq.last)
				lq.last->next = node;
			if (!lq.first)
				lq.first = node;
			lq.last = node;
			lq.count++;
			res = 1;
			cond_notify(&lq.cond);
		}
	}
	mutex_unlock(&lq.lock);
	return res;
}

int log_printhex (const void* bin, uint32_t size)
{
	uint32_t text_size = 0;
	char* text = bin2hex(bin, size, &text_size, 64);
	return log_push_msg(text, text_size);
}

int log_printf (const char* format, ...)
{
	int err = 0;
	if (lq.enabled) {
	        int buff_size = 0;
                va_list ap;
                va_start(ap, format);
#ifdef WIN32
	        buff_size = _vscprintf(format, ap);
                va_end(ap);
#else
                buff_size = vsnprintf(0, 0, format, ap);
#endif
                buff_size += 2;
		char* text = malloc(buff_size);
                va_start(ap, format);
		int size = vsnprintf(text, buff_size, format, ap);
		va_end(ap);
		if (size > 0) {
			err = log_push_msg(text, size);
		}
	}
	return err;
}

static int __log_pop_msg (char** text, uint32_t* text_size)
{
	int res = 0;
	if (lq.count) {
		if (text)
			*text = lq.first->text;
		else
			free(lq.first->text);
		if (text_size)
			*text_size = lq.first->text_size;
		log_queue_node_t* node = lq.first;
		lq.first = node->next;
		if (node == lq.last)
			lq.last = 0;
		free(node);
		lq.count--;
		res = 1;
	}
	return res;
}

static int __log_write_msg (void)
{
	char* text = 0;
	uint32_t size = 0;
	if (__log_pop_msg(&text, &size)) {
		write(lq.fd, text, size);
		if (lq.stdout_flag)
			write(STDOUT_FILENO, text, size);
		free(text);
		return 1;
	}
	return 0;
}

static void __log_close (void)
{
	mutex_lock(&lq.lock);
	/* Pop all messages and write them to file. */
	while (__log_write_msg()) ;
	/* Close file. */
//	int oldflags = fcntl(lq.fd, F_GETFL, 0);
//	fcntl(lq.fd, F_SETFL, oldflags | O_FSYNC);
	write(lq.fd, 0, 0);
	close(lq.fd);
	lq.filename = 0;
	lq.fd = 0;
	mutex_unlock(&lq.lock);
}

static void __log_thread (void)
{
	do {
		cond_wait(&lq.cond);
                mutex_lock(&lq.lock);
		while (__log_write_msg());
		mutex_unlock(&lq.lock);
	} while (!lq.stopped);
	__log_close();
}

int log_start (const char* filename, LOG_START_TYPE_T start_type, int stdout_flag)
{
	int res = 0;

	mutex_init(&lq.lock);
        cond_init(&lq.cond);

	if (!lq.enabled) {
		res = __log_open(filename, start_type, stdout_flag);
		if (res) {
			int err = thread_start(&lq.thread, __log_thread);
			res = !err;
		}
	}
	lq.enabled = res;
	return res;
}

void log_stop (void)
{
	if (lq.enabled) {
		mutex_lock(&lq.lock);
		lq.stopped = 1;
		lq.enabled = 0;
                mutex_unlock(&lq.lock);
		cond_notify(&lq.cond);
		thread_join(&lq.thread);
	}
}

#ifdef COMPILING_TEST

int thread_count = 0;

void test_log (void)
{
        int thread_self = thread_count++;
	log_printf("thread %d started\n", thread_self);
	int count = 10;
	while (count--) {
		log_printf("thread %d count %d\n", thread_self, count);
	}
	log_printf("thread %d exitting\n", thread_self);
}

#define NUM_THREADS 10

int main (int argc, const char* argv[])
{

	intptr_t t;
	int rc;
	thread_t thread[NUM_THREADS*2];

	for(t=0; t<NUM_THREADS; t++) {
		printf("Main: creating thread %zd\n", t);
		rc = thread_start(&thread[t], test_log);
		if (rc) {
			printf("ERROR; return code from thread_start() is %d\n", rc);
			exit(-1);
		}
	}

	log_start("log.txt", LOG_APPEND, 1);

	for(t=NUM_THREADS; t<NUM_THREADS*2; t++) {
		log_printf("Main: creating thread %zd\n", t);
		rc = thread_start(&thread[t], test_log);
		if (rc) {
			log_printf("ERROR; return code from thread_start() is %d\n", rc);
			exit(-1);
		}
	}

	for(t=0; t<NUM_THREADS*2; t++) {
		thread_join(&thread[t]);
		log_printf("Main: completed join with thread %zd\n", t);
	}

	log_stop();

	return 0;
}

#endif
