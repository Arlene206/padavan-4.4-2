include $(ROOTDIR)/rules.mk

PKG_NAME:=busybox
PKG_VERSION:=1.24

include $(INCLUDE_DIR)/package.mk

SRC_DIR=./busybox-1.24.x

# The package builds from trunk/build/busybox-1.24, outside the source tree.
# Keep the shared DHCPv6 DUID policy header discoverable after SRC_DIR is copied.
EXTRA_CFLAGS += -I$(ROOTDIR)/user/shared/src

$(eval $(call BuildPackage,busybox))

romfs:
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(MAKE_FLAGS) \
		CONFIG_PREFIX=$(ROMFSDIR) \
		install
