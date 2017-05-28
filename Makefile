test : test.c
	gcc -D_FILE_OFFSET_BITS=64 -o test test.c -lfuse
