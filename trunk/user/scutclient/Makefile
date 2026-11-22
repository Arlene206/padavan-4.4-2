# https://github.com/hanwckf/scutclient.git
SRC_NAME := scutclient-upstream
SRC_FILE := scutclient-a58996875c61d0cdf8ec435057befc1c87bd6863
THISDIR = $(shell pwd)

all: config_test
	$(MAKE) -j$(HOST_NCPU) -C $(SRC_NAME)/build

extract_test:
	( if [ ! -d $(THISDIR)/$(SRC_NAME) ]; then \
		tar -xf $(SRC_FILE).tar.gz; \
	fi )

apply_credentials_overlay: extract_test
	( if ! grep -q 'scutclient_load_credentials_file' $(SRC_NAME)/src/main.c || \
	      ! grep -q 'src/scutclient_credentials.c' $(SRC_NAME)/CMakeLists.txt; then \
		patch -d $(SRC_NAME) -p1 < $(THISDIR)/patches/0001-private-credentials-file.patch || exit 1; \
		rm -f ./config_done; \
	fi; \
	cp $(THISDIR)/patches/scutclient_credentials.c $(SRC_NAME)/src/scutclient_credentials.c && \
	cp $(THISDIR)/patches/scutclient_credentials.h $(SRC_NAME)/src/scutclient_credentials.h )

config_test: apply_credentials_overlay
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	( mkdir -p $(SRC_NAME)/build && cd $(SRC_NAME)/build ; \
		cmake -DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_TOOLCHAIN_FILE=$(CONFIG_CMAKE_TOOLCHAIN_FILE) \
		.. \
	)

clean:
	rm -rf $(SRC_NAME)/build
	rm -f config_done

romfs:
	${ROMFSINST} $(THISDIR)/scripts/ /usr/bin/
	$(ROMFSINST) -p +x $(THISDIR)/$(SRC_NAME)/build/scutclient /usr/bin/bin_scutclient
