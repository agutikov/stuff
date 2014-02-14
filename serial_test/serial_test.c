
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <errno.h>
#include <sys/time.h>
#include <time.h>

#include "params.h"
#include "logger.h"
#include "utils.h"

#ifndef WIN32
#include <sys/ioctl.h>
#include <pthread.h>
#include <termios.h>
#endif



int instant_pipe (int fd_in, int fd_out, uint32_t buffer_size)
{
	int res = 1;
	uint32_t total_send = 0;
	uint32_t total_rcvd = 0;

	uint8_t* buffer = (uint8_t*) l_malloc(buffer_size);
	if (!buffer)
		return res;

	while (1) {
		int rcvd = l_read(fd_in, buffer, buffer_size);
		if (rcvd >= 0)
			total_rcvd += rcvd;
		else {
			l_error("read\n");
                        break;
		}
		printf("total rcvd %u\n", total_rcvd);
		int send = l_write(fd_out, buffer, rcvd);
		if (send >= 0)
			total_send += send;
		else {
			l_error("write\n");
			break;
		}
		printf("total send %u\n", total_send);
	}
	res = 0;

	if (buffer)
		free(buffer);

	return res;
}

#ifdef WIN32

int bytes_left_stdin (HANDLE handle)
{

        DWORD bytes_left_stdin;
        PeekNamedPipe(handle, NULL, 0, NULL, &bytes_left_stdin, NULL);
        return bytes_left_stdin;
}

int bytes_left_com_port (HANDLE handle)
{
        COMSTAT comstat;
        DWORD errors;

        ClearCommError(handle, &errors, &comstat);

        return comstat.cbInQue;
}

