/*
  Copyright (C) JAEHYUK CHO
  All rights reserved.
  Code by JaeHyuk Cho <mailto:minzkn@minzkn.com>
*/

#if !defined(_ISOC99_SOURCE)
# define _ISOC99_SOURCE (1L)
#endif

#if !defined(_GNU_SOURCE)
# define _GNU_SOURCE (1L)
#endif

#if !defined(FUSE_USE_VERSION)
# define FUSE_USE_VERSION 26
#endif

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <fuse.h>

#define def_mzfuse_virtual_proc_pathname "/virtual_proc"

#if defined(def_mzfuse_virtual_proc_pathname)
# include <sys/stat.h>
# include <dirent.h>
# include <unistd.h>
# define def_mzfuse_real_proc_pathname "/proc"
#endif

#if !defined(mzfuse_virtual_filesystem_t)
typedef struct mzfuse_virtual_filesystem_ts {
    const char *m_pathname;
    const unsigned char *m_data;
}__mzfuse_virtual_filesystem_t;
# define mzfuse_virtual_filesystem_t __mzfuse_virtual_filesystem_t
#endif

int main(int s_argc, char **s_argv);

static int mzfuse_getattr(const char *s_pathname, struct stat *s_statbuffer);
static int mzfuse_open(const char *s_pathname, struct fuse_file_info *s_fileinfo);
static int mzfuse_read(const char *s_pathname, char *s_data, size_t s_size, off_t s_offset, struct fuse_file_info *s_fileinfo);
static int mzfuse_readdir(const char *s_pathname, void *s_buffer, fuse_fill_dir_t s_filler, off_t s_offset, struct fuse_file_info *s_fileinfo);

static mzfuse_virtual_filesystem_t g_mzfuse_virtual_filesystem[] = {
    {"/.", (const unsigned char *)0},
    {"/..", (const unsigned char *)0},
    {"/TESTFILE00", (const unsigned char *)"kkkk DATA 00"},
    {"/TESTFILE01", (const unsigned char *)"VIRTUAL DATA 01"},
    {"/TESTFILE02", (const unsigned char *)"VIRTUAL DATA 02"},
    {"/TESTFILE03", (const unsigned char *)"VIRTUAL DATA 03"},
    {(const char *)0, (const unsigned char *)0}
};

static struct fuse_operations g_mzfuse_operations = {
    /* int (*getattr) (const char *, struct stat *); */
    .getattr = mzfuse_getattr,
    /* int (*readlink) (const char *, char *, size_t); */
    /* int (*getdir) (const char *, fuse_dirh_t, fuse_dirfil_t); */
    /* int (*mknod) (const char *, mode_t, dev_t); */
    /* int (*mkdir) (const char *, mode_t); */
    /* int (*unlink) (const char *); */
    /* int (*rmdir) (const char *); */
    /* int (*symlink) (const char *, const char *); */
    /* int (*rename) (const char *, const char *); */
    /* int (*link) (const char *, const char *); */
    /* int (*chmod) (const char *, mode_t); */
    /* int (*chown) (const char *, uid_t, gid_t); */
    /* int (*truncate) (const char *, off_t); */
    /* int (*utime) (const char *, struct utimbuf *); */
    /* int (*open) (const char *, struct fuse_file_info *); */
    .open = mzfuse_open,
    /* int (*read) (const char *, char *, size_t, off_t, struct fuse_file_info *); */
    .read = mzfuse_read,
    /* int (*write) (const char *, const char *, size_t, off_t, struct fuse_file_info *); */
    /* int (*statfs) (const char *, struct statvfs *); */
    /* int (*flush) (const char *, struct fuse_file_info *); */
    /* int (*release) (const char *, struct fuse_file_info *); */
    /* int (*fsync) (const char *, int, struct fuse_file_info *); */
    /* int (*setxattr) (const char *, const char *, const char *, size_t, int); */
    /* int (*getxattr) (const char *, const char *, char *, size_t); */
    /* int (*listxattr) (const char *, char *, size_t); */
    /* int (*removexattr) (const char *, const char *); */
    /* int (*opendir) (const char *, struct fuse_file_info *); */
    /* int (*readdir) (const char *, void *, fuse_fill_dir_t, off_t, struct fuse_file_info *); */
    .readdir = mzfuse_readdir,
    /* int (*releasedir) (const char *, struct fuse_file_info *); */
    /* int (*fsyncdir) (const char *, int, struct fuse_file_info *); */
    /* void *(*init) (struct fuse_conn_info *conn); */
    /* void (*destroy) (void *); */
    /* int (*access) (const char *, int); */
    /* int (*create) (const char *, mode_t, struct fuse_file_info *); */
    /* int (*ftruncate) (const char *, off_t, struct fuse_file_info *); */
    /* int (*fgetattr) (const char *, struct stat *, struct fuse_file_info *); */
    /* int (*lock) (const char *, struct fuse_file_info *, int cmd, struct flock *); */
    /* int (*utimens) (const char *, const struct timespec tv[2]); */
    /* int (*bmap) (const char *, size_t blocksize, uint64_t *idx); */
    /* unsigned int flag_nullpath_ok : 1; */
    /* unsigned int flag_reserved : 31; */
    /* int (*ioctl) (const char *, int cmd, void *arg, struct fuse_file_info *, unsigned int flags, void *data); */
    /* int (*poll) (const char *, struct fuse_file_info *, struct fuse_pollhandle *ph, unsigned *reventsp); */
};

