EXEC = ralinkiappd
OBJS = rtmpiapp.o mt_iapp_util.o
DEPFILES = $(OBJS:.o=.d)
.DEFAULT_GOAL := $(EXEC)

# If you want to debug daemon, add following line
CFLAGS += -I./ -DDBG -DSYSLOG -fvisibility=hidden -MMD -MP
#-DSYSLOG
-include $(DEPFILES)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

clean:
	rm -f core *.o *.d ralinkiappd
