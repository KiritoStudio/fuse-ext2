/**
 * Copyright (c) 2008-2009 Alper Akcan <alper.akcan@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in the main directory of the fuse-ext2
 * distribution in the file COPYING); if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#include <errno.h>

typedef enum {
	file,
	directory,
} f_type;

typedef struct f_s {
	int type;
	char *name;
} f_t;

static f_t *files[] = {
	&(f_t) {directory, "/Library/Filesystems/fuse-ext2.fs"},
	&(f_t) {directory, "/System/Library/Filesystems/fuse-ext2.fs"},
	&(f_t) {directory, "/Library/PreferencePanes/fuse-ext2.prefPane"},
	&(f_t) {directory, "/System/Library/PreferencePanes/fuse-ext2.prefPane"},
	&(f_t) {file, "/usr/local/bin/fuse-ext2"},
	&(f_t) {file, "/usr/local/bin/fuse-ext2.wait"},
	&(f_t) {file, "/usr/local/bin/fuse-ext2.probe"},
	&(f_t) {file, "/usr/local/bin/fuse-ext2.mke2fs"},
	&(f_t) {file, "/usr/local/bin/fuse-ext2.e2label"},
	&(f_t) {file, "/usr/local/lib/pkgconfig/fuse-ext2.pc"},
	NULL,
};

static int rm_file (const char *path)
{
	if (access(path, W_OK | W_OK | F_OK) != 0) {
		printf("access failed for '%s' (%s)\n", path, strerror(errno));
		if (errno == ENOENT) {
			return 0;
		}
		return -1;
	}
	if (unlink(path) != 0) {
		printf("unlink failed for '%s' (%s)\n", path, strerror(errno));
	}
	return 0;
}

static int rm_directory (const char *path)
{
	DIR *dp;
	char *p;
	struct stat stbuf;
	struct dirent *current;
	dp = opendir(path);
	if (dp == NULL) {
		printf("opendir() failed for '%s' (%s)\n", path, strerror(errno));
		if (errno == ENOENT) {
			return 0;
		}
		return -1;
	}
	while ((current = readdir(dp)) != NULL) {
		if (strcmp(current->d_name, ".") == 0 ||
		    strcmp(current->d_name, "..") == 0) {
			continue;
		}
		p = (char *) malloc(sizeof(char) * (strlen(path) + 1 + strlen(current->d_name) + 1));
		if (p == NULL) {
			printf("malloc failed for '%s/%s'\n", path, current->d_name);
			continue;
		}
		sprintf(p, "%s/%s", path, current->d_name);
		if (lstat(p, &stbuf) != 0) {
			printf("stat failed for '%s' (%s)\n", p, strerror(errno));
			free(p);
			continue;
		}
		if (S_ISDIR(stbuf.st_mode)) {
			if (rm_directory(p) != 0) {
				printf("rm_directory() failed for '%s'\n", p);
			}
		} else {
			if (rm_file(p) != 0) {
				printf("rm_file() failed for '%s'\n", p);
			}
		}
		free(p);
	}
	if (rmdir(path) != 0) {
		printf("rmdir() failed for '%s'\n", path);
	}
	closedir(dp);
	return 0;
}

int main (int argc, char *argv[])
{
	f_t **f;
	for (f = files; *f != NULL; f++) {
		if ((*f)->type == directory) {
			rm_directory((*f)->name);
		} else if ((*f)->type == file) {
			rm_file((*f)->name);
		} else {
			printf("unkown file type '%d' for '%s'\n", (*f)->type, (*f)->name);
			exit(1);
		}
	}
	return 0;
}
