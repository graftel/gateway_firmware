CC=gcc


IDIR=include
BDIR=build
ODIR=obj
LDIR=lib
SDIR=src
SRCEXT := c
CFLAGS=-I$(IDIR) `pkg-config --cflags --libs glib-2.0` -Wall -O2 -g
#LIBS=-lconfig -lbluetooth -lsqlite3 -lpthread -lmhash -luuid
LIBS=-lm -ljson-c -lbluetooth -lsqlite3

APP_NAME=bridge

#IoT client directory
IOT_CLIENT_DIR = ~/aws-lib

PLATFORM_DIR = $(IOT_CLIENT_DIR)/platform/linux/mbedtls
PLATFORM_COMMON_DIR = $(IOT_CLIENT_DIR)/platform/linux/common

IOT_INCLUDE_DIRS += -I $(IOT_CLIENT_DIR)/include
IOT_INCLUDE_DIRS += -I $(IOT_CLIENT_DIR)/external_libs/jsmn
IOT_INCLUDE_DIRS += -I $(PLATFORM_COMMON_DIR)
IOT_INCLUDE_DIRS += -I $(PLATFORM_DIR)

IOT_SRC_FILES += $(shell find $(IOT_CLIENT_DIR)/src/ -name '*.c')
IOT_SRC_FILES += $(shell find $(IOT_CLIENT_DIR)/external_libs/jsmn -name '*.c')
IOT_SRC_FILES += $(shell find $(PLATFORM_DIR)/ -name '*.c')
IOT_SRC_FILES += $(shell find $(PLATFORM_COMMON_DIR)/ -name '*.c')

#TLS - mbedtls
MBEDTLS_DIR = $(IOT_CLIENT_DIR)/external_libs/mbedTLS
TLS_LIB_DIR = $(MBEDTLS_DIR)/library
TLS_INCLUDE_DIR = -I $(MBEDTLS_DIR)/include
EXTERNAL_LIBS += -L$(TLS_LIB_DIR)
LD_FLAG += $(CFLAGS)
LD_FLAG += -Wl,-rpath,$(TLS_LIB_DIR)
LD_FLAG += -ldl $(TLS_LIB_DIR)/libmbedtls.a $(TLS_LIB_DIR)/libmbedcrypto.a $(TLS_LIB_DIR)/libmbedx509.a -lpthread

#Aggregate all include and src directories
INCLUDE_ALL_DIRS += $(IOT_INCLUDE_DIRS)
INCLUDE_ALL_DIRS += $(TLS_INCLUDE_DIR)
#INCLUDE_ALL_DIRS += $(APP_INCLUDE_DIRS)


#SOURCES=main.c att.c btio.c crypto.c gatt.c gattrib.c log.c util.c uuid.c utils.c check_internet.c
_SOURCES=$(shell find $(SDIR) -type f -name *.$(SRCEXT))
#_SOURCES=$(addprefix $(SDIR)/,$(SOURCES))

SRC_FILES += $(_SOURCES)
SRC_FILES += $(IOT_SRC_FILES)

# Logging level control
LOG_FLAGS += -DENABLE_IOT_DEBUG
LOG_FLAGS += -DENABLE_IOT_INFO
LOG_FLAGS += -DENABLE_IOT_WARN
LOG_FLAGS += -DENABLE_IOT_ERROR

COMPILER_FLAGS += $(LOG_FLAGS)
#If the processor is big endian uncomment the compiler flag
#COMPILER_FLAGS += -DREVERSED

MBED_TLS_MAKE_CMD = $(MAKE) -C $(MBEDTLS_DIR)

PRE_MAKE_CMD = $(MBED_TLS_MAKE_CMD)
MAKE_CMD = $(CC) $(SRC_FILES) $(COMPILER_FLAGS) -o $(BDIR)/$(APP_NAME) $(LD_FLAG) $(EXTERNAL_LIBS) $(INCLUDE_ALL_DIRS) $(LIBS)


#_DEPS=$(_SOURCES:.c=.h)
#DEPS=$(patsubst %,$(IDIR)/%,$(_DEPS))

#OBJ=$(_SOURCES:.c=.o)
#OBJ=$(patsubst $(SDIR),$(ODIR),$(_SOURCES))

#EXECUTABLE=$(BDIR)/$(APP_NAME)

#$(ODIR)/%.o: %.c $(DEPS)
#	$(CC) -c -o $@ $< $(CFLAGS)

#$(EXECUTABLE): $(OBJ)
#	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)


all:
	$(PRE_MAKE_CMD)
	$(DEBUG)$(MAKE_CMD)
	$(POST_MAKE_CMD)


.PHONY: clean

clean:
	rm -f $(ODIR)/*.o $(BDIR)/*
	$(MBED_TLS_MAKE_CMD) clean
