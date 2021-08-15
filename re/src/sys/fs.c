/**
 * @file fs.c  File-system functions
 *
 * Copyright (C) 2010 Creytiv.com
 */
#define _DEFAULT_SOURCE 1
#define _BSD_SOURCE 1
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#if defined(WIN32) || defined(__WIN32__)
#include <windows.h>
#include <shlobj.h>
#include <direct.h>
#include <lmaccess.h>
#endif
#include <re_types.h>
#include <re_fmt.h>
#include <re_sys.h>


/**
 * Create a directory with full path
 *
 * @param path Directory path
 * @param mode Access permissions
 *
 * @return 0 if success, otherwise errorcode
 */
int fs_mkdir(const char *path, uint16_t mode)
{
	int ret;

	if (!path)
		return EINVAL;

#if defined(WIN32) || defined(__WIN32__)
	(void)mode;
	ret = _mkdir(path);
#else
	ret = mkdir(path, mode);
#endif
	if (ret < 0)
		return errno;

	return 0;
}


/**
 * Get the home directory for the current user
 *
 * @param path String to write home directory
 * @param sz   Size of path string
 *
 * @return 0 if success, otherwise errorcode
 */
int fs_gethome(char *path, size_t sz)
{
#if defined(WIN32) || defined(__WIN32__)
	char win32_path[MAX_PATH];

	if (!path || !sz)
		return EINVAL;

	if (S_OK != SHGetFolderPath(NULL,
				    CSIDL_APPDATA | CSIDL_FLAG_CREATE,
				    NULL,
				    0,
				    win32_path)) {
		return ENOENT;
	}

	str_ncpy(path, win32_path, sz);

	return 0;

#elif defined(HAVE_PWD_H)
	const char *loginname;
	struct passwd *pw;

	if (!path || !sz)
		return EINVAL;

	loginname = sys_username();
	if (!loginname)
		return ENOENT;

	pw = getpwnam(loginname);
	if (!pw)
		return errno;

	str_ncpy(path, pw->pw_dir, sz);

	return 0;
#else
	(void)path;
	(void)sz;
	return ENOSYS;
#endif
}


/**
 * Check if given path is directory
 *
 * @param path Directory
 *
 * @return True if directory, False if not
 */
bool fs_isdir(const char *path)
{
	struct stat st;

	if (!path)
		return false;

	if (stat(path, &st) < 0)
		return false;

	if ((st.st_mode & S_IFMT) != S_IFDIR)
		return false;

	return true;
}


/**
 * Check if given file exists and is a regular file
 *
 * @param file Filepath
 *
 * @return True if exists and is regular file, False if not
 */
bool fs_isfile(const char *file)
{
	struct stat st;

	if (!file)
		return false;

	if (stat(file, &st) < 0)
		return false;

	if ((st.st_mode & S_IFMT) != S_IFREG)
		return false;

	return true;
}
