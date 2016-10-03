EXECS=send_recv #ping_pong ring
MPICC?=mpicc -std=c++11

all: ${EXECS}

send_recv: send_recv.cpp MPIntf.cpp
	${MPICC} -o send_recv send_recv.cpp MPIntf.cpp

#ping_pong: ping_pong.c
#	${MPICC} -o ping_pong ping_pong.c

#ring: ring.c
#	${MPICC} -o ring ring.c

clean:
	rm -f ${EXECS}
