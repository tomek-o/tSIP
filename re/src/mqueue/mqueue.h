/**
 * @file mqueue.h Thread Safe Message Queue -- Internal API
 *
 * Copyright (C) 2010 Creytiv.com
 */


#if defined(WIN32) || defined(__WIN32__)
int pipe(re_sock_t fds[2]);
ssize_t pipe_read(re_sock_t s, void *buf, size_t len);
ssize_t pipe_write(re_sock_t s, const void *buf, size_t len);
#else
static inline ssize_t pipe_read(re_sock_t s, void *buf, size_t len)
{
	return read(s, buf, len);
}


static inline ssize_t pipe_write(re_sock_t s, const void *buf, size_t len)
{
	return write(s, buf, len);
}
#endif
