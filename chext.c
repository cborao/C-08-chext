
// JULIO 2020. CÉSAR BORAO MORATINOS: chext.c

#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

enum {
	File = 0,
	Directory = 1,
};

int
haveperms(struct stat *buffer) {

	if ((buffer->st_mode & S_IRUSR) == 00400)
		return 1;

	if ((buffer->st_mode & S_IRGRP) == 00040)
		return 1;

	if ((buffer->st_mode & S_IROTH) == 00004)
		return 1;

	return 0;
}

int
filetype(struct stat *buffer) {

	if ((buffer->st_mode & S_IFMT) == S_IFREG && haveperms(buffer))
		return File;

	if ((buffer->st_mode & S_IFMT) == S_IFDIR)
		return Directory;

	return -1;
}

char *
newpath(char *filepath, char *ext, char *newext) {

	int size = strlen(filepath)-strlen(ext)+strlen(newext)+1;
	char newpath[size];
	strncpy(newpath,filepath,strlen(filepath)-strlen(ext));
	newpath[strlen(filepath)-strlen(ext)] = '\0';
	return strcat(newpath,newext);
}

int
checkfile(char *filepath, char *oldext, char *newext) {

	char *ext;
	if ((ext = strrchr(filepath,'.')) == NULL)
		return -1;

	ext++;

	if (strcmp(ext,oldext) == 0) {
		char *newname = newpath(filepath,ext,newext);
		if (rename(filepath,newname) < 0)
			errx(EXIT_FAILURE, "error using rename");
	}
	return 1;
}

char *
filepath(char *dirpath, char *name) {

	char *filepath;
	if ((filepath = malloc(strlen(dirpath)+sizeof('\0')+strlen(name)+1)) < 0)
		errx(EXIT_FAILURE,"malloc failed!");

	strncpy(filepath,dirpath,strlen(dirpath)+1);
	strcat(filepath,"/");
	strcat(filepath,name);
	return filepath;
}

void
checkdir(char *oldext, char *newext, char *dirpath) {

	struct dirent *entry;
	struct stat buffer;
	DIR *dir;

	if ((dir = opendir(dirpath)) == NULL)
		errx(EXIT_FAILURE, "error opening directory: %s", dirpath);

	while ((entry = readdir(dir)) != NULL) {

		char *filename = filepath(dirpath,entry->d_name);
		if (stat(filename, &buffer) < 0)
			errx(EXIT_FAILURE, "error reading file %s metadata", entry->d_name);

		switch (filetype(&buffer)) {
			case File:
				checkfile(filename,oldext,newext);
				break;
			case Directory:
				if ((strcmp(entry->d_name,".") != 0) && (strcmp(entry->d_name,"..") != 0))
					checkdir(oldext,newext,filename);
		}
		free(filename);
	}
	if (closedir(dir) < 0)
		errx(EXIT_FAILURE,"cannot close %s directory", dirpath);
}

int
main(int argc, char *argv[]) {

	--argc;
	++argv;

	if (argc != 3)
		errx(EXIT_FAILURE, "usage: [old extension] [new extension] [directory]");

	checkdir(argv[0],argv[1],argv[2]);
	exit(EXIT_SUCCESS);
}
