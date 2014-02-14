/*
 * params.h
 *
 *  Created on: 26.12.2013
 *      Author: ahutsikau
 */

#ifndef PARAMS_H_
#define PARAMS_H_

#include <stdlib.h>
#include "logger.h"
#include <stdio.h>

typedef enum RUN_TYPES_ {
        RUN_TEST,
        RUN_IPIPE,
        RUN_FPIPE,
        RUN_OPEN,
        RUN_THREAD,
        RUN_SEND,
        RUN_RECV
} RUN_TYPES_T;

typedef struct program_options_ {
        RUN_TYPES_T run_type;
        int files_number;
        const char* filename[2];
        const char* log_filename;
        LOG_START_TYPE_T log_type;
        int logstdout;
        uint32_t buffer_size;
        uint32_t count;
        int flow_ctrl_en;
        int iterm;
        int oterm;
} program_options_t;

int get_program_options (program_options_t* options, int argc, const char* argv[]);

#endif /* PARAMS_H_ */
