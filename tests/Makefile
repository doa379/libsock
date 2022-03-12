LOCAL = ../..
LIBSPATH = -L $(LOCAL)/libsock -Wl,-R$(LOCAL)/libsock '-Wl,-R$$ORIGIN' -L /usr/local/lib
INCS = -I /usr/local/include -I $(LOCAL)/
LIBS =

SRC0 = http.c
OBJ0 = $(SRC0:.c=.o)
SRC1 = keep_alive.c
OBJ1 = $(SRC1:.c=.o)
SRC2 = bio_client.c
OBJ2 = $(SRC2:.c=.o)
SRC3 = https.c
OBJ3 = $(SRC3:.c=.o)

CC = clang
CC_FLAGS = -std=c99 -Wall -fPIE -fPIC -pedantic
REL_CFLAGS = -O3
REL_LDFLAGS = -s
DBG_CFLAGS = -g
DBG_LDFLAGS =

CFLAGS = $(REL_CFLAGS)
LDFLAGS = $(REL_LDFLAGS) $(LIBSPATH)

all: http keep_alive bio_client https

.c.o:
	@echo CC $<
	@$(CC) $(CC_FLAGS) $(CFLAGS) $(INCS) -c $<

http: $(OBJ0)
	@echo CC -o $@
	@$(CC) $(OBJ0) $(LDFLAGS) -l sock -o $@

keep_alive: $(OBJ1)
	@echo CC -o $@
	@$(CC) $(OBJ1) $(LDFLAGS) -l sock -o $@

bio_client: $(OBJ2)
	@echo CC -o $@
	@$(CC) $(OBJ2) $(LDFLAGS) -l ssl -l crypto -o $@

https: $(OBJ3)
	@echo CC -o $@
	@$(CC) $(OBJ3) $(LDFLAGS) -l sock -o $@

clean:
	@echo Cleaning...
	@rm -f $(OBJ0) $(OBJ1) $(OBJ2) $(OBJ3)
	@rm -f http keep_alive bio_client https
