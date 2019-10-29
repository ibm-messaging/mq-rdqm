MQDIR=
INCDIR=$(MQDIR)/inc
LIBDIR=$(MQDIR)/lib64

all : rdqmget rdqmput

complete.o : ../complete.c
	cc -I $(INCDIR) -c ../complete.c

connection.o : ../connection.c
	cc -I $(INCDIR) -c ../connection.c

globals.o : ../globals.c
	cc -I $(INCDIR) -c ../globals.c

log.o : ../log.c
	cc -I $(INCDIR) -c ../log.c

options.o : ../options.c
	cc -I $(INCDIR) -c ../options.c

rdqmget : ../rdqmget.c complete.o connection.o globals.o log.o options.o
	cc -I $(INCDIR) -L $(LIBDIR) -l mqic_r -o rdqmget ../rdqmget.c complete.o connection.o globals.o log.o options.o

rdqmput : ../rdqmput.c complete.o connection.o globals.o log.o options.o
	cc -I $(INCDIR) -L $(LIBDIR) -l mqic_r -o rdqmput ../rdqmput.c complete.o connection.o globals.o log.o options.o

clean:
	rm -f *.o rdqmget rdqmput
