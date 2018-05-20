# This makefile triggers the targets in the application.mk

# The default value "../../.." assumes that this makefile is placed in the
# folder xdk110/Apps/<App Folder> where the BCDS_BASE_DIR is the parent of
# the xdk110 folder.
BCDS_BASE_DIR ?= ../../..

# Macro to define Start-up method. change this macro to "CUSTOM_STARTUP" to have custom start-up.
export BCDS_SYSTEM_STARTUP_METHOD = DEFAULT_STARTUP
export BCDS_APP_NAME = XdkHeadTrack
export BCDS_APP_DIR = $(CURDIR)
export BCDS_APP_SOURCE_DIR = $(BCDS_APP_DIR)/source

# Default WiFi settings
# Modes:
#	0 - no security (requires: APP_WIFI_SSID)
#	1 - WEP (requires: APP_WIFI_SSID, APP_WIFI_PASSPHRASE)
#	2 - WPA (requires: APP_WIFI_SSID, APP_WIFI_PASSPHRASE)
#	3 - WPS (requires: nothing)
#	4 - WPS (requires: nothing -> static PIN: 35072317)
APP_WIFI_MODE = 3
APP_WIFI_SSID = \"TEST_SSID\"
APP_WIFI_PASSPHRASE = \"TEST_PASSPHRASE\"

# Please refer BCDS_CFLAGS_COMMON variable in application.mk file
# and if any addition flags required then add that flags only in the below macro
export BCDS_CFLAGS_COMMON += \
	-D APP_WIFI_SSID=$(APP_WIFI_SSID) \
	-D APP_WIFI_PASSPHRASE=$(APP_WIFI_PASSPHRASE) \

# List all the application header file under variable BCDS_XDK_INCLUDES
export BCDS_XDK_INCLUDES = \
	-I$(BCDS_APP_DIR)/include \

# List all the application source file under variable BCDS_XDK_APP_SOURCE_FILES in a similar pattern as below
export BCDS_XDK_APP_SOURCE_FILES = \
	$(BCDS_APP_SOURCE_DIR)/BleUi.c \
	$(BCDS_APP_SOURCE_DIR)/ButtonUi.c \
	$(BCDS_APP_SOURCE_DIR)/HeadTrack.c \
	$(BCDS_APP_SOURCE_DIR)/LedAnimator.c \
	$(BCDS_APP_SOURCE_DIR)/Logger.c \
	$(BCDS_APP_SOURCE_DIR)/Main.c \
	$(BCDS_APP_SOURCE_DIR)/WifiUi.c \

.PHONY: clean	debug release flash_debug_bin flash_release_bin

clean: 
	$(MAKE) -C $(BCDS_BASE_DIR)/xdk110/Common -f application.mk clean

debug: 
	$(MAKE) -C $(BCDS_BASE_DIR)/xdk110/Common -f application.mk debug

release: 
	$(MAKE) -C $(BCDS_BASE_DIR)/xdk110/Common -f application.mk release
	
clean_Libraries:
	$(MAKE) -C $(BCDS_BASE_DIR)/xdk110/Common -f application.mk clean_libraries
	
flash_debug_bin: 
	$(MAKE) -C $(BCDS_BASE_DIR)/xdk110/Common -f application.mk flash_debug_bin
	
flash_release_bin: 
	$(MAKE) -C $(BCDS_BASE_DIR)/xdk110/Common -f application.mk flash_release_bin

cleanlint: 
	$(MAKE) -C $(BCDS_BASE_DIR)/xdk110/Common -f application.mk cleanlint
	
lint: 
	$(MAKE) -C $(BCDS_BASE_DIR)/xdk110/Common -f application.mk lint
	
cdt:
	$(MAKE) -C $(BCDS_BASE_DIR)/xdk110/Common -f application.mk cdt	
