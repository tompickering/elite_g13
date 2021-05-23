all: elite

elite: elite.c
	$(CC) -g elite.c keydefs.h -o elite -lg13 -ljson-c -lX11
