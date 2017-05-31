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

#define MAX_FILE_NO 10
#define MAX_NAME 256
#define FILE_SIZE 256

typedef struct tdir {
	char* dpath;
	int inode;
//	files* f_ptr;
//	tdir* d_ptr;
}tdir;

typedef struct tfile {
	char* hello_path;
	unsigned char* hello_str;
}tfile;

static struct tdir d_entry[MAX_FILE_NO];
static tfile** files;
static int dir_no = 0;
static int file_no = 0;

static int hello_getattr(const char* path, struct stat* stbuf) {
	int idx = 0;
	int d_idx = 0;

	memset(stbuf, 0, sizeof(struct stat));

	while(d_entry[d_idx].dpath != ((const char *)0)) {
		if(strcmp(path, d_entry[d_idx].dpath) == 0) {
			break;
		}
		++d_idx;
	}
	
	if(d_entry[d_idx].dpath == ((const char *)0))
		return -ENOENT;

	if(strcmp(path, d_entry[d_idx].dpath) == 0) {	
		stbuf->st_ino = 1 + d_idx;
		stbuf->st_mode = S_IFDIR|0755;
		stbuf->st_nlink = 2;
		stbuf->st_uid = getuid();
		stbuf->st_gid = getgid();
		stbuf->st_atime = time(NULL);
		stbuf->st_mtime = time(NULL);
		
		return 0;
	}
		
	while(files[idx]->hello_path != ((const char *)0)) {
		if(strcmp(path, files[idx]->hello_path) == 0) {
			break;
		}
		++idx;
	}

	if(files[idx]->hello_path == ((const char *)0))
		return -ENOENT;
	else {
		stbuf->st_mode = S_IFREG|0666;
		stbuf->st_nlink = 1;
		stbuf->st_size = FILE_SIZE;
		stbuf->st_uid = getuid();
		stbuf->st_gid = getgid();
		stbuf->st_atime = time(NULL);
		stbuf->st_mtime = time(NULL);

		return 0;
	}

	return 0;
}

/*
static int hello_setattr(const char* path, struct stat* stbuf) {


	return 0;
}
*/
static int hello_getxattr(const char* path, const char* name, char* value, size_t size) {
	int res;

	if((res = lgetxattr(path, name, value, size)) == -1)
		return -1;

	return res;
}

static int hello_setxattr(const char* path, const char* name, const char* value,
		size_t size, int flags) {

	return 0;
}

static int hello_mkdir(const char* path, mode_t mode) {
	int d_idx = 2;
	
	for(int idx=0; idx<file_no; idx++) {
		if(strcmp(path, files[idx]->hello_path) == 0)
			return -EEXIST;
	}

	for(int a=2; a<dir_no+2; a++) {
		if(strcmp(path, d_entry[a].dpath) == 0)
			return -EEXIST;
		++d_idx;
	}

	d_entry[d_idx].dpath = (char*)malloc(MAX_NAME);
	memset(d_entry[d_idx].dpath, 0, MAX_NAME);

	memcpy(d_entry[d_idx].dpath, path, strlen(path));
	dir_no++;

	return 0;
}

static int hello_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset,
		struct fuse_file_info* fi) {
	int idx = 0;
	int d_idx = 2;

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	
	while((dir_no+2) != d_idx) {
		filler(buf, d_entry[d_idx].dpath+1, NULL, 0);
		++d_idx;
	}

	while(file_no != idx) {
		filler(buf, files[idx]->hello_path+1, NULL, 0);
		++idx;
	}

	return 0;
}

static int hello_open(const char* path, struct fuse_file_info* fi) {
	int idx;
	int fd = 3;

	for(idx=0; idx<file_no; idx++) {
		if(strcmp(path, files[idx]->hello_path) == 0)
			break;
		++fd;
	}

	if(files[idx]->hello_path != ((const char *)0))
		fi->flags = O_CREAT|O_WRONLY|O_TRUNC;
	else
		fi->flags = O_RDWR;

	fi->fh = fd;

	return 0;
}

static int hello_release(const char* path, struct fuse_file_info* fi) {
	int idx = 0;
	
	while(files[idx]->hello_path != ((const char *)0)) {
		if(strcmp(path, files[idx]->hello_path) == 0)
			break;
		++idx;
	}

	fi->fh = -1;

	return 0;
}

