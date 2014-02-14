#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <stdint.h>

typedef enum LOG_START_TYPE_ {
	LOG_TRUNC,
	LOG_APPEND,
//	LOG_NEW_FILE
} LOG_START_TYPE_T;

/**
 * \return 1 if file opened, 0 - otherwise
 */
int log_start (const char* filename, LOG_START_TYPE_T start_type, int stdout_flag);

void log_stop (void);

/**
 * NOTICE: do not copy content of text, just grab the pointer
 *
 * \return 1 if message successfully pushed, 0 - otherwise
 */
int log_push_msg (char* text, uint32_t text_size);

/**
 * \return 1 if message successfully pushed, 0 - otherwise
 */
int log_printf (const char* format, ...);

int log_printhex (const void* bin, uint32_t size);


#endif
