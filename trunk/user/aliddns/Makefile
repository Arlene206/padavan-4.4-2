THISDIR = $(shell pwd)
all:

clean:

romfs:
	$(ROMFSINST) -p +x $(THISDIR)/aliddns.sh /usr/bin/aliddns.sh
	$(ROMFSINST) -p +x $(THISDIR)/aliddns-ipv4.sh /usr/bin/aliddns-ipv4.sh
	$(ROMFSINST) /etc_ro/ddns_script.sh