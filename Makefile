PRODUCTION		:= 0
PRODUCTION_VERSION	:= 0.0.1
PRODUCTION_YEAR		:= 2021

ifeq ($(PRODUCTION),1)
VERSION_TAG		:= $(PRODUCTION_VERSION)
else
VERSION_TAG		:= $(shell git describe --tags || echo $(PRODUCTION_VERSION))
endif
VERSION_YEAR		:= $(shell echo $(PRODUCTION_YEAR))

PREFIX		?=/usr/local
INSTALLDIR	= $(DESTDIR)$(PREFIX)/bin

HOSTOS		:= $(shell uname -s)
GPIOSUPPORT=off

CC		?= gcc
CFLAGS		?= -O3 -Wall -Wextra
CFLAGS 		+= -std=gnu99
INSTFLAGS	= -m 0755

ifeq ($(HOSTOS), Linux)
INSTFLAGS += -D
endif

all: build

build:
ifeq ($(HOSTOS), Linux)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o hcxbtdumptool hcxbtdumptool.c $(LDFLAGS) -lbluetooth -lcrypto -lssl -DVERSION_TAG=\"$(VERSION_TAG)\" -DVERSION_YEAR=\"$(VERSION_YEAR)\"
else
	$(info OS not supported)
endif


install: build
ifeq ($(HOSTOS), Linux)
	install $(INSTFLAGS) hcxbtdumptool $(INSTALLDIR)/hcxbtdumptool
else
	$(info OS not supported)
endif


ifeq ($(HOSTOS), Linux)
	rm -f hcxbtdumptool
else
	$(info OS not supported)
endif
	rm -f *.o *~


clean:
ifeq ($(HOSTOS), Linux)
	rm -f hcxbtdumptool
else
	$(info OS not supported)
endif
	rm -f *.o *~


uninstall:
ifeq ($(HOSTOS), Linux)
	rm -f $(INSTALLDIR)/hcxbtdumptool
else
	$(info OS not supported)
endif
