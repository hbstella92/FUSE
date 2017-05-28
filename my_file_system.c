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

#include "fuse.h"

#define BUFSIZE 256

typedef struct file {
	char* file_path;
	int fd;
	char* file_str;
	off_t file_offt;
	int str_size;
	int file_idx;
}file_t;

static int fuse_getattr(const char* path, struct stat* stbuf) {
	memset(stbuf, 0, sizeof(struct stat));

	if((strcmp(path, "/") == 0) || (strcmp(path, "/.") == 0)) {
//				(strcmp(path, "/..") == 0)) {
		stbuf->st_mode = S_IFDIR|0755;
		stbuf->st_nlink = 2;
		stbuf->st_uid = getuid();
		stbuf->st_gid = getgid();

		return 0;
	}

	if(path != ((const char *)0)) {
		stbuf->st_mode = S_IFREG|0666;
		stbuf->st_nlink = 1;
		stbuf->st_uid = getuid();
		stbuf->st_gid = getgid();
		stbuf->st_size = BUFSIZE;

		return 0;
	}
	else
		return -ENOENT;

}

static int fuse_opendir(const char* path, struct fuse_file_info* fi) {
	DIR* dp;

	if(path == ((const char*)0))
			return -ENOENT;

	dp = opendir(path);
	if(dp == NULL)
		return -ENOENT;

	return 0;
}

static int fuse_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset,
		struct fuse_file_info* fi) {
	struct stat* stbuf;
	memset(stbuf, 0, sizeof(struct stat));
	stat(path, stbuf);
	
	if((stbuf->st_mode & S_IFDIR) == S_IFDIR)
		printf("ddddddddddddddddddddddddddddddddddddddddd\n");
	if((stbuf->st_mode & S_IFREG) == S_IFREG)
		printf("rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr\n");
/*	
	DIR* dp;
	struct dirent* d_entry;
	struct stat* stbuf;
	
	dp = opendir(path);
	if(dp == NULL)
		return -ENOENT;

	while((d_entry = readdir(dp)) != NULL) {
		memset(stbuf, 0, sizeof(struct stat));

		stbuf->st_ino = d_entry->d_ino;
		stbuf->st_mode = d_entry->d_type << 12;
		stbuf->st_size = d_entry->d_reclen;
printf("DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD_entry name is = %s \n", d_entry->d_name);
		if(filler(buf, d_entry->d_name, NULL, 0))
			break;
	}

	closedir(dp);
*/
	return 0;
}

/*	if(path == ((const char*)0))
		return -ENOENT;

	stat(path, stbuf);

	if(stbuf->st_mode != S_IFDIR)
		return -ENOTDIR;

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	filler(buf, path+1, NULL, 0);

	return 0;
*/

static int fuse_open(const char* path, struct fuse_file_info* fi) {
	struct stat* stbuf;

	if(path == ((const char *)0))
		return -ENOENT;

	if(fuse_getattr(path, stbuf) != ENOENT) {
		fi->flags = O_CREAT|O_WRONLY|O_TRUNC;
		fi->fh = (uint64_t)open(path, fi->flags);

		if(fi->fh == -1)
			return -ENOENT;
	}

	return 0;
//	myfile->fd = fopen(path, "r+");
}

static int fuse_read(const char* path, char* buf, size_t size, off_t offset,
		struct fuse_file_info* fi) {
	size_t nread;
	char* tmp_buffer;

	if(path == ((const char *)0))
		return -ENOENT;

	tmp_buffer = (char*)malloc(BUFSIZE);

	if((nread = read(fi->fh, tmp_buffer, BUFSIZE)) < 0)
		printf("READ ERROR!!\n");

	if(offset < nread) {
		if(offset + size > nread) {
			size = nread - offset;
		}
		memcpy(buf, tmp_buffer + offset, size);
	}
	else
		nread = 0;

	return nread;
}

static struct fuse_operations fuse_ops = {
	.getattr = fuse_getattr,
	.readdir = fuse_readdir,
	.open = fuse_open,
	.read = fuse_read,
};

int main(int argc, char** argv) {
	return fuse_main(argc, argv, &fuse_ops, NULL);
}

