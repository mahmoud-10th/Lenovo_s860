#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>

struct exclude_list{
	char *dir;
};

/* return -1 on failure, with errno set to the first error */
static int unlink_recursive(const char* name)
{
    struct stat st;
    DIR *dir;
    struct dirent *de;
    int fail = 0;

    //printf("unlink %s\n", name);
    /* is it a file or directory? */
    if (lstat(name, &st) < 0)
        return -1;

    /* a file, so unlink it */
    if (!S_ISDIR(st.st_mode))
        return unlink(name);

    /* a directory, so open handle */
    dir = opendir(name);
    if (dir == NULL)
        return -1;

    /* recurse over components */
    errno = 0;
    while ((de = readdir(dir)) != NULL) {
        char dn[PATH_MAX];
        if (!strcmp(de->d_name, "..") || !strcmp(de->d_name, "."))
            continue;
        sprintf(dn, "%s/%s", name, de->d_name);
        if (unlink_recursive(dn) < 0) {
            fail = 1;
            break;
        }
        errno = 0;
    }
    /* in case readdir or unlink_recursive failed */
    if (fail || errno < 0) {
        int save = errno;
        closedir(dir);
        errno = save;
        return -1;
    }

    /* close directory handle */
    if (closedir(dir) < 0)
        return -1;

    /* delete target directory */
    return rmdir(name);
}

static int
is_exclude_dir(const char *abs_path, const struct exclude_list *dirs)
{
	int ret = 0;
	int i = 0;
	char *p;

	do {
		p = dirs[i++].dir;
		//printf("exclude dir:%s\n", p);
		if (!p)
			break;
		if (strcmp(abs_path, p) == 0) {
			ret = 1;
			break;
		}
	}while(1);

	return ret;
}

static int
is_exclude_dir_parent(const char *abs_path, const struct exclude_list *dirs)
{
	int ret = 0;
	int i = 0;
	char *p;

	do {
		p = dirs[i++].dir;
		//printf("exclude dir:%s\n", p);
		if (!p)
			break;
		if (strstr(p, abs_path) == p) {
			ret = 1;
			break;
		}
	}while(1);

	return ret;
}

// abs_path: path to rm and exclude specified dirs
// dirs: exclude dir list
int unlink_recursive_with_exclude(const char *abs_path, const struct exclude_list *dirs)
{
	int ret = 0;
	DIR *d;
	struct dirent *de;

	printf("abs_path=%s\n", abs_path);
	// if path is excluded, ignore it
	if (is_exclude_dir(abs_path, dirs)) {
		printf("keep %s\n", abs_path);
		return 0;
	}

	printf("abs_path is not exclude dir\n", abs_path);
	// if path is not any of exclude parent dirs
	if (!is_exclude_dir_parent(abs_path, dirs)) {
		printf("remove %s\n", abs_path);
		return unlink_recursive(abs_path);
	}

	printf("abs_path is exclude dir's parent\n", abs_path);
	// path is one of exclude dirs' parent, loop it recursively
	d = opendir(abs_path);
	if (d == NULL) {
		printf("error opening %s\n", abs_path);
		return ret;
	}

	while ((de = readdir(d)) != NULL) {
	        char abs_sub_path[PATH_MAX];
		printf("   %s\n", de->d_name);
		if (!strcmp(de->d_name, "..") || !strcmp(de->d_name, "."))
			continue;
		sprintf(abs_sub_path, "%s/%s", abs_path, de->d_name);
		printf("check dir:%s\n", abs_sub_path);
		ret = unlink_recursive_with_exclude(abs_sub_path, dirs);
	}
	closedir(d);
	printf("end\n");

	return ret;
}

int wipe_volume_with_exclude_list(const char *start_path)
{
	char *p;
	int i = 0;
	struct exclude_list dirs[] = {
		// has to keep this file, see
		// initialize_directories()/installd.c for detail
		{"/data/.layout_version"},
                /*lenovo-caijh, add for theme preload for custom, 2014.4.10 begin*/
                {"/data/system/themepreload.dat"},
                {"/data/system/default_lockscreen_wallpaper.png"},
                /*lenovo-caijh, add for theme preload for custom, 2014.4.10 end*/
		// keep user data
		{"/data/media"},
		NULL
	};

	do {
		p = dirs[i++].dir;
		if (!p)
			break;
		printf("exclude dir:%s\n", p);
	}while(1);
	return unlink_recursive_with_exclude(start_path, dirs);
}

