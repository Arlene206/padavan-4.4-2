include $(ROOTDIR)/rules.mk

PKG_NAME:=aliddns
SRC_DIR:=./src

include $(INCLUDE_DIR)/package.mk

TARGET_CFLAGS += -I$(STAGING_DIR)/include
TARGET_LDFLAGS += -L$(STAGING_DIR)/lib

$(eval $(call BuildPackage,aliddns))

THISDIR = $(shell pwd)

romfs:
	$(INSTALL_DIR) $(ROMFSDIR)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/aliddns-hmac-sha1 $(ROMFSDIR)/usr/bin/aliddns-hmac-sha1
	$(ROMFSINST) -p +x $(THISDIR)/aliddns.sh /usr/bin/aliddns.sh
	$(ROMFSINST) -p +x $(THISDIR)/aliddns-ipv4.sh /usr/bin/aliddns-ipv4.sh
	$(ROMFSINST) /etc_ro/ddns_script.sh