int just_open_test (int fd_in, int fd_out, uint32_t buffer_size)
{
        int res = 1;
        uint32_t total_rcvd = 0;
        int run_read = 1;
        int runnning = 1;

        uint8_t* buffer = (uint8_t*) l_malloc(buffer_size);
        if (!buffer)
                return res;

        SYSTEMTIME now;
        SYSTEMTIME next_show;
        GetLocalTime(&next_show);

        printf("read %s\n", run_read ? (run_read == 2 ? "slow" : "fast") : "disabled");

        HANDLE stdin_handle = GetStdHandle(STD_INPUT_HANDLE);

        while (runnning) {

                GetLocalTime(&now);

                FILETIME _now;
                FILETIME _next_show;

                SystemTimeToFileTime(&now, &_now);
                SystemTimeToFileTime(&next_show, &_next_show);

                int bytes = bytes_left_com_port((HANDLE)fd_in);

                if (CompareFileTime(&_now, &_next_show) >= 0) { // > 0 means that _now is later than _next_show

                        printf("[%02d:%02d:%02d.%03d] %d av, total rcvd %u (%uM %uk)\n",
                                now.wHour, now.wMinute, now.wSecond, now.wMilliseconds,
                                bytes,
                                total_rcvd, total_rcvd/(1024*1024), total_rcvd/1024);

                        ULONGLONG now_64 = (((ULONGLONG) _now.dwHighDateTime) << 32) + _now.dwLowDateTime;

                        now_64 += 10000000;

                        _now.dwLowDateTime  = (DWORD) (now_64 & 0xFFFFFFFF );
                        _now.dwHighDateTime = (DWORD) (now_64 >> 32 );

                        FileTimeToSystemTime(&_now, &next_show);
                }

                if (bytes) {
                        if (run_read == 1) {
                                int rcvd = l_read(fd_in, buffer, buffer_size);
                                if (rcvd >= 0) {
                                        total_rcvd += rcvd;
                                } else {
                                        l_error("read\n");
                                        break;
                                }
                        } else if (run_read == 2) {
                                int rcvd = l_read(fd_in, buffer, buffer_size);
                                if (rcvd >= 0) {
                                        total_rcvd += rcvd;
                                } else {
                                        l_error("read\n");
                                        break;
                                }
                                usleep(100000);
                        }
                }

                continue;


                if (bytes_left_stdin(stdin_handle)) {
                        int rcvd;
                        char c;
                        rcvd = read(STDIN_FILENO, &c, 1);
                        if (rcvd < 0) {
                                perror("read");
                                break;
                        } else if (rcvd == 1) {
                                switch (c) {
                                        case 't':
                                                run_read++;
                                                if (run_read > 2)
                                                        run_read = 0;
                                                printf("read %s\n", run_read ? (run_read == 2 ? "slow" : "fast") : "disabled");
                                                break;
                                        case 'q':
                                                printf("exit\n");
                                                runnning = 0;
                                                break;
                                };
                        }
                }

        }
        res = 0;

        if (buffer)
                free(buffer);

        return res;
}
#else
int just_open_test (int fd_in, int fd_out, uint32_t buffer_size)
{
	int res = 1;
	uint32_t total_rcvd = 0;
	uint32_t prev_total_rcvd = 0;
	int run_read = 1;
	int runnning = 1;

	uint8_t* buffer = (uint8_t*) l_malloc(buffer_size);
	if (!buffer)
		return res;

	struct termios stdio_ttystate;
	struct termios stdio_ttysave;
	tcgetattr(STDIN_FILENO, &stdio_ttystate);
	stdio_ttysave = stdio_ttystate;
	stdio_ttystate.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	tcsetattr(STDIN_FILENO, TCSANOW, &stdio_ttystate);

	struct timeval now;
	struct timeval next_show;
	gettimeofday(&next_show, 0);

	printf("read %s\n", run_read ? (run_read == 2 ? "slow" : "fast") : "disabled");

	while (runnning) {

		gettimeofday(&now, 0);

		if (timercmp(&now, &next_show, >)) {
			int avaliable;
			int r = ioctl(fd_in, FIONREAD, &avaliable);
			if (!r) {
				struct tm *tm;
				tm = localtime(&now.tv_sec);
				struct timeval dt;
				timersub(&now, &next_show, &dt);
				int speed = (total_rcvd - prev_total_rcvd)*1000/((dt.tv_sec + 1)*1000 + dt.tv_usec/1000);

				printf("[%02d:%02d:%02d.%03ld] %d av, total rcvd %u (%uM %uk), %d b/s (%d Mb/s, %d kb/s)\n",
					tm->tm_hour, tm->tm_min, tm->tm_sec, now.tv_usec/1000,
					avaliable,
					total_rcvd, total_rcvd/(1024*1024), total_rcvd/1024,
					speed, speed/(1024*1024), speed/1024);

				prev_total_rcvd = total_rcvd;

			} else {
				int err = errno;
				printf("ioctl FIONREAD return %d, errno 0x%X : %s\n", r, err, strerror(err));
				runnning = 0;
			}
			next_show = now;
			next_show.tv_sec += 1;
		}

		fd_set set;
		struct timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		FD_ZERO(&set);
		FD_SET(STDIN_FILENO, &set);
		FD_SET(fd_in, &set);

		int sel = select(FD_SETSIZE, &set, NULL, NULL, &timeout);

		if (sel == -1) {
			perror("select error: ");
			break;
		} else if (sel == 0) {
			continue;
		} else if (sel > 0) {
			if (FD_ISSET(STDIN_FILENO, &set)) {
				int rcvd;
				char c;
				rcvd = read(STDIN_FILENO, &c, 1);
				if (rcvd < 0) {
					perror("read");
					break;
				} else if (rcvd == 1) {
					switch (c) {
						case 't':
							run_read++;
							if (run_read > 2)
								run_read = 0;
							printf("read %s\n", run_read ? (run_read == 2 ? "slow" : "fast") : "disabled");
							break;
						case 'q':
							printf("exit\n");
							runnning = 0;
							break;
					};
				}
			}
			if (FD_ISSET(fd_in, &set)) {
				if (run_read == 1) {
					int rcvd = l_read(fd_in, buffer, buffer_size);
					if (rcvd >= 0) {
						total_rcvd += rcvd;
					} else {
						l_error("read\n");
						break;
					}
					// printf("total rcvd %u\n", total_rcvd);
				} else if (run_read == 2) {
					int rcvd = l_read(fd_in, buffer, buffer_size);
					if (rcvd >= 0) {
						total_rcvd += rcvd;
					} else {
						l_error("read\n");
						break;
					}
					usleep(100000);
				}
			}
		} else {
			printf("select: undefined behavior\n");
			break;
		}
	}
	res = 0;

	if (buffer)
		free(buffer);

	tcsetattr(STDIN_FILENO, TCSANOW, &stdio_ttysave);

	return res;
}
#endif

