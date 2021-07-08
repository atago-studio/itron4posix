CC=gcc
CFLAGS = -g -O -c -fPIC -I../include
AR=ar
ARFLAGS = crsv

objs = common.o exsync.o mempool.o sync.o task.o time.o

.PHONY : all

all : libitron.a

libitron.a : $(objs)
	$(AR) $(ARFLAGS) $@ $(objs)

clean :
	rm -f *.o *.a
