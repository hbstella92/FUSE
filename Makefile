hello : hello.c
	gcc -D_FILE_OFFSET_BITS=64 -o hello hello.c -lfuse