static int hello_create(const char* path, mode_t mode, struct fuse_file_info* fi) {
	tfile* newfile;
	int idx;

	for(idx=0; idx<file_no; idx++) {
		if(strcmp(path, files[idx]->hello_path) == 0)
			return -EEXIST;
	}

	newfile = (tfile*)malloc(sizeof(struct tfile));
	newfile->hello_path = (char*)malloc(256);
	newfile->hello_str = (char*)malloc(256);
	
	strcpy(newfile->hello_path, path);
	newfile->hello_str = NULL;
	fi->flags = O_CREAT|O_WRONLY|O_TRUNC;
	mode = 0644;

	files[idx] = newfile;
	file_no = idx + 1;
printf("CCCCCCCCCCCCCCCCCCCCCCCCCCCCC str is = %s\n", files[idx]->hello_path);
	return 0;
}
/*
static int hello_unlink(const char* path) {
	int idx = 0;

	if(file_no > 0) {
		while(files[idx]->hello_path != ((const char *)0)) {
			if(strcmp(path, files[idx]->hello_path) == 0)
				break;
			++idx;
		}

		if(strcmp(path, files[idx]->hello_path) == 0) {
			free(files[idx]->hello_str);
			free(files[idx]->hello_path);
			free(files[idx]);

//			memset(files[idx]->hello_path, 0, sizeof(256));
//			memset(files[idx]->hello_str, 0, sizeof(256));


			file_no--;

			return 0;
		}
	}
	else
		return -ENOENT;
}
*/
static int hello_read(const char* path, char* buf, size_t size, off_t offset,
		struct fuse_file_info* fi) {
	int idx;
	off_t nread;

	for(idx=0; idx<file_no; idx++) {
		if(strcmp(path, files[idx]->hello_path) == 0)
			break;
	}

	if(files[idx]->hello_path == ((const char *)0))
		return -ENOENT;

	nread = strlen(files[idx]->hello_str);

	if(offset > nread)
		return 0;

	if(offset + (off_t)size > nread) {
		size = (size_t)(nread - offset);
	}
	memcpy(buf, files[idx]->hello_str + offset, size);

	return nread;
}

static int hello_write(const char* path, const char* buf, size_t size, off_t offset,
		struct fuse_file_info* fi) {
	int idx;
	off_t nread;

	for(idx=0; idx<file_no; idx++) {
		if(strcmp(path, files[idx]->hello_path) == 0)
			break;
	}

	if(files[idx]->hello_path == ((const char *)0))
		return -ENOENT;
	
	nread = strlen(buf);

	files[idx]->hello_str = (char*)malloc(nread);
	memset(files[idx]->hello_str, 0, sizeof(files[idx]->hello_str));

	memcpy(files[idx]->hello_str, buf, nread);

	return nread;
}

static int hello_utimens(const char* path, const struct timespec ts[2]) {
	struct utimbuf ubuf;

	ubuf.actime = ts[0].tv_sec;
	ubuf.modtime = ts[1].tv_sec;

	return 0;
}

static struct fuse_operations hello_oper = {
	.getattr = hello_getattr,
//	.setattr = hello_setattr,
#ifdef HAVE_SETXATTR
	.getxattr = hello_getxattr,
	.setxattr = hello_setxattr,
#endif
	.mkdir = hello_mkdir,
	.readdir = hello_readdir,
	.open = hello_open,
	.release = hello_release,
	.create = hello_create,
//	.unlink = hello_unlink,
	.read = hello_read,
	.write = hello_write,
	.utimens = hello_utimens,
};

int main(int argc, char** argv) {
	files = (tfile**)malloc(sizeof(struct tfile*));
	
	for(int i=0; i<MAX_FILE_NO; i++) {
		*files = (tfile*)malloc(sizeof(struct tfile));
	}
	
	d_entry[0].dpath = (char*)malloc(sizeof(char)*2);
	d_entry[1].dpath = (char*)malloc(sizeof(char)*3);

	memcpy(d_entry[0].dpath, "/", sizeof(char)*2);
	memcpy(d_entry[1].dpath, "..", sizeof(char)*3);

//	d_entry[0].dpath = (char*)malloc(sizeof(char)*3);
//	d_entry[1].dpath = (char*)malloc(sizeof(char)*2);
	
//	memcpy(d_entry[0].dpath, "..", sizeof(char)*3);
//	memcpy(d_entry[1].dpath, "/", sizeof(char)*2);

	return fuse_main(argc, argv, &hello_oper, &files);
}
