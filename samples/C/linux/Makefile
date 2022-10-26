MQDIR=/opt/mqm
INCDIR=$(MQDIR)/inc
LIBDIR=$(MQDIR)/lib64
CC=gcc
CCFLAGS=-m64 -I $(INCDIR)
LDFLAGS=-L $(LIBDIR) -l mqic_r -Wl,-rpath=$(LIBDIR) -Wl,-rpath=/usr/lib64

prefix=/usr/local

.PHONY : all
all : rdqmget rdqmput

complete.o : ../complete.c
	$(CC) $(CCFLAGS) -c ../complete.c

connection.o : ../connection.c
	$(CC) $(CCFLAGS) -c ../connection.c

globals.o : ../globals.c
	$(CC) $(CCFLAGS) -c ../globals.c

log.o : ../log.c
	$(CC) $(CCFLAGS) -c ../log.c

options.o : ../options.c
	$(CC) $(CCFLAGS) -c ../options.c

rdqmget : ../rdqmget.c complete.o connection.o globals.o log.o options.o
	$(CC) $(CCFLAGS) -o rdqmget ../rdqmget.c complete.o connection.o globals.o log.o options.o $(LDFLAGS)

rdqmput : ../rdqmput.c complete.o connection.o globals.o log.o options.o
	$(CC) $(CCFLAGS) -o rdqmput ../rdqmput.c complete.o connection.o globals.o log.o options.o $(LDFLAGS)

.PHONY : clean
clean:
	rm -f *.o rdqmget rdqmput

.PHONY : install
install : $(prefix)/bin/rdqmget $(prefix)/bin/rdqmput

$(prefix)/bin/rdqmget : rdqmget
	install rdqmget $(prefix)/bin

$(prefix)/bin/rdqmput : rdqmput
	install rdqmput $(prefix)/bin

.PHONY : uninstall
uninstall:
	rm -f $(prefix)/bin/rdqmget $(prefix)/bin/rdqmput
