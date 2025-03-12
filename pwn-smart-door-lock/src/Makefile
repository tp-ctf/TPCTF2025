CFLAGS=-Wall -ggdb -I../../lib -I../../lib/cpp -I../../arm32/include -I../../arm32/lib 
# CFLAGS=-Wall -O2 -s -I../../lib -I../../lib/cpp -I../../arm32/include -I../../arm32/lib 
LDFLAGS=-L../../lib ../../lib/cpp/libmosquittopp.a ../../lib/libmosquitto.a
CFLAGS += -I/path/to/toolchain/openssl-3.0.11/arm32/include -I/path/to/toolchain/cJSON/build/cjson-arm32/include
CXXFLAGS += -I/path/to/toolchain/openssl-3.0.11/arm32/include -I/path/to/toolchain/cJSON/build/cjson-arm32/include
LDFLAGS += -L/path/to/toolchain/openssl-3.0.11/arm32/lib  -L/path/to/toolchain/cJSON/build/cjson-arm32/lib -lssl -lcrypto -lcjson

#LDFLAGS=-L../../release/lib
.PHONY: all clean

all : mqtt_lock

mqtt_lock : main.o mqtt_lock.o
	${CXX} $^ -o $@ ${LDFLAGS} -static 

main.o : main.cpp
	${CXX} -c $^ -o $@ ${CFLAGS}

mqtt_lock.o : door_lock.cpp
	${CXX} -c $^ -o $@ ${CFLAGS}

clean :
	-rm -f *.o mqtt_lock
