#
# Copyright (C) 2018 Guohao Tan <koho@flyinglight.org>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=idrcom
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/idrcom
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Lightweight Stable Dr.COM Client
	MAINTAINER:=Guohao Tan <koho@flyinglight.org>
	URL:=https://github.com/koho/idrcom
	DEPENDS:=+libpthread
endef

define Package/idrcom/description
	idrcom is a lightweight stable Dr.COM client for embedded devices.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/idrcom/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/idrcom $(1)/usr/bin

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) files/idrcom.init $(1)/etc/init.d/idrcom

	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_CONF) files/idrcom.conf $(1)/etc/idrcom.conf
endef

define Package/idrcom/postrm
	#!/bin/sh
	rm -rf /tmp/idrcom.log
	rm -rf /etc/rc.d/S100idrcom
	rm -rf /etc/rc.d/K20idrcom
endef

$(eval $(call BuildPackage,idrcom))

