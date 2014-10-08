CFLAGS=-Wall -g -DDEBUG -DOSX -DUART_TESTS

all:uuart.app

OBJS=Main.o\
	Debug.o\
	Common.o\
	Portable.o\
	OsxUart.o\
    Uart.o

uuart.app: $(OBJS)
	cc $(OBJS) -o uuart.app

Main.o: Main.h Main.c
	cc $(CFLAGS) -c Main.c

Uart.o: Uart.h Uart.c
	cc $(CFLAGS) -c Uart.c

OsxUart.o: OsxUart.h OsxUart.c
	cc $(CFLAGS) -c OsxUart.c

Portable.o: Portable.h Portable.c
	cc $(CFLAGS) -c Portable.c

Debug.o: Debug.h Debug.c
	cc $(CFLAGS) -c Debug.c

Common.o: Common.h Common.c
	cc $(CFLAGS) -c Common.c

clean: 
	rm *.o *.exe *.app

