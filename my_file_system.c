#define FUSE_USE_VERSION 26

#include <sys/statvfs.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <fuse.h>

typedef struct file {
	const char* name;
	char* msg;
}file_t;

static file_t myfiles[] = {};
static int myfile_fd;

static int fuse_getattr(const char* path, struct stat* stbuf) {
	file_t fusefile;
	int idx = 0;
	
	memset(stbuf, 0, sizeof(struct stat));

	if((strcmp(path, "/") == 0) || (strcmp(path+1, ".") == 0) ||
				(strcmp(path+1, "..") == 0)) {
		stbuf->st_mode = S_IFDIR|0755;
		stbuf->st_nlink = 2;
		stbuf->st_uid = getuid();
		stbuf->st_gid = getgid();
	}
	else {
		stbuf->st_mode = S_IFREG|0666;
		stbuf->st_nlink = 1;
		stbuf->st_uid = getuid();
		stbuf->st_gid = getgid();
//		stat(fusefile.name, stbuf);
		stbuf->st_size = strlen("TEST");
//		stbuf->st_size = strlen(fusefile.msg);
	}

	return 0;
}

static int fuse_opendir(const char* path, struct fuse_file_info* fi) {
	DIR* dir;

	if(path == ((const char*)0))
			return -ENOENT;

	if((strcmp(path, "/") == 0) || (strcmp(path+1, ".") == 0) ||
				(strcmp(path+1, "..") == 0)) {
		dir = opendir(path);

		if(dir == ((DIR*)0))
			return -ENOENT;

		return 0;
	}
	else
		return -ENOTDIR;
}

static int fuse_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset,
		struct fuse_file_info* fi) {
	if(path == ((const char*)0))
		return -ENOENT;

//	if(strcmp(path, "/") != 0)
//		return -ENOENT;

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	filler(buf, path+1, NULL, 0);

	return 0;
}

//static int fuse_mkdir(const char* path, mode_t mode);

static int fuse_open(const char* path, struct fuse_file_info* fi) {
	if(path == ((const char*)0))
		return -ENOENT;

	myfile_fd = fi->fh;

	return 0;
}

//static int fuse_release(const char* path, struct fuse_file_info* fi);

static int fuse_read(const char* path, char* buf, size_t size, off_t offset,
		struct fuse_file_info* fi) {
	file_t myfile;
	size_t nread;

	if(path == ((const char*)0))
		return -ENOENT;

	myfile.msg = (char*)malloc(size);
	read(myfile_fd, myfile.msg, size);
	nread = strlen(myfile.msg);
	
	if(offset < nread) {
		if(offset + size > nread) {
			size = nread - offset;
		}
		memcpy(buf, myfile.msg + offset, size);
	}
	else
		nread = 0;

	free(myfile.msg);

	return nread;
}

//static int fuse_write(const char* path, const char* buf, size_t size, off_t offset,
//		struct fuse_file_info* fi);

//static int fuse_create(const char* path, mode_t mode, struct fuse_file_info* fi);

//fuse_remove

static struct fuse_operations fuse_ops = {
	.getattr = fuse_getattr,
//	.opendir = fuse_opendir,
	.readdir = fuse_readdir,
//	.mkdir = fuse_mkdir,
	.open = fuse_open,
//	.release = fuse_release, //close
	.read = fuse_read,
//	.write = fuse_write,
//	.create = fuse_create,
	//remove
};

int main(int argc, char** argv) {
	return fuse_main(argc, argv, &fuse_ops, NULL);
}

