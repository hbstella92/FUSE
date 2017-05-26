#define FUSE_USE_VERSION 26

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fuse.h>

#define LISTSIZE 5

static const char* fs_path = "/hanbyeol";
static const char* message = "MESSAGE!\n";

static char* file_list[] = {"DATA_STRUCTURE", "UNIX_PROGRAMMING", "COMPUTER_STRUCTURE",
	"SYSTEM_PROGRAMMING", "ALGORITHM"};

static int in_list(const char* filename) {
	char* cmpname;

	for(int i=0; i<LISTSIZE; i++) {
		strcpy(cmpname, file_list[i]);

		if(!strcmp(filename, cmpname))
			continue;
		else
			return -ENOENT;
	}
	return 0;
}

static int fs_getattr(const char* path, struct stat* stbuf);
static int fs_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset,
		struct fuse_file_info* fi);
//static int fuse_mkdir(const char* path, mode_t mode);
static int fs_open(const char* path, struct fuse_file_info* fi);
//static int fuse_release(const char* path, struct fuse_file_info* fi);
static int fs_read(const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info* fi);
//static int fuse_write(const char* path, const char* buf, size_t size, off_t offset,
//		struct fuse_file_info* fi);
//static int fuse_create(const char* path, mode_t mode, struct fuse_file_info* fi);
//fuse_remove

static struct fuse_operations my_fs = {
	.getattr = fs_getattr,
	.readdir = fs_readdir,
//	.mkdir = fuse_mkdir,
	.open = fs_open,
//	.release = fuse_release, //close
	.read = fs_read,
//	.write = fuse_write,
//	.create = fuse_create,
//  remove
};

int main(int argc, char** argv) {
	return fuse_main(argc, argv, &my_fs, NULL);
}

static int fs_getattr(const char* path, struct stat* stbuf) {
	memset(stbuf, 0, sizeof(struct stat));
	
	if(!strcmp(path, "/")) {
		stbuf->st_mode = S_IFDIR|0755;
		stbuf->st_nlink = 2;
//		stbuf->st_uid = getuid();
	}
	else if(!strcmp(path, fs_path)) {
//	else if(in_list(path) == 0) {
		stbuf->st_mode = S_IFREG|0444;
		stbuf->st_nlink = 1;
//		stbuf->st_uid = getuid();
	}
	else {
		return -ENOENT;
	}
	
	return 0;
}

static int fs_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset,
		struct fuse_fill_info* fi) {
	if(!strcmp(path, "/"))
		return -ENOENT;

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	filler(buf, fs_path+1, NULL, 0);
//	for(int i=0; i<LISTSIZE; i++) {
//		filler(buf, file_list[i], NULL, 0);
//	}

	return 0;
}

static int fs_open(const char* path, struct fuse_file_info* fi) {
	if(!strcmp(path, fuse_path))
		return -ENOENT;

	//
	if((fi->flags & 3) != O_RDONLY)
		return -EACCES;

	return 0;
}

static int fs_read(const char* path, char* buf, size_t size, off_t offset,
		struct fuse_file_info* fi) {
	size_t len;

	if(!strcmp(path, fuse_path))
		return -ENOENT;

	len = strlen(message);

	if(offset < len) {
		if(offset + size > len) {
			size = len - offset;
		}
		memcpy(buf, message+offset, size);
	}
	else {
		size = 0;
	}

	return size;
}
