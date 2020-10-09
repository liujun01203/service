INC =		-I../mcd/. -I../old/. -I../base/. -I../ccd/. -I../dcc/. -I../watchdog/. -I../wtg/. -I../libhttp/.
LIB =		-L../watchdog/. -lwatchdog -L../libhttp/. -lhttp

X86_64 = 0
SYSBIT = $(shell getconf LONG_BIT)
ifeq ($(SYSBIT),64)
	X86_64 = 1
endif

ifeq ($(X86_64),0)
	CFLAGS =	-g -Wall -fno-strict-aliasing -D_GNU_SOURCE
else
	CFLAGS =	-g -Wall -fno-strict-aliasing -D_GNU_SOURCE -fPIC
endif

OBJS =		../ccd/tfc_net_ipc_mq.o	\
			../old/tfc_ipc_sv.o	\
			../mcd/tfc_cache_proc.o	\
			../base/tfc_base_config_file.o	\
			../libhttp/tfc_base_http.o	\
			../ccd/myalloc.o	\
			../ccd/tfc_net_open_shmalloc.o	\
			../watchdog/tfc_base_watchdog_client.o

all: $(OBJS)
ifeq ($(X86_64),0)
	g++ $(CFLAGS) -shared -o simple_http_mcd.so simple_http_mcd.cpp $(OBJS) $(INC) $(LIB)
else
	g++ $(CFLAGS) -shared -o simple_http_mcd.so simple_http_mcd.cpp $(OBJS) $(INC) $(LIB)
endif

%.o: %.cpp
	$(CXX) $(CFLAGS) $(INC) -c -o $@ $<
%.o: %.c
	$(CXX) $(CFLAGS) $(INC) -c -o $@ $<

install:
	cp ../ccd/ccd ../../bin/simple_http_ccd
	cp ../mcd/mcd ../../bin/simple_http_mcd
	cp ../watchdog/watchdog ../../bin/simple_http_watchdog
	cp ../libhttp/http_complete.so ../../bin/http_complete.so
	cp ../example/simple_http_mcd.so ../../bin/simple_http_mcd.so

clean:
	rm -rf *.o *.gch simple_http_mcd.so
	rm -rf $(OBJS)

