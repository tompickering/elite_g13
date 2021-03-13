all: elite

elite: elite.c
	$(CC) elite.c -o elite -lg13 -ljson-c
