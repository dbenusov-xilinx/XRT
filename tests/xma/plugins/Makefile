#CC    = gcc
CC    = g++

xrt_install_path := "/opt/xilinx/xrt"
ifdef XRT_INSTALL_PATH
	xrt_install_path := ${XRT_INSTALL_PATH}
endif

CFLAGS       = -std=c++11 -fPIC -g -I. -I${xrt_install_path}/include -I${XMA_INCLUDE}
#CFLAGS       = -fPIC -g -I. -I../../../src/xma/include
LDFLAGS      = -shared

SOURCES = $(shell echo *.c)
HEADERS = $(shell echo *.h)
OBJECTS = $(SOURCES:.c=.o)
#TARGET  = example.so
TARGETS = $(SOURCES:.c=.so)

#PREFIX = $(DESTDIR)/usr/local
#BINDIR = $(PREFIX)/bin

#%.o: %.c $(HEADERS)
%.o: %.c
	$(CC) -c $^ $(CFLAGS)

%.so: %.o 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

.PHONY: all
all: $(TARGETS)


.PHONY : clean
clean:
	rm -rf $(OBJECTS) $(TARGETS)

