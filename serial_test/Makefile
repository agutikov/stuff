
ifndef INSTALL_PREFIX
    INSTALL_PREFIX=/home/ahutsikau/work/nextgen/export/upload/tests
endif

HOST=x86_64-pc-linux-gnu
WIN32_HOST=i686-pc-mingw32
TARGET=arm-none-linux-gnueabi

CC=gcc
CXX=g++

CFLAGS= -Wall -std=gnu99
LDFLAGS= -pthread

TEST_CFLAGS= ${CFLAGS} -ggdb -DCOMPILING_TEST
TEST_LDFLAGS= ${LDFLAGS}

ifndef arm_none_linux_gnueabi_SYSROOT
    $(error Please define arm_none_linux_gnueabi_SYSROOT variable)
endif
TARGET_CFLAGS= --sysroot=${arm_none_linux_gnueabi_SYSROOT}

APPS=serial_test tty_test
WIN32_APPS=serial_test
LIB=logger params
TESTS=logger
LINUX_LIB=linux/utils
WIN32_LIB=win/utils

HOST_EXECUTABLES=${APPS:%=${HOST}/%}
WIN32_HOST_EXECUTABLES=${WIN32_APPS:%=${WIN32_HOST}/%.exe}
TARGET_EXECUTABLES=${APPS:%=${TARGET}/%}

TEST_HOST_EXECUTABLES=${TESTS:%=test/${HOST}/%_test}
TEST_TARGET_EXECUTABLES=${TESTS:%=test/${TARGET}/%_test}
TEST_WIN32_EXECUTABLES=${TESTS:%=test/${WIN32_HOST}/%_test.exe}

LINUX_LIB_HOST_OBJS=${LIB:%=${HOST}/%.o} ${LINUX_LIB:%=${HOST}/%.o}
LINUX_LIB_TARGET_OBJS=${LIB:%=${TARGET}/%.o} ${LINUX_LIB:%=${TARGET}/%.o}
WIN32_LIB_HOST_OBJS=${LIB:%=${WIN32_HOST}/%.o} ${WIN32_LIB:%=${WIN32_HOST}/%.o}

OBJS=${APPS:%=${HOST}/%.o} ${WIN32_APPS:%=${WIN32_HOST}/%.o} ${APPS:%=${TARGET}/%.o} \
${LINUX_LIB_HOST_OBJS} ${LINUX_LIB_TARGET_OBJS} ${WIN32_LIB_HOST_OBJS}
DEPS=${OBJS:.o=.d}

DIRS=${HOST} ${WIN32_HOST} ${TARGET} test/${HOST} test/${TARGET} test/${WIN32_HOST} \
${HOST}/linux ${WIN32_HOST}/win ${TARGET}/linux

.PHONY : all
all : ${DIRS} ${HOST_EXECUTABLES} ${WIN32_HOST_EXECUTABLES} ${TARGET_EXECUTABLES} tests

rebuild : clean all

.PHONY : tests
tests : ${TEST_HOST_EXECUTABLES} ${TEST_TARGET_EXECUTABLES} ${TEST_WIN32_EXECUTABLES}

.PHONY : clean
clean :
	rm -rf ${HOST} ${WIN32_HOST} ${TARGET} test

install : all ${INSTALL_PREFIX}
	mkdir -p ${INSTALL_PREFIX}/${TARGET}
	mkdir -p ${INSTALL_PREFIX}/${HOST}
	mkdir -p ${INSTALL_PREFIX}/${WIN32_HOST}
	cp -v ${HOST_EXECUTABLES} ${INSTALL_PREFIX}/${HOST}/
	cp -v ${TARGET_EXECUTABLES} ${INSTALL_PREFIX}/${TARGET}/
	cp -v ${WIN32_HOST_EXECUTABLES} ${INSTALL_PREFIX}/${WIN32_HOST}/
	mkdir -p /home/ftp/serial_test/${TARGET}
	mkdir -p /home/ftp/serial_test/${HOST}
	mkdir -p /home/ftp/serial_test/${WIN32_HOST}
	cp -v ${HOST_EXECUTABLES} /home/ftp/serial_test/${HOST}/
	cp -v ${TARGET_EXECUTABLES} /home/ftp/serial_test/${TARGET}/
	cp -v ${WIN32_HOST_EXECUTABLES} /home/ftp/serial_test/${WIN32_HOST}/


${HOST_EXECUTABLES} : ${HOST}/% : ${HOST}/%.o ${LINUX_LIB_HOST_OBJS}
	${HOST}-gcc ${LDFLAGS} -o $@ $^

${WIN32_HOST_EXECUTABLES} : ${WIN32_HOST}/%.exe : ${WIN32_HOST}/%.o ${WIN32_LIB_HOST_OBJS}
	${WIN32_HOST}-gcc -static -o $@ $^

${TARGET_EXECUTABLES} : ${TARGET}/% : ${TARGET}/%.o ${LINUX_LIB_TARGET_OBJS}
	${TARGET}-gcc ${TARGET_CFLAGS} ${LDFLAGS} -o $@ $^

${TEST_HOST_EXECUTABLES} : test/${HOST}/%_test : %.c test/${HOST}
	${HOST}-gcc ${TEST_CFLAGS} ${TEST_LDFLAGS} -o $@ $< ${LINUX_LIB:%=%.c}

${TEST_TARGET_EXECUTABLES} : test/${TARGET}/%_test : %.c test/${TARGET}
	${TARGET}-gcc ${TARGET_CFLAGS} ${TEST_CFLAGS} ${TEST_LDFLAGS} -o $@ $< ${LINUX_LIB:%=%.c}

${TEST_WIN32_EXECUTABLES} : test/${WIN32_HOST}/%_test.exe : %.c test/${WIN32_HOST}
	${WIN32_HOST}-gcc ${CFLAGS} ${TEST_CFLAGS} -o $@ $< ${WIN32_LIB:%=%.c}

-include ${DEPS}

${HOST}/%.o : %.c
	${HOST}-gcc -MD -c -fPIC ${CFLAGS} ${CFLAGS_EXT} -o $@ $<

${WIN32_HOST}/%.o : %.c
	${WIN32_HOST}-gcc -D_WIN32_WINNT=0x0600 -DWINVER=0x0600 -MD -c ${CFLAGS} -o $@ $<

${TARGET}/%.o : %.c
	${TARGET}-gcc ${TARGET_CFLAGS} -MD -c -fPIC ${CFLAGS} ${CFLAGS_EXT} -o $@ $<

${DIRS} :
	mkdir -p $@

${INSTALL_PREFIX} :
	mkdir -p $@
