# $Id: Makefile,v 1.1 2012/08/07 11:33:45 sobomax Exp $

LIB=	g722
PREFIX?= /usr/local
LIBDIR= ${PREFIX}/lib
NO_SHARED=	YES
NO_PROFILE=	YES
INCLUDEDIR= ${PREFIX}/include
MAN=
SRCS=	g722_decode.c g722_encode.c
INCS=	g722.h g722_private.h g722_encoder.h g722_decoder.h
WARNS?=	2
CFLAGS+= -I${.CURDIR} ${PICFLAG}

CLEANFILES+=test test.raw

test: test.c lib${LIB}.a
	rm -f ${.CURDIR}/test.raw
	${CC} ${CFLAGS} -o ${.CURDIR}/test test.c lib${LIB}.a -lm
	${.CURDIR}/test
	sha256 test.raw | diff ${.CURDIR}/test.checksum -

.include <bsd.lib.mk>