static int mzfuse_getattr(const char *s_pathname, struct stat *s_statbuffer)
{
    int s_index = 0;
#if defined(def_mzfuse_virtual_proc_pathname)	
    size_t s_virtual_proc_prefix_size = strlen(def_mzfuse_virtual_proc_pathname);
    char s_real_temp_pathname[256];
#endif    

    (void)memset((void *)s_statbuffer, 0, sizeof(struct stat));

    if(strcmp(s_pathname, "/") == 0) {
        s_statbuffer->st_mode = S_IFDIR | 0755;
        s_statbuffer->st_nlink = 2;

        return(0);
    }

#if defined(def_mzfuse_virtual_proc_pathname)    
    if(strncmp(s_pathname, def_mzfuse_virtual_proc_pathname, s_virtual_proc_prefix_size) == 0) {
        (void)snprintf(
	    (char *)(&s_real_temp_pathname[0]),
	    sizeof(s_real_temp_pathname),
	    "%s%s",
	    def_mzfuse_real_proc_pathname,
	    (char *)(&s_pathname[s_virtual_proc_prefix_size])
	);

	return(stat(s_real_temp_pathname, s_statbuffer));
    }
#endif    

    while(g_mzfuse_virtual_filesystem[s_index].m_pathname != ((const char *)0)) {
        if(strcmp(s_pathname, g_mzfuse_virtual_filesystem[s_index].m_pathname) == 0) {
            break;
        }
        ++s_index;
    }
    
    if(g_mzfuse_virtual_filesystem[s_index].m_pathname == ((const char *)0)) {
        return(-ENOENT);
    }
        
    s_statbuffer->st_mode = S_IFREG | 0444;
    s_statbuffer->st_nlink = 1;
    s_statbuffer->st_size = strlen("TEST DATA");

    return(0);
}

static int mzfuse_open(const char *s_pathname, struct fuse_file_info *s_fileinfo)
{
    int s_index = 0;
    
    while(g_mzfuse_virtual_filesystem[s_index].m_pathname != ((const char *)0)) {
        if(strcmp(s_pathname, g_mzfuse_virtual_filesystem[s_index].m_pathname) == 0) {
            break;
        }
        ++s_index;
    }
    
    if(g_mzfuse_virtual_filesystem[s_index].m_pathname == ((const char *)0)) {
        return(-ENOENT);
    }
    
    if((s_fileinfo->flags & O_RDONLY) != O_RDONLY) {
        return(-EACCES);
    }

    return(0);
}

