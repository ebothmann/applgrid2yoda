# APPLgrid flags
AFLAGS=$(shell applgrid-config --cxxflags)
ALDFLAGS=$(shell applgrid-config --ldflags)

# LHAPDF flags
LHAFLAGS= $(shell lhapdf-config --cppflags)
LHALDFLAGS=  $(shell lhapdf-config --ldflags)

# YODA flags
YODAFLAGS=$(shell yoda-config --cppflags)
YODALDFLAGS=$(shell yoda-config --ldflags)

# Combined flags
TSTFLAGS=$(AFLAGS) $(LHAFLAGS) $(YODAFLAGS)
TSTLDFLAGS=$(ALDFLAGS) $(LHALDFLAGS) $(YODALDFLAGS)

.PHONY:	all test clean

all:	test

test:
	g++ -Wall $(TSTFLAGS) $(TSTLDFLAGS) applgrid2yoda.cpp -o applgrid2yoda
	
clean:
	rm -f applgrid2yoda
