include $(ROOTDIR)/rules.mk

PKG_NAME:=shared
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

$(eval $(call BuildPackage,shared))

romfs:
	$(INSTALL_DIR) $(ROMFSDIR)/lib
	$(CP) $(PKG_BUILD_DIR)/libshared.so $(ROMFSDIR)/lib/
	$(INSTALL_DIR) $(ROMFSDIR)/usr/lib
	$(INSTALL_BIN) $(CURDIR)/scripts/process_lifecycle.sh $(ROMFSDIR)/usr/lib/process_lifecycle.sh