int just_send_test (int fd_out, uint32_t buffer_size, uint32_t count_times)
{
	int res = 1;
	uint32_t total_send = 0;

	uint8_t* buffer_out = l_malloc(buffer_size);
	if (!buffer_out)
		return res;

	uint32_t u32_value = 0;
	uint32_t *u32_ptr = (uint32_t*)buffer_out;

	while (count_times--) {

		for (uint32_t i = 0; i < buffer_size/4; i++) {
			u32_ptr[i] = u32_value;
			u32_value++;
		}

		int send = l_write(fd_out, buffer_out, buffer_size);
		if (send >= 0)
			total_send += send;
		else {
			l_error("write\n");
			break;
		}
		// printf("total send %u\n", total_send);
	}
	res = 0;

	if (buffer_out)
		free(buffer_out);

	return res;
}

#ifdef WIN32
int just_recv_test (int fd_in, uint32_t buffer_size)
{
        int res = 1;
        uint32_t total_rcvd = 0;
        int run_read = 1;
        int runnning = 1;
        uint32_t u32_value = 0;
        int total_diff = 0;

        uint8_t* buffer = (uint8_t*) l_malloc(buffer_size);
        if (!buffer)
                return res;

        SYSTEMTIME now;
        SYSTEMTIME next_show;
        GetLocalTime(&next_show);

        printf("read %s\n", run_read ? "enabled" : "disabled");

        HANDLE stdin_handle = GetStdHandle(STD_INPUT_HANDLE);

        while (runnning) {

                GetLocalTime(&now);

                FILETIME _now;
                FILETIME _next_show;

                SystemTimeToFileTime(&now, &_now);
                SystemTimeToFileTime(&next_show, &_next_show);

                int bytes = bytes_left_com_port((HANDLE)fd_in);

                if (CompareFileTime(&_now, &_next_show) >= 0) { // > 0 means that _now is later than _next_show

                        printf("[%02d:%02d:%02d.%03d] %d av, total rcvd %u (%uM %uk)\n",
                                now.wHour, now.wMinute, now.wSecond, now.wMilliseconds,
                                bytes,
                                total_rcvd, total_rcvd/(1024*1024), total_rcvd/1024);

                        ULONGLONG now_64 = (((ULONGLONG) _now.dwHighDateTime) << 32) + _now.dwLowDateTime;

                        now_64 += 10000000;

                        _now.dwLowDateTime  = (DWORD) (now_64 & 0xFFFFFFFF );
                        _now.dwHighDateTime = (DWORD) (now_64 >> 32 );

                        FileTimeToSystemTime(&_now, &next_show);
                }

                if (bytes) {
                        if (run_read) {
                                int rcvd = l_read(fd_in, buffer, buffer_size);
                                if (rcvd >= 0) {

                                        while (rcvd % 4) {
                                                int tmp = 0;

                                                while (tmp == 0) {
                                                        tmp = l_read(fd_in, buffer + rcvd, rcvd % 4);
                                                }

                                                if (tmp < 0) {
                                                        l_error("read\n");
                                                        runnning = 0;
                                                        break;
                                                }

                                                rcvd += tmp;
                                        }

                                        uint32_t *u32_ptr = (uint32_t*) buffer;
                                        int i = 0;

                                        while (i < rcvd) {
                                                if (u32_value != *u32_ptr) {
                                                        int diff = *u32_ptr - u32_value;
                                                        diff *= 4;
                                                        total_diff += abs(diff);

                                                        printf("broaken: %u %d %d 0x%04X 0x%04X [%02X%02X%02X%02X] 0x%X %d\n",
                                                                        total_rcvd, rcvd, i, u32_value, *u32_ptr,
                                                                        buffer[i], buffer[i+1], buffer[i+2], buffer[i+3],
                                                                        diff, total_diff);

                                                        u32_value = *u32_ptr;

                                                }

                                                u32_value++;
                                                u32_ptr++;
                                                i += 4;
                                        }

                                        total_rcvd += rcvd;
                                } else {
                                        l_error("read\n");
                                        runnning = 0;
                                }
                                // printf("total rcvd %u\n", total_rcvd);
                        }
                }

                continue;


                if (bytes_left_stdin(stdin_handle)) {
                        int rcvd;
                        char c;
                        rcvd = read(STDIN_FILENO, &c, 1);
                        if (rcvd < 0) {
                                perror("read");
                                break;
                        } else if (rcvd == 1) {
                                switch (c) {
                                        case 't':
                                                run_read = !run_read;
                                                printf("read %s\n", run_read ? "enabled" : "disabled");
                                                break;
                                        case 'q':
                                                printf("exit\n");
                                                runnning = 0;
                                                break;
                                };
                        }
                }

        }
        res = 0;

        if (buffer)
                free(buffer);

        return res;
}
#else
int just_recv_test (int fd_in, uint32_t buffer_size)
{
	int res = 1;
	uint32_t total_rcvd = 0;
	uint32_t prev_total_rcvd = 0;
	int run_read = 1;
	int runnning = 1;

	uint8_t* buffer = (uint8_t*) l_malloc(buffer_size);
	if (!buffer)
	        return res;

	struct termios stdio_ttystate;
	struct termios stdio_ttysave;
	tcgetattr(STDIN_FILENO, &stdio_ttystate);
	stdio_ttysave = stdio_ttystate;
	stdio_ttystate.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	tcsetattr(STDIN_FILENO, TCSANOW, &stdio_ttystate);

	printf("read %s\n", run_read ? "enabled" : "disabled");

	uint32_t u32_value = 0;
	int total_diff = 0;

	timer_restart(1000);

	while (runnning) {

		if (timer_expired()) {
			int avaliable;
			int r = ioctl(fd_in, FIONREAD, &avaliable);
			if (!r) {
			        tymestamp_t now = timer_timestamp();
				int speed = (total_rcvd - prev_total_rcvd)*1000/timer_elapsed_ms();

				printf("[%02d:%02d:%02d.%03d] %d av, total rcvd %u (%uM %uk), %d b/s (%d Mb/s, %d kb/s) %d\n",
				           now.h, now.m, now.s, now.ms,
                                           avaliable,
                                           total_rcvd, total_rcvd/(1024*1024), total_rcvd/1024,
                                           speed, speed/(1024*1024), speed/1024, u32_value);

				prev_total_rcvd = total_rcvd;

			} else {
				int err = errno;
				printf("ioctl FIONREAD return %d, errno 0x%X : %s\n", r, err, strerror(err));
				runnning = 0;
			}

		        timer_restart(1000);
		}

		fd_set set;
		struct timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		FD_ZERO(&set);
		FD_SET(STDIN_FILENO, &set);
		FD_SET(fd_in, &set);

		int sel = select(FD_SETSIZE, &set, NULL, NULL, &timeout);

		if (sel == -1) {
			perror("select error: ");
			break;
		} else if (sel == 0) {
			continue;
		} else if (sel > 0) {
			if (FD_ISSET(STDIN_FILENO, &set)) {
				int rcvd;
				char c;
				rcvd = read(STDIN_FILENO, &c, 1);
				if (rcvd < 0) {
					perror("read");
					break;
				} else if (rcvd == 1) {
					switch (c) {
						case 't':
							run_read = !run_read;
							printf("read %s\n", run_read ? "enabled" : "disabled");
							break;
						case 'q':
							printf("exit\n");
							runnning = 0;
							break;
					};
				}
			}
			if (FD_ISSET(fd_in, &set)) {
				if (run_read) {
					int rcvd = l_read(fd_in, buffer, buffer_size);
					if (rcvd >= 0) {

						while (rcvd % 4) {
							int tmp = 0;

							while (tmp == 0) {
								tmp = l_read(fd_in, buffer + rcvd, rcvd % 4);
							}

							if (tmp < 0) {
								l_error("read\n");
								runnning = 0;
								break;
							}

							rcvd += tmp;
						}

						uint32_t *u32_ptr = (uint32_t*) buffer;
						int i = 0;

						while (i < rcvd) {
							if (u32_value != *u32_ptr) {
								int diff = *u32_ptr - u32_value;
								diff *= 4;
								total_diff += abs(diff);

								printf("broaken: %u %d %d 0x%04X 0x%04X [%02X%02X%02X%02X] 0x%X %d\n",
										total_rcvd, rcvd, i, u32_value, *u32_ptr,
										buffer[i], buffer[i+1], buffer[i+2], buffer[i+3],
										diff, total_diff);

								u32_value = *u32_ptr;

							}

							u32_value++;
							u32_ptr++;
							i += 4;
						}

						total_rcvd += rcvd;
					} else {
						l_error("read\n");
						runnning = 0;
					}
					// printf("total rcvd %u\n", total_rcvd);
				}
			}
		} else {
			printf("select: undefined behavior\n");
			break;
		}
	}
	res = 0;

	if (buffer)
		free(buffer);

	tcsetattr(STDIN_FILENO, TCSANOW, &stdio_ttysave);

	return res;
}
#endif


