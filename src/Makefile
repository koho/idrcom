TOPDIR = $(shell pwd)
BIN_DIR = /usr/local/bin
CONF_DIR = /etc
INSTALL = sudo install
INCLUDE = -I$(TOPDIR)
TARGET = idrcom
CONF = idrcom.conf
LIBS = -lpthread
OBJSDIR = common core service worker
CFLAGS += -fno-exceptions -Wno-deprecated-declarations
export TOPDIR INCLUDE CFLAGS

$(TARGET) : $(OBJSDIR)
	$(MAKE) -C common
	$(MAKE) -C core
	$(MAKE) -C service
	$(MAKE) -C worker
	$(CC) $(CFLAGS) $(INCLUDE) -c main.cpp -o main.o
	$(CC) $(LDFLAGS) main.o \
					 common/*.o \
					 core/*.o \
					 service/*.o \
					 worker/*.o $(LIBS) -o bin/$(TARGET)

install:
	$(INSTALL) -d $(BIN_DIR)
	$(INSTALL) -m 0755 bin/$(TARGET) $(BIN_DIR)
	$(INSTALL) -d $(CONF_DIR)
	$(INSTALL) ../files/$(CONF) $(CONF_DIR)

uninstall:
	rm -f $(BIN_DIR)/$(TARGET)
	sudo rm -f $(CONF_DIR)/$(CONF)

clean:
	rm -rf bin/$(TARGET)
	rm common/*.o core/*.o service/*.o worker/*.o main.o
