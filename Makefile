DESTDIR ?= ${HOME}/Boost/
SOURCE1 = udp_client.cpp
SOURCE2 = udp_server.cpp

all: server client

server: ${SOURCE2}
	g++ ${SOURCE2} -g -o $@ -pthread -lboost_program_options

client: ${SOURCE1}
	g++ ${SOURCE1} -g -o $@ -pthread -lboost_program_options

clean:
	${RM} server
	${RM} client

instal: all
	mkdir -p ${DESTDIR}
	cp server ${DESTDIR}
	cp client ${DESTDIR}

.PHONY: all clean install
