/*
 * params.c
 *
 *  Created on: 26.12.2013
 *      Author: ahutsikau
 */

#include "params.h"
#include <string.h>

#define COUNT(_ARR_) (sizeof((_ARR_))/sizeof((_ARR_)[0]))

int get_program_options (program_options_t* options, int argc, const char* argv[])
{
        int res = -11;
        int free_arg_pos = 0;

        if (argc < 3)
                goto USAGE;

        options->run_type = RUN_TEST;
        options->files_number = 0;
        for (int i = 0; i < COUNT(options->filename); i++)
                options->filename[i] = 0;
        options->log_filename = 0;
        options->log_type = LOG_APPEND;
        options->buffer_size = 1024;
        options->count = 1;
        options->logstdout = 0;
        options->flow_ctrl_en = 0;
        options->iterm = 0;
        options->oterm = 0;


        for (int pos = 1; pos < argc; pos++) {
                if (argv[pos][0] == '-') {
                        char* tail = 0;
                        if (!strcmp(argv[pos], "--bs")) {
                                pos++;
                                options->buffer_size = strtol(argv[pos], &tail, 0);
                        } else if (!strcmp(argv[pos], "--count")) {
                                pos++;
                                options->count = strtol(argv[pos], &tail, 0);
                        } else if (!strcmp(argv[pos], "--log")) {
                                pos++;
                                //TODO: check argc
                                options->log_filename = argv[pos];
                        } else if (!strcmp(argv[pos], "--lA")) {
                                options->log_type = LOG_APPEND;
                        } else if (!strcmp(argv[pos], "--lT")) {
                                options->log_type = LOG_TRUNC;
                        } else if (!strcmp(argv[pos], "--logstdout")) {
                                options->logstdout = 1;
                        } else if (!strcmp(argv[pos], "--crtscts")) {
                                options->flow_ctrl_en = 1;
                        } else if (!strcmp(argv[pos], "--iterm")) {
                                options->iterm = 1;
                        } else if (!strcmp(argv[pos], "--oterm")) {
                                options->oterm = 1;
                        } else if (!strcmp(argv[pos], "--ioterm")) {
                                options->iterm = 1;
                                options->oterm = 1;
                        } else
                                goto USAGE;
                } else {
                        switch (free_arg_pos) {
                        case 0:
                                if (!strcmp(argv[pos], "test")) {
                                        options->run_type = RUN_TEST;
                                } else if (!strcmp(argv[pos], "ipipe")) {
                                        options->run_type = RUN_IPIPE;
                                } else if (!strcmp(argv[pos], "fpipe")) {
                                        options->run_type = RUN_FPIPE;
                                } else if (!strcmp(argv[pos], "open")) {
                                        options->run_type = RUN_OPEN;
                                } else if (!strcmp(argv[pos], "thread")) {
                                        options->run_type = RUN_THREAD;
                                } else if (!strcmp(argv[pos], "send")) {
                                        options->run_type = RUN_SEND;
                                } else if (!strcmp(argv[pos], "recv")) {
                                        options->run_type = RUN_RECV;
                                } else
                                        goto USAGE;
                                break;
                        case 1:
                                options->filename[options->files_number++] = argv[pos];
                                break;
                        case 2:
                                options->filename[options->files_number++] = argv[pos];
                                break;
                        default:
                                goto USAGE;
                        }
                        free_arg_pos++;
                }
        }
        if (options->files_number == 1 || options->files_number == 2) {
                res = 0;
                goto OUT;
        }
USAGE:
        printf("\nUsage: %s <run_type> <serial device> [<serial device>] [--bs <buffer size>] [--count <count times>] [--log <log filename>] [--lA] [--lT] \n\n", argv[0]);
        printf("  run_type:\n");
        printf("    open - program just openes the device file, but do not read data, start read on 't', then stop 'q'.\n");
        printf("    test - program will generate testing data, send it to output device and wait for reading it from input device to compare it with sended.\n");
        printf("    ipipe - instant pipe - program will read data from input device and send it to output device each time read call returns.\n");
        printf("    fpipe - fixed block pipe - program will read data from input device and send it to output device by block of defined size.\n");
        printf("    thread - read from input file in another thread, write in current\n");
        printf("    send - just send data into file \n");
        printf("    recv - just read from file \n");
        printf("\n");
        printf("  serial device - If defined one serial device - it will used as both input and output, otherwise first will be input and second will be output.\n\n");
        printf("  --bs <buffer size> - size of buffer to read and write from/to devices, default value is 1024 bytes\n\n");
        printf("  --count <count times> - number of buffers to send to device in test mode, default value is once (1 time)\n\n");
        printf("  --crtscts - enable RTS/CTS flow control on terminals \n\n");
        printf("  --iterm - input file is terminal \n\n");
        printf("  --oterm - output file is terminal \n\n");
        printf("  --ioterm - ianput and output files are terminals \n\n");
        printf("  --log <log filename> - \n\n");
        printf("  --logstdout - \n\n");
        printf("  logging mode:\n");
        printf("    --lA - append to existing log file.\n");
        printf("    --lT - truncate log file if it exists.\n");
        printf("    default is append\n");
        printf("\n");
OUT:
        return res;
}


