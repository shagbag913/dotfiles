CC	:= clang
LFLAGS	:= -lm -lasound -lpthread -lnotify
CFLAGS	:= -Wall -Wextra -Wpedantic -O3 -flto

INCLUDE	:= $(shell pkg-config --cflags glib-2.0)
INCLUDE += $(shell pkg-config --cflags gdk-pixbuf-2.0)

default:
	$(CC) $(LFLAGS) $(CFLAGS) $(INCLUDE) -o lemonbar_utils lemonbar_utils.c
