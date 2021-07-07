/*
 * bnconfig.h.cmake -- Configuration file for BigNum library.
 *
 * cmake processes this file.
 */
#ifndef _BNCONFIG_H
#define _BNCONFIG_H

/* Checks for the presence and absence of various header files */
#cmakedefine HAVE_ASSERT_H 1
#define NO_ASSERT_H !HAVE_ASSERT_H

#cmakedefine HAVE_LIMITS_H 1
#define NO_LIMITS_H !HAVE_LIMITS_H

#cmakedefine HAVE_STDLIB_H 1
#define NO_STDLIB_H !HAVE_STDLIB_H

#cmakedefine HAVE_STRING_H 1
#define NO_STRING_H !HAVE_STRING_H

#cmakedefine HAVE_STRINGS_H 1

#cmakedefine NEED_MEMORY_H 1

/* We go to some trouble to find accurate times... */

/* Define if you have Posix.4 glock_gettime() */
#cmakedefine HAVE_CLOCK_GETTIME 1
/* Define if you have Solaris-style gethrvtime() */
#cmakedefine HAVE_GETHRVTIME 1
/* Define if you have getrusage() */
#cmakedefine HAVE_GETRUSAGE 1
/* Define if you have clock() */
#cmakedefine HAVE_CLOCK 1
/* Define if you have time() */
#cmakedefine HAVE_TIME 1

/*
 * Define as 0 if #including <sys/time.h> automatically
 * #includes <time.h>, and doing so explicitly causes an
 * error.
 */
#define TIME_WITH_SYS_TIME 0

/* Defines for various kinds of library brokenness */

/* Define if <stdio.h> is missing prototypes (= lots of warnings!) */
#cmakedefine NO_STDIO_PROTOS 1

/* Define if <assert.h> depends on <stdio.h> and breaks without it */
#cmakedefine ASSERT_NEEDS_STDIO 1
/* Define if <assert.h> depends on <stdlib.h> and complains without it */
#cmakedefine ASSERT_NEEDS_STDLIB 1

/*
 * Define if <string.h> delcares the mem* functions to take char *
 * instead of void * parameters (= lots of warnings)
 */
#cmakedefine MEM_PROTOS_BROKEN 1

/* If not available, bcopy() is substituted */
#cmakedefine HAVE_MEMMOVE 1
#define NO_MEMMOVE !HAVE_MEMMOVE
#cmakedefine HAVE_MEMCPY 1
#define NO_MEMCPY !HAVE_MEMCPY

#endif /* _BNCONFIG_H */
