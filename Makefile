LOCAL = ..
LIBSPATH = -L ${LOCAL}/libsock -Wl,-R$(LOCAL)/libsock '-Wl,-R$$ORIGIN' -L /usr/local/lib
INCS = -I /usr/local/include -I ${LOCAL}/
LIBS =

SRC_LIBSOCK = sock.c
OBJ_LIBSOCK = ${SRC_LIBSOCK:.c=.o}

CC = clang
REL_CFLAGS = -std=c99 -c -Wall -fPIE -fPIC -pedantic -O3 ${INCS}
DEB_CFLAGS = -std=c99 -c -Wall -fPIE -fPIC -pedantic -g ${INCS}
CFLAGS = ${DEB_CFLAGS}
LDFLAGS += ${LIBSPATH}

all: libsock.so

.c.o:
	@echo CC $<
	@${CC} ${CFLAGS} $<

libsock.so: ${OBJ_LIBSOCK}
	@echo CC -o $@
	@${CC} -shared -o $@ ${OBJ_LIBSOCK} ${LDFLAGS} ${LIBS}

clean:
	@echo Cleaning
	@rm -f ${OBJ_LIBSOCK}
	@rm -f libsock.so
