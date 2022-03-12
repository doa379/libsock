LOCAL = ..
LIBSPATH = -L $(LOCAL)/libsock -Wl,-R$(LOCAL)/libsock '-Wl,-R$$ORIGIN' -L /usr/local/lib
INCS = -I /usr/local/include -I $(LOCAL)/
LIBS = -l ssl -l crypto

SRC_LIBSOCK = sock.c ssl.c
OBJ_LIBSOCK = $(SRC_LIBSOCK:.c=.o)

CC = clang
CC_FLAGS = -std=c99 -Wall -fPIE -fPIC -pedantic
REL_CFLAGS = -O3
REL_LDFLAGS = -s
DBG_CFLAGS = -g -O0
DBG_LDFLAGS =

CFLAGS = $(REL_CFLAGS)
LDFLAGS = $(REL_LDFLAGS)

all: libsock.so

.c.o:
	@echo CC $<
	@$(CC) $(CC_FLAGS) $(CFLAGS) $(INCS) -c $<

libsock.so: $(OBJ_LIBSOCK)
	@echo CC -o $@
	@$(CC) -shared -o $@ $(OBJ_LIBSOCK) $(LDFLAGS) $(LIBSPATH) $(LIBS)

clean:
	@echo Cleaning
	@rm -f $(OBJ_LIBSOCK)
	@rm -f libsock.so
