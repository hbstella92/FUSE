#define FUSE_USE_VERSION 26

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fuse.h>

#define BUF_SIZE 256
#define LIST_SIZE 5

typedef struct my_fuse_file {
	const char* filename;
	const char* msg;
}my_fuse_file_t;

static const char* fuse_path = "/hanbyeol";
static my_fuse_file_t file_list[] = {
	{"DATA_STRUCTURE", "data_structure\n"},
	{"UNIX_PROGRAMMING", "unix_programming\n"},
	{"COMPUTER_STRUCTURE", "computer_structure\n"},
	{"SYSTEM_PROGRAMMING", "system_programming\n"},
	{"ALGORITHM", "algorithm\n"}
};

static int in_file_list(const char*);
static int fuse_getattr(const char* path, struct stat* stbuf);
static int fuse_mkdir(const char* path, mode_t mode);
static int fuse_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset,
		struct fuse_file_info* fi);
static int fuse_open(const char* path, struct fuse_file_info* fi);
static int fuse_read(const char* path, char* buf, size_t size, off_t offset,
		struct fuse_file_info* fi);

static int fuse_getattr(const char* path, struct stat* stbuf) {
	int idx = 0;

	memset(stbuf, 0, sizeof(struct stat));

	if(!strcmp(path, "/")) {
		stbuf->st_mode = S_IFDIR|0755;
		stbuf->st_nlink = 2;
	}
	else if(!strcmp(path, file_list[idx].filename)) {
		stbuf->st_mode = S_IFREG|0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(file_list[idx].msg);
	}
	else {
		return -ENOENT;
	}

/*	while(file_list[idx].filename != NULL) {
		if(!strcmp(path, file_list[idx].filename)) {
			stbuf->st_mode = S_IFREG|0444;
			stbuf->st_nlink = 1;
			stbuf->st_size = strlen(file_list[idx].msg);
		}
		else {
			return -ENOENT;
		}

		idx++;
	}
*/
/*	while(idx != LIST_SIZE) {
		if(!strcmp(path, file_list[idx].filename)) {
			stbuf->st_mode = S_IFREG|0444;
			stbuf->st_nlink = 1;
			stbuf->st_size = strlen(file_list[idx].msg);
		}
		else {
			return -ENOENT;
		}
		idx++;
	}
*/
	return 0;
}

static int fuse_mkdir(const char* path, mode_t mode) {
	if(mkdir(path, mode) == -1)
		return -ENOSYS;

}

static int fuse_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset,
		struct fuse_file_info* fi) {
	int idx = 0;

	if(strcmp(path, "/") != 0)
		return -ENOENT;

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	filler(buf, fuse_path+1, NULL, 0);
	
	while(idx != LIST_SIZE) {
		filler(buf, file_list[idx++].filename, NULL, 0);
	}

	return 0;
}

static int fuse_open(const char* path, struct fuse_file_info* fi) {
	int idx = 0;
	
	if(strcmp(path, fuse_path) != 0)
		return -ENOENT;

	while(idx != LIST_SIZE) {
		if(strcmp(path, file_list[idx++].filename) != 0)
			return -ENOENT;
	}

	if((fi->flags & O_RDONLY) != O_RDONLY)
		return -EACCES;

	return 0;
}

static int fuse_read(const char* path, char* buf, size_t size, off_t offset,
		struct fuse_file_info* fi) {
	size_t len;

	if(strcmp(path, fuse_path) != 0)
		return -ENOENT;

	len = strlen(file_list[LIST_SIZE-1].msg);

	if(offset < len) {
		if(offset + size > len) {
			size = len - offset;
		}
		memcpy(buf, file_list[LIST_SIZE-1].msg + offset, size);
	}
	else {
		size = 0;
	}

	return size;
}

static struct fuse_operations fuse_op = {
	.getattr = fuse_getattr,
	.mkdir = fuse_mkdir,
	.readdir = fuse_readdir,
	.open = fuse_open,
	.read = fuse_read,
};

int main(int argc, char** argv) {
	return fuse_main(argc, argv, &fuse_op, NULL);
}

static int in_file_list(const char* path) {
	char* fname;

	fname = (char*)malloc(BUF_SIZE);

	for(int i=0; i<LIST_SIZE; i++) {
		memset(fname, 0, BUF_SIZE);
		strcpy(fname, file_list[i].filename);

		if(!strcmp(path, fname))
			continue;
		else
			return -ENOENT;
	}

	free(fname);
	return 0;
}
