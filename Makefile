CC      = gcc
CFLAGS  = -Wall -g -pthread -I/usr/include/tirpc
LDLIBS  = -ltirpc -pthread

GEN     = calc.h calc_xdr.c calc_clnt.c calc_svc.c

all: calc_client calc_server

$(GEN): calc.x
	rpcgen -M calc.x

calc_server: calc_svc.o calc_xdr.o calc_server.o calc_ops.o
	$(CC) -o $@ $^ $(LDLIBS)

calc_client: calc_clnt.o calc_xdr.o calc_client.o calc_ops.o
	$(CC) -o $@ $^ $(LDLIBS)

%.o: %.c calc.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(GEN) *.o calc_client calc_server

.PHONY: all clean
