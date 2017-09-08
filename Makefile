CC=gcc


IDIR=include
BDIR=build
ODIR=obj
LDIR=lib
SDIR=src
SRCEXT := c
CFLAGS=-I$(IDIR) `pkg-config --cflags --libs glib-2.0` -Wall -O2
#LIBS=-lconfig -lbluetooth -lsqlite3 -lpthread -lmhash -luuid
LIBS=-lm -ljson-c -lbluetooth -lmhash

APP_NAME=bridge

#SOURCES=main.c att.c btio.c crypto.c gatt.c gattrib.c log.c util.c uuid.c utils.c check_internet.c
_SOURCES=$(shell find $(SDIR) -type f -name *.$(SRCEXT))
#_SOURCES=$(addprefix $(SDIR)/,$(SOURCES))

_DEPS=$(_SOURCES:.c=.h)
DEPS=$(patsubst %,$(IDIR)/%,$(_DEPS))

OBJ=$(_SOURCES:.c=.o)
OBJ=$(patsubst $(SDIR),$(ODIR),$(_SOURCES))


EXECUTABLE=$(BDIR)/$(APP_NAME)

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(EXECUTABLE): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o $(BDIR)/*