static int mzfuse_read(const char *s_pathname, char *s_data, size_t s_size, off_t s_offset, struct fuse_file_info *s_fileinfo)
{
    off_t s_result;
    int s_index = 0;
    
    while(g_mzfuse_virtual_filesystem[s_index].m_pathname != ((const char *)0)) {
        if(strcmp(s_pathname, g_mzfuse_virtual_filesystem[s_index].m_pathname) == 0) {
            break;
        }
        ++s_index;
    }
    
    if(g_mzfuse_virtual_filesystem[s_index].m_pathname == ((const char *)0)) {
        return(-ENOENT);
    }

    s_result = strlen((const char *)g_mzfuse_virtual_filesystem[s_index].m_data);
    if(s_offset >= s_result) {
        return(0);
    }

    if(s_result < (s_offset + ((off_t)s_size))) {
        s_size = (size_t)(s_result - s_offset);
    }

    (void)memcpy((void *)s_data, (const void *)(&g_mzfuse_virtual_filesystem[s_index].m_data[s_offset]), s_size);

    return(s_result);
}

static int mzfuse_readdir(const char *s_pathname, void *s_buffer, fuse_fill_dir_t s_filler, off_t s_offset, struct fuse_file_info *s_fileinfo)
{
    int s_index = 0;
#if defined(def_mzfuse_virtual_proc_pathname)	
    size_t s_virtual_proc_prefix_size = strlen(def_mzfuse_virtual_proc_pathname);
	
    DIR *s_dir;
    struct dirent *s_dirent;
    struct stat s_stat;

    char s_real_temp_pathname[256];
#endif

    if(strcmp(s_pathname, "/") == 0) {
        while(g_mzfuse_virtual_filesystem[s_index].m_pathname != ((const char *)0)) {
            /* typedef int (*fuse_fill_dir_t) (void *buf, const char *name, const struct stat *stbuf, off_t off); */
            s_filler(s_buffer, (const char *)(&g_mzfuse_virtual_filesystem[s_index].m_pathname[1]), (const struct stat *)0, (off_t)0);
    
            ++s_index;
        }

#if defined(def_mzfuse_virtual_proc_pathname)	
	if(stat(def_mzfuse_real_proc_pathname, (struct stat *)(&s_stat)) == 0) {
            s_filler(s_buffer, &def_mzfuse_virtual_proc_pathname[1], (const struct stat *)(&s_stat), (off_t)0);
	}
#endif	

        return(0);
    }
    
#if defined(def_mzfuse_virtual_proc_pathname)    
    if(strncmp(s_pathname, def_mzfuse_virtual_proc_pathname, s_virtual_proc_prefix_size) == 0) {
        (void)snprintf(
	    (char *)(&s_real_temp_pathname[0]),
	    sizeof(s_real_temp_pathname),
	    "%s%s",
	    def_mzfuse_real_proc_pathname,
	    (char *)(&s_pathname[s_virtual_proc_prefix_size])
	);

	s_dir = opendir((const char *)(&s_real_temp_pathname[0]));
	if(s_dir == ((DIR *)0)) {
            return(-ENOENT);
	}

	for(;;) {
	    s_dirent = readdir(s_dir);
	    if(s_dirent == ((struct dirent *)0)) {
	        break;
	    }
	    
	    (void)snprintf(
	        (char *)(&s_real_temp_pathname[0]),
		sizeof(s_real_temp_pathname),
		"%s/%s",
		def_mzfuse_real_proc_pathname,
		(char *)(&s_dirent->d_name[0])
	    );
	    
	    if(stat(s_real_temp_pathname, (struct stat *)(&s_stat)) == (-1)) {
	        continue;
	    }

	    /* typedef int (*fuse_fill_dir_t) (void *buf, const char *name, const struct stat *stbuf, off_t off); */
            s_filler(s_buffer, (const char *)(&s_dirent->d_name[0]), (const struct stat *)(&s_stat), (off_t)0);
	}

	(void)closedir(s_dir);

        return(0);
    }
#endif    

    return(-ENOENT);
}

int main(int s_argc, char **s_argv)
{
    /* int fuse_main(int argc, char *argv[], const struct fuse_operations *op, void *user_data); */
    return(fuse_main(s_argc, s_argv, (const struct fuse_operations *)(&g_mzfuse_operations), (void *)0));
}

/* End of source */
