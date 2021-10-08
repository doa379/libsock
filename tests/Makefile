LOCAL = ../..
LIBSPATH = -L ${LOCAL}/libsock -Wl,-R$(LOCAL)/libsock '-Wl,-R$$ORIGIN' -L /usr/local/lib
INCS = -I /usr/local/include -I ${LOCAL}/
LIBS =

SRC0 = http.c
OBJ0 = ${SRC0:.c=.o}
SRC1 = http_ka.c
OBJ1 = ${SRC1:.c=.o}
SRC2 = bio_client.c
OBJ2 = ${SRC2:.c=.o}
SRC3 = https.c
OBJ3 = ${SRC3:.c=.o}

CC = clang
REL_CFLAGS = -std=c99 -c -Wall -fPIE -fPIC -pedantic -O3 ${INCS}
DEB_CFLAGS = -std=c99 -c -Wall -fPIE -fPIC -pedantic -g ${INCS}
CFLAGS = ${DEB_CFLAGS}
LDFLAGS += ${LIBSPATH}

all: http http_ka bio_client https

.c.o:
	@echo CC $<
	@${CC} ${CFLAGS} $<

http: ${OBJ0}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ0} ${LDFLAGS} -l sock

http_ka: ${OBJ1}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ1} ${LDFLAGS} -l sock

bio_client: ${OBJ2}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ2} ${LDFLAGS} -l ssl -l crypto

https: ${OBJ3}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ3} ${LDFLAGS} -l sock

clean:
	@echo Cleaning
	@rm -f ${OBJ0} ${OBJ1} ${OBJ2} ${OBJ3}
	@rm -f http http_ka bio_client https
