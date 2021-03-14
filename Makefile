all: elite

elite: elite.c
	$(CC) -g elite.c -o elite -lg13 -ljson-c -lX11