int fixed_block_pipe (int fd_in, int fd_out, uint32_t buffer_size)
{
	int res = 1;
	uint32_t total_send = 0;
	uint32_t total_rcvd = 0;

	uint8_t* buffer = (uint8_t*) l_malloc(buffer_size);
	if (!buffer)
		return res;

	while (1) {
		uint32_t wait_for = buffer_size;
		uint8_t* ptr = buffer;
		while (wait_for > 0) {
			int rcvd = l_read(fd_in, ptr, wait_for);
			if (rcvd >= 0)
				total_rcvd += rcvd;
			else {
				l_error("read\n");
				break;
			}
			ptr += rcvd;
			wait_for -= rcvd;
		}
		printf("total rcvd %u\n", total_rcvd);
		int send = l_write(fd_out, buffer, buffer_size);
		if (send >= 0)
			total_send += send;
		else {
			l_error("write\n");
			break;
		}
		printf("total send %u\n", total_send);
	}
	res = 0;

	if (buffer)
		free(buffer);

	return res;
}

void log_err (void)
{
	int err = errno;
	const char* errstr = strerror(err);
	log_printf("ERROR: %d - %s\n", err, errstr);
}

struct stream
{
	int fd;
	uint8_t* buffer;
	uint32_t buffer_size;
	uint32_t count;
};

