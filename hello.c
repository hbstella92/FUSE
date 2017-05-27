#define FUSE_USE_VERSION 26

#include <sys/statvfs.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/xattr.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

#include "fuse.h"
//#include "fuse_lowlevel.h"

typedef struct ds {
	const char* hello_path;
	unsigned char* hello_str;
}ds_t;

static ds_t aaa[] = {
	{"/.", (unsigned char*)0},
	{"/..", (unsigned char*)0},
	{"/hello", (unsigned char*)"Hello World!\n"},
	{"/file_one", (unsigned char*)"1234567890123456789\n"},
	{(const char*)0, (unsigned char*)0}
};

static int fd;
static FILE* fp;

static int hello_chmod(const char* path, mode_t mode) {
	int idx = 0;
	struct stat* buf;
	int ret = 0;

	while(aaa[idx].hello_path != ((const char *)0)) {
		if(strcmp(path, aaa[idx].hello_path) == 0) {
			break;
		}
		++idx;
	}

	if(aaa[idx].hello_path == ((const char *)0))
		return -ENOENT;

	//find error code
	if((ret = stat(aaa[idx].hello_path, buf)) < 0)
		return -1;

	buf->st_mode = mode;

	return ret;
}

static int hello_open(const char* path, struct fuse_file_info* fi) {
	int idx = 0;

	while(aaa[idx].hello_path != ((const char *)0)) {
		if(strcmp(path, aaa[idx].hello_path) == 0) {
			break;
		}
		++idx;
	}

	if(aaa[idx].hello_path == ((const char *)0))
		return -ENOENT;

//	fd = fi->fh;
	fp = fopen(aaa[idx].hello_path, "r+");

	return 0;
}

static int hello_read(const char* path, char* buf, size_t size, off_t offset,
		struct fuse_file_info* fi) {
	int idx = 0;
	off_t result;

	while(aaa[idx].hello_path != ((const char *)0)) {
		if(strcmp(path, aaa[idx].hello_path) == 0) {
			break;
		}
		++idx;
	}

	if(aaa[idx].hello_path == ((const char *)0))
		return -ENOENT;

	result = strlen(aaa[idx].hello_str);

	if(offset >= result)
		return 0;

	if(result < offset + (off_t)size) {
		size = (size_t)(result - offset);
	}
	memcpy(buf, &aaa[idx].hello_str[offset], size);

	return result;
}

static int hello_write(const char* path, const char* buf, size_t size, off_t offset,
		struct fuse_file_info* fi) {
	int idx = 0;
	int nread = 0;

	if(strcmp(path, "/") != 0) {
		while(aaa[idx].hello_path != ((const char*)0)) {
			if(strcmp(path, aaa[idx].hello_path) ==0) {
				break;
			}
			++idx;
		}

		if(aaa[idx].hello_path == ((const char*)0))
			return -ENOENT;

		nread = strlen(buf);

		aaa[idx].hello_str = (char*)malloc(nread);
		memset(aaa[idx].hello_str, 0, sizeof(aaa[idx].hello_str));

		memcpy(aaa[idx].hello_str, buf, nread);
/*		if(offset >= nread)
			return 0;

		if(nread < offset + (off_t)size) {
			size = (size_t)nread - offset;
		}
		
		free(aaa[idx].hello_str);
*/	}

	return nread;
}

