/**
 * @file re_sys.h  Interface to system module
 *
 * Copyright (C) 2010 Creytiv.com
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef VERSION
#define VERSION "?"
#endif

/**
 * @def ARCH
 *
 * Architecture
 */
#ifndef ARCH
#define ARCH "x86"
#endif

/**
 * @def OS
 *
 * Operating System
 */
#ifndef OS
#if defined(WIN32) || defined(__WIN32__)
#define OS "win32"
#else
#define OS "?"
#endif
#endif

struct re_printf;
int  sys_rel_get(uint32_t *rel, uint32_t *maj, uint32_t *min,
		 uint32_t *patch);
int  sys_kernel_get(struct re_printf *pf, void *unused);
int  sys_build_get(struct re_printf *pf, void *unused);
const char *sys_arch_get(void);
const char *sys_os_get(void);
const char *sys_libre_version_get(void);
int sys_coredump_set(bool enable);
int sys_daemon(void);
void sys_usleep(unsigned int us);

static inline void sys_msleep(unsigned int ms)
{
	sys_usleep(ms * 1000);
}


uint16_t sys_htols(uint16_t v);
uint32_t sys_htoll(uint32_t v);
uint16_t sys_ltohs(uint16_t v);
uint32_t sys_ltohl(uint32_t v);
uint64_t sys_htonll(uint64_t v);
uint64_t sys_ntohll(uint64_t v);


/* Random */
void     rand_init(void);
uint16_t rand_u16(void);
uint32_t rand_u32(void);
uint64_t rand_u64(void);
char     rand_char(void);
void     rand_str(char *str, size_t size);
void     rand_bytes(uint8_t *p, size_t size);

/* Time */

/** \brief Get time string with miliseconds */
char* sys_time(char* buf, int size);

/* File-System */
int  fs_mkdir(const char *path, uint16_t mode);
int  fs_gethome(char *path, size_t sz);
bool fs_isdir(const char *path);
bool fs_isfile(const char *file);

#ifdef __cplusplus
}
#endif

