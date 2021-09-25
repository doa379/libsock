LOCAL = ../..
LIBSPATH = -L ${LOCAL}/libsock -Wl,-R$(LOCAL)/libsock '-Wl,-R$$ORIGIN' -L /usr/local/lib
INCS = -I /usr/local/include -I ${LOCAL}/
LIBS =

SRC0 = http.c
OBJ0 = ${SRC0:.c=.o}
SRC1 = bio_client.c
OBJ1 = ${SRC1:.c=.o}

CC = cc
RELEASE_CFLAGS = -std=c99 -c -Wall -fPIE -fPIC -pedantic -O3 ${INCS}
DEBUG_CFLAGS = -std=c99 -c -Wall -fPIE -fPIC -pedantic -g ${INCS}
CFLAGS = ${DEBUG_CFLAGS}
LDFLAGS += ${LIBSPATH}

all: http bio_client

.c.o:
	@echo CC $<
	@${CC} ${CFLAGS} $<

http: ${OBJ0}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ0} ${LDFLAGS} -l sock

bio_client: ${OBJ1}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ1} ${LDFLAGS} -l ssl -l crypto

clean:
	@echo Cleaning
	@rm -f ${OBJ0} ${OBJ1}
	@rm -f http bio_client
