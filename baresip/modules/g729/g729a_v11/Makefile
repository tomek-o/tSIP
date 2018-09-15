ARCH=$(PROC)
CFLAGS+=-Wall -O3 -funroll-loops -ggdb
ifneq (${OSARCH},CYGWIN)
CFLAGS += -fPIC
endif
LIB=libg729.a

OBJS=\
acelp_ca.o \
basic_op.o \
cod_ld8a.o \
cor_func.o \
de_acelp.o \
dec_gain.o \
dec_lag3.o \
dec_ld8a.o \
dspfunc.o \
filter.o \
gainpred.o \
lpc.o \
lpcfunc.o \
lspdec.o \
lspgetq.o \
oper_32b.o \
pitch_a.o \
postfilt.o \
post_pro.o \
p_parity.o \
pred_lt3.o \
pre_proc.o \
qua_gain.o \
qua_lsp.o \
tab_ld8a.o \
taming.o \
util.o

all: $(LIB)


$(LIB): $(OBJS)
	ar cr $(LIB) $(OBJS)
	ranlib $(LIB)

clean:
	rm -f $(LIB) *.o

install:

