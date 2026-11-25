#
# U2EC Makefile
#
# Copyright (C) 2008 ASUSTeK Corporation
#
#

.DEFAULT_GOAL := all

PROGS	= u2ec
OBJS	= usbsock.o testusb.o

usbsock.o: u2ec_socket_io_policy.h u2ec_discovery_snapshot_policy.h
DEBUG	= n

ifeq ($(DEBUG),y)
	DEBFLAGS = -O -g -DU2EC_DEBUG
	DEBFLAGS += -DPDEBUG_SENDSECV
	DEBFLAGS += -DPDEBUG_DECODE
	OBJS += debug.o decode.o
else
	DEBFLAGS = -s
endif

CFLAGS += -DSUPPORT_LPRng
CFLAGS += -Wall -I.
CFLAGS += $(DEBFLAGS)

LDFLAGS += -lpthread
LDFLAGS += -lshared
LDFLAGS += -lusb

all: $(PROGS)

$(PROGS): $(OBJS)
	$(CC) -o $(PROGS) $(OBJS) $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(PROGS) *.o *~
