needham_dir = ..
current_dir = .

SOURCES:= ns_server.c ns_client.c ns_daemon.c ns_driver_blowfish.c ns_driver_rijndael.c ns_driver_no_server.c ns_driver_no_daemon.c 
EXECUTABLES:= $(patsubst %.c, %, $(SOURCES))
CFLAGS:=-Wall -g
CPPFLAGS:=-I$(needham_dir) -I$(current_dir)
LDFLAGS:= -L$(needham_dir) -L$(current_dir)
LDLIBS:=-lneedham -lns_API -lssl -lcrypto


.PHONY: all clean $(SOURCES)

all: libns_API.a $(EXECUTABLES) 

libns_API.a: ns_API.o
	$(AR) $(ARFLAGS) $@ $^
	ranlib $@
ns_API.o: ns_API.c
	gcc -c ns_API.c $(CPPFLAGS)


clean:
	rm -rf $(EXECUTABLES) $(MYLIB) $(OBJECTS) ns_API.o

	for exe in $(EXECUTABLES) ; do \
		rm -rf $(addsuffix .dSYM,$$exe) ; \
	done
