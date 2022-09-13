include $(TOPDIR)/rules.mk

PKG_NAME:=openvpn-ubus
PKG_RELEASE:=1
PKG_VERSION:=1.0.0

include $(INCLUDE_DIR)/package.mk

define Package/openvpn-ubus
	CATEGORY:=Base system
	TITLE:=Openvpn ubus
endef

define Package/openvpn-ubus/description
	Daemon to monitor openvpn server
endef

define Package/openvpn-ubus/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/openvpn_ubus $(1)/usr/bin
	$(INSTALL_BIN) ./files/openvpn_ubus.init $(1)/etc/init.d/openvpn_ubus
	$(INSTALL_CONF) ./files/openvpn_ubus.config $(1)/etc/config/openvpn_ubus
endef

$(eval $(call BuildPackage,openvpn-ubus))