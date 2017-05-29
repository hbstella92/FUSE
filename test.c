#define FUSE_USE_VERSION 26

#include <sys/statvfs.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/xattr.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

#include "fuse.h"

#define FILE_NO 10
#define FILE_SIZE 256

typedef struct tfile {
	char* hello_path;
	unsigned char* hello_str;
}tfile;

static tfile** files;
static int file_no = 0;

static int hello_getattr(const char* path, struct stat* stbuf) {
	int idx = 0;
	memset(stbuf, 0, sizeof(struct stat));

	if(strcmp(path, "/") == 0) {
		stbuf->st_ino = 1;
		stbuf->st_mode = S_IFDIR|0755;
		stbuf->st_nlink = 2;
		stbuf->st_uid = getuid();
		stbuf->st_gid = getgid();
		stbuf->st_atime = time(NULL);
		stbuf->st_mtime = time(NULL);
		
		return 0;
	}
	
	while(files[idx]->hello_path != ((const char *)0)) {
		if(strcmp(path, files[idx]->hello_path) == 0)
			break;
		++idx;
	}

	if(files[idx]->hello_path == ((const char *)0))
		return -ENOENT;

	stbuf->st_mode = S_IFREG|0666;
	stbuf->st_nlink = 1;
	stbuf->st_size = FILE_SIZE;
	stbuf->st_uid = getuid();
	stbuf->st_gid = getgid();
	stbuf->st_atime = time(NULL);
	stbuf->st_mtime = time(NULL);

	return 0;
}

static int hello_getxattr(const char* path, const char* name, char* value, size_t size) {
	int res;

	res = lgetxattr(path, name, value, size);
	if(res == -1)
		return -1;
	
	return res;
}

static int hello_setxattr(const char* path, const char* name, const char* value,
		size_t size, int flags) {

	return 0;
}

static int hello_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset,
		struct fuse_file_info* fi) {
	int idx = 0;

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

	while(file_no != idx) {
		filler(buf, files[idx]->hello_path+1, NULL, 0);
		++idx;
	}

	return 0;
}

static int hello_open(const char* path, struct fuse_file_info* fi) {
	int idx = 0;

	while(files[idx]->hello_path != ((const char *)0)) {
		if(strcmp(path, files[idx]->hello_path) == 0)
			break;
		++idx;
	}

	if(files[idx]->hello_path != ((const char *)0)) {
//		fi->flags = O_CREAT|O_WRONLY|O_TRUNC;
		fi->flags = O_RDWR;
	}
	
	fi->flags = O_CREAT|O_WRONLY|O_EXCL;

	return 0;
}

static int hello_create(const char* path, mode_t mode, struct fuse_file_info* fi) {
	tfile* newfile;
	int nidx = 0;

	newfile = (tfile*)malloc(sizeof(struct tfile));
	newfile->hello_path = (char*)malloc(256);
	newfile->hello_str = (char*)malloc(256);
	
	strcpy(newfile->hello_path, path);
	newfile->hello_str = NULL;
	fi->flags = O_CREAT|O_WRONLY|O_TRUNC;
	mode = 0666;
	
	while(files[nidx] != ((struct tfile *)NULL))
		nidx++;
	
	files[nidx-1] = newfile;
	file_no = nidx;


	return 0;
}

static int hello_read(const char* path, char* buf, size_t size, off_t offset,
		struct fuse_file_info* fi) {
	int idx = 0;
	off_t nread;

	if(file_no > 0) {
		while(files[idx]->hello_path != ((const char *)0)) {
			if(strcmp(path, files[idx]->hello_path) == 0)
				break;
			++idx;
		}

		//	if(path == ((const char *)0))
		//		return -ENOENT;

		if(strcmp(path, files[idx]->hello_path) == 0) {
			nread = strlen(files[idx]->hello_str);

			if(offset > nread)
				return 0;

			if(offset + (off_t)size > nread) {
				size = (size_t)(nread - offset);
			}
			memcpy(buf, files[idx]->hello_str + offset, size);
		}

		return nread;
	}
	else
		return -ENOENT;
}

static int hello_write(const char* path, const char* buf, size_t size, off_t offset,
		struct fuse_file_info* fi) {

}

static int hello_utimens(const char* path, const struct timespec ts[2]) {
	struct utimbuf ubuf;

	ubuf.actime = ts[0].tv_sec;
	ubuf.modtime = ts[1].tv_sec;

	return 0;
}

static struct fuse_operations hello_oper = {
	.getattr = hello_getattr,
#ifdef HAVE_SETXATTR
	.getxattr = hello_getxattr,
	.setxattr = hello_setxattr,
#endif
	.readdir = hello_readdir,
	.open = hello_open,
	.create = hello_create,
	.read = hello_read,
	.write = hello_write,
	.utimens = hello_utimens,
};

int main(int argc, char** argv) {
	files = (tfile**)malloc(sizeof(struct tfile*));
	
	for(int i=0; i<FILE_NO; i++) {
		*files = (tfile*)malloc(sizeof(struct tfile));
	}

	return fuse_main(argc, argv, &hello_oper, &files);
}
