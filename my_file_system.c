#define FUSE_USE_VERSION 26

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fuse.h>

static int fuse_getattr(const char* path, struct stat* stbuf);
static int fuse_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi);
static int fuse_mkdir(const char* path, mode_t mode);
static int fuse_open(const char* path, struct fuse_file_info* fi);
static int fuse_release(const char* path, struct fuse_file_info* fi);
static int fuse_read(const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info* fi);
static int fuse_write(const char* path, const char* buf, size_t size, off_t offset, struct fuse_file_info* fi);
static int fuse_create(const char* path, mode_t mode, struct fuse_file_info* fi);
//fuse_remove

static struct fuse_operations my_own_fs = {
	.getattr = fuse_getattr,
	.readdir = fuse_readdir,
	.mkdir = fuse_mkdir,
	.open = fuse_open,
	.release = fuse_release, //close
	.read = fuse_read,
	.write = fuse_write,
	.create = fuse_create,
	//remove
};

int main(int argc, char** argv) {
	return fuse_main(argc, argv, &my_own_fs, NULL);
}

static int fuse_open(const char* path, struct fuse_file_info* fi) {
	
}