void* _reading_thread (void* arg)
{
	struct stream* read_stream = arg;

	int total_rcvd = 0;
	int total_wait = read_stream->buffer_size*read_stream->count;
	while (total_rcvd < total_wait) {
		int rcvd = read(read_stream->fd, read_stream->buffer, total_wait - total_rcvd);
		if (rcvd < 0) {
			log_printf("read %d\n", rcvd);
			log_err();
			break;
		}
		total_rcvd += rcvd;
		log_printf("total_rcvd %d\n", total_rcvd);
		usleep(1000);
	}

	return 0;
}

#ifdef WIN32
int thread_test (int fd_in, int fd_out, uint32_t buffer_size, uint32_t count)
{
        printf("Not implemented\n");
        return 0;
}
#else
int thread_test (int fd_in, int fd_out, uint32_t buffer_size, uint32_t count)
{
	int res = 1;
	int total_send = 0;

	uint8_t* buffer_out = l_malloc(buffer_size);
	if (!buffer_out)
                return res;
	uint8_t* buffer_in = l_malloc(buffer_size);
	if (!buffer_in) {
	        free(buffer_out);
	        return res;
	}


	struct stream read_stream = {fd_in, buffer_in, buffer_size, count};

	for (uint32_t i = 0; i < buffer_size; i++) {
		buffer_out[i] = i;
	}

	pthread_t reading_thread;

	int err = pthread_create(&reading_thread, 0, _reading_thread, &read_stream);
	log_printf("pthread_create %d\n", err);

	for (uint32_t i = 0; i < count; i++) {
		int send = write(fd_out, buffer_out, buffer_size);
		log_printf("send %d\n", send);
		total_send += send;
		log_printf("total_send %d\n", total_send);
	}

	pthread_join(reading_thread, 0);

	res = 0;

	if (buffer_out)
		free(buffer_out);
	if (buffer_in)
		free(buffer_in);

	return res;
}
#endif

int simple_test (int fd_in, int fd_out, uint32_t buffer_size, uint32_t count_times)
{
	int res = 1;
	uint32_t total_send = 0;
	uint32_t total_rcvd = 0;

	uint8_t* buffer_out = l_malloc(buffer_size);
	if (!buffer_out)
                return res;
	uint8_t* buffer_in = l_malloc(buffer_size);
	if (!buffer_in) {
                free(buffer_out);
                return res;
        }

	for (uint32_t i = 0; i < buffer_size; i++) {
		buffer_out[i] = i;
	}

	while (count_times--) {
		int send = l_write(fd_out, buffer_out, buffer_size);
		if (send >= 0)
			total_send += send;
		else {
			l_error("write\n");
			break;
		}
		printf("total send %u\n", total_send);
		uint8_t* ptr = buffer_in;
		while (send > 0) {
			int rcvd = l_read(fd_in, ptr, send);
			if (rcvd >= 0)
				total_rcvd += rcvd;
			else {
				l_error("read\n");
				break;
			}
			ptr += rcvd;
			send -= rcvd;
		}
		printf("total rcvd %u\n", total_rcvd);
		printf("memcmp(0x%p, 0x%p, %u)", buffer_out, buffer_in, buffer_size);
		int cmpres = memcmp(buffer_out, buffer_in, buffer_size);
		printf(" = %d\n", cmpres);
	}
	res = 0;

	if (buffer_out)
		free(buffer_out);
	if (buffer_in)
		free(buffer_in);

	return res;
}