static int hello_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset,
		struct fuse_file_info* fi) {
	int idx = 0;
	size_t size = strlen("/dir");
	DIR* dir;
	struct dirent* d_entry;
	struct stat status;
	char tmp_path[256];

	if(strcmp(path, "/") == 0) {
		while(aaa[idx].hello_path != ((const char *)0)) {
			filler(buf, &aaa[idx].hello_path[1], NULL, 0);

			++idx;
		}

		if(stat("/dir", (struct stat*)&status) == 0) {
			filler(buf, "dir", (const struct stat*)&status, 0);
		}
		return 0;
	}

	if(strncmp(path, "/dir", size) == 0) {
		snprintf((char*)(&tmp_path[0]), sizeof(tmp_path), "%s%s", "/dir", (char*)(&path[size]));

		dir = opendir((const char*)(&tmp_path[0]));

		if(dir == ((DIR*)0))
			return -ENOENT;

		for(;;) {
			d_entry = readdir(dir);

			if(d_entry == (struct dirent*)0)
				break;

			snprintf((char*)(&tmp_path[0]), sizeof(tmp_path), "%s/%s",
					"/dir", (char*)(&d_entry->d_name[0]));

			if(stat(tmp_path, (struct stat*)(&status)) == -1)
				continue;

			filler(buf, (const char *)(&d_entry->d_name[0]), (const struct stat*)(&status),
					(off_t)0);
		}

		closedir(dir);

		return 0;
	}
	return -ENOENT;
}

static int hello_getattr(const char* path, struct stat* stbuf) {

	int idx = 0;
	size_t size = strlen("/dir");
	char tmp_path[256];

	memset(stbuf, 0, sizeof(struct stat));

	if(strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR|0755;
		stbuf->st_nlink = 2;

		return 0;
	}

	if(strncmp(path, "/dir", size) == 0) {
		snprintf((char*)(&tmp_path[0]), sizeof(tmp_path), "%s%s", "/dir",
				(char*)(&path[size]));
		return stat(tmp_path, stbuf);
	}

	while(aaa[idx].hello_path != ((const char*)0)) {
		if(strcmp(path, aaa[idx].hello_path) == 0)
			break;
		++idx;
	}

	if(aaa[idx].hello_path == ((const char*)0))
		return -ENOENT;

	stbuf->st_mode = S_IFREG|0666;
	stbuf->st_nlink = 1;
	stbuf->st_uid = getuid();
	stbuf->st_gid = getgid();
	stbuf->st_size = strlen(aaa[idx].hello_str);

	return 0;
}
/*
static int hello_release(const char* path, struct fuse_file_info* fi) {
	int idx = 0;
	struct stat* stbuf;
	FILE* fp;

	while(aaa[idx].hello_path != (const char*)0) {
		if(strcmp(path, aaa[idx].hello_path) == 0)
			break;
		++idx;
	}

	if(aaa[idx].hello_path == ((const char *)0))
		return -ENOENT;

	if(strcmp(path, rm_file) != 0)
		return -ENOENT;

	fp = fopen(path, "r");

	stat(path, stbuf);
	if(stbuf->st_nlink != 1) {
		printf("there are references!\n");
		return -1;
	}
	if(fp != NULL) {
		fclose(fp);
		fp = NULL;
	}
	printf("close rm_file!\n");


	return 0;
}
*/
/*
static int hello_lib_getxattr(const char* path, const char* name, char* value, size_t size) {
	int idx = 0;
	int ret = 0;

	while(aaa[idx].hello_path != (const char*)0) {
		if(strcmp(path, aaa[idx].hello_path) == 0)
			break;
		++idx;
	}

	if(aaa[idx].hello_path == ((const char *)0))
		return -ENOENT;

	ret = lgetxattr(aaa[idx].hello_path, name, value, size);

	// find errno
	if(ret == -1)
		return -1;

	return ret;
}
*/
static struct fuse_operations hello_oper = {
	.getattr = hello_getattr,
//	.mkdir = hello_mkdir,
//	.rmdir = hello_rmdir,
//	.chmod = hello_chmod,
	.open = hello_open,
	.read = hello_read,
	.write = hello_write,
//	.release = hello_release,
	.readdir = hello_readdir,
//	.create = hello_create,
};
/*
static struct fuse_lowlevel_ops fuse_path_ops = {
#ifdef HAVE_SETXATTR
	.getxattr = hello_lib_getxattr,
#endif
};
*/
int main(int argc, char** argv) {
	return fuse_main(argc, argv, &hello_oper, NULL);
}
