CC=gcc
CFLAGS=-I$(IDIR)

IDIR=include
BDIR=build
ODIR=obj
LDIR=lib
SDIR=src
LIBS=-lconfig -lbluetooth -lsqlite3 -lpthread -lmhash -luuid

APP_NAME=bridge

SOURCES=bridge.c sqlfunc.c dynamoDBfunc.c
_SOURCES=$(addprefix $(SDIR)/,$(SOURCES))

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
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 