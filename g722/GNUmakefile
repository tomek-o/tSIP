VPATH = .

PREFIX?= /usr/local
LIBDIR= ${PREFIX}/lib
INCLUDEDIR= ${PREFIX}/include

SRCS_C= g722_decode.c g722_encode.c
SRCS_H= g722.h g722_private.h g722_encoder.h g722_decoder.h

CFLAGS?= -O2 -pipe

OBJS = $(SRCS_C:.c=.o)

all: libg722.a

libg722.a: $(OBJS) $(SRCS_H)
	$(AR) cq $@ $(OBJS)
	ranlib $@ 

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f libg722.a $(OBJS) test test.raw

test: test.c libg722.a
	${CC} ${CFLAGS} -o test test.c libg722.a -lm

install:
	install -d ${DESTDIR}${LIBDIR}
	install libg722.a ${DESTDIR}${LIBDIR}
	install -d ${DESTDIR}${INCLUDEDIR}
	install ${SRCS_H} ${DESTDIR}${INCLUDEDIR}