int main (int argc, const char* argv[])
{
	int res = 1;

	program_options_t options;

	res = get_program_options(&options, argc, argv);
	if (res)
	        return res;

	int fd[2] = {0};
	int *fd_in = 0;
	int *fd_out = 0;
	const char* filename_in = 0;
	const char* filename_out = 0;

	if (options.files_number == 1) {
		fd[0] = l_open(options.filename[0]);
		if (fd[0] < 0)
	                return res;

		fd_in = &fd[0];
		fd_out = &fd[0];

		filename_in = options.filename[0];
		filename_out = options.filename[0];

		if (options.iterm && options.oterm)
			set_term(fd[0], options.flow_ctrl_en);

	} else if (options.files_number == 2) {
		fd[0] = l_open(options.filename[0]);
		if (fd[0] < 0)
	                return res;

		fd[1] = l_open(options.filename[1]);
		if (fd[1] < 0)
	                return res;

		fd_in = &fd[0];
		fd_out = &fd[1];

		filename_in = options.filename[0];
		filename_out = options.filename[1];

		if (options.iterm)
			set_term(fd[0], options.flow_ctrl_en);

		if (options.oterm)
			set_term(fd[1], options.flow_ctrl_en);
	} else
		return res;

	printf("\n");

	if (options.log_filename)
		printf("log to \"%s\", %s\n", options.log_filename, (options.log_type == LOG_APPEND ? "append" : "truncate"));
	if (options.logstdout)
		printf("log to stdout\n");
	if (options.log_filename || options.logstdout)
		log_start(options.log_filename, options.log_type, options.logstdout);

	switch (options.run_type) {
	case RUN_TEST:
		printf("running test\ninput from \"%s\"\noutput to \"%s\"\nbuffer size = %u\ncount times = %u\n\n",
		       filename_in, filename_out, options.buffer_size, options.count);
		res = simple_test(*fd_in, *fd_out, options.buffer_size, options.count);
		break;
	case RUN_IPIPE:
		printf("running instant pipe\ninput from \"%s\"\noutput to \"%s\"\nbuffer size = %u\n\n",
		       filename_in, filename_out, options.buffer_size);
		res = instant_pipe(*fd_in, *fd_out, options.buffer_size);
		break;
	case RUN_FPIPE:
		printf("running fozed block pipe\ninput from \"%s\"\noutput to \"%s\"\nbuffer size = %u\n\n",
		       filename_in, filename_out, options.buffer_size);
		res = fixed_block_pipe(*fd_in, *fd_out, options.buffer_size);
		break;
	case RUN_OPEN:
		printf("just open device, start read on anykey, then stop on anykey \ninput from \"%s\"\n\n",
		       filename_in);
		res = just_open_test(*fd_in, *fd_out, options.buffer_size);
		break;
	case RUN_THREAD:
		printf("running hardware flow control test\ninput from \"%s\"\noutput to \"%s\"\nbuffer size = %u\ncount times = %u\n\n",
		       filename_in, filename_out, options.buffer_size, options.count);
		res = thread_test(*fd_in, *fd_out, options.buffer_size, options.count);
		break;
	case RUN_SEND:
		printf("just send data to device, output to \"%s\"\nbuffer size = %u\ncount times = %u\n\n",
		       filename_out, options.buffer_size, options.count);
		res = just_send_test(*fd_out, options.buffer_size, options.count);
		break;
	case RUN_RECV:
		printf("just receive data from device, start read on 't', then stop on 'q' \ninput from \"%s\"\nbuffer size = %u\n\n",
		       filename_in, options.buffer_size);
		res = just_recv_test(*fd_in, options.buffer_size);
		break;
	}

	log_stop();

	if (fd[0] >= 0)
		l_close(fd[0]);
	if (fd[1] >= 0)
		l_close(fd[1]);

	return res;
}

