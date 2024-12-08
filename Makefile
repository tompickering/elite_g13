all: elite

elite: elite.c
	$(CC) -g elite.c keydefs.h -o elite -I/usr/include/lua5.4 -llua5.4 -lg13 -ljson-c -lX11
