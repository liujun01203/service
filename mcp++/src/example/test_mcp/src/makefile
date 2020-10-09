PUBLIC = ../../mcp++/src
INC =		-I${PUBLIC}/mcd/. -I${PUBLIC}/old/. -I${PUBLIC}/base/. -I${PUBLIC}/ccd/. -I${PUBLIC}/dcc/. -I${PUBLIC}/watchdog/. -I${PUBLIC}/wtg/. -I${PUBLIC}/libhttp/.
LIB =		-L${PUBLIC}/watchdog/. -lwatchdog -L${PUBLIC}/libhttp/. -lhttp

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

OBJS =		${PUBLIC}/ccd/tfc_net_ipc_mq.o	\
			${PUBLIC}/old/tfc_ipc_sv.o	\
			${PUBLIC}/mcd/tfc_cache_proc.o	\
			${PUBLIC}/base/tfc_base_config_file.o	\
			${PUBLIC}/libhttp/tfc_base_http.o	\
			${PUBLIC}/ccd/myalloc.o	\
			${PUBLIC}/ccd/tfc_net_open_shmalloc.o	\
			${PUBLIC}/watchdog/tfc_base_watchdog_client.o

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
	cp ${PUBLIC}/ccd/ccd ../../bin/simple_http_ccd
	cp ${PUBLIC}/mcd/mcd ../../bin/simple_http_mcd
	cp ${PUBLIC}/watchdog/watchdog ../../bin/simple_http_watchdog
	cp ${PUBLIC}/libhttp/http_complete.so ../../bin/http_complete.so
	cp ${PUBLIC}/example/simple_http_mcd.so ../../bin/simple_http_mcd.so

clean:
	rm -rf *.o *.gch simple_http_mcd.so
	rm -rf $(OBJS)


