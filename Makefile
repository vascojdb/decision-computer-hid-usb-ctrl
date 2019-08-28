#
# Makefile -- 
#

PROG		= DecisionUsbdio
VERSION		= 1.0
NAME		= $(PROG)-$(VERSION)
IMGDIR		= /usr/local/bin

ifneq (,)
This makefile requires GNU Make.
endif

.SUFFIXES:


#
# fsclient output files
#

OBJS			= dcihid.o main.o
EXES			= DecisionUsbdio
HEADERS			= 


#
# compiler-specific options
#

CC		= gcc

CFLAGS		= -O -g
XTRACFLAGS	= -Wall -pipe -I.
XTRACFLAGS	+=

LDFLAGS		=
XTRALDFLAGS	=

LIB		=
RANLIB		= ranlib

STRIP		= strip --strip-unneeded

#
# rules
#

all:	$(EXES)

%.o: %.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $(XTRACFLAGS) $<

$(EXES): $(OBJS) $(LIB)
	$(CC) -o $@ $(OBJS) $(LDFLAGS) $(XTRALDFLAGS)
	$(STRIP) $@
	@echo Compilation done.

.PHONY:		install
install:	$(EXES)
	@echo Installing in ${IMGDIR}...
	[ -d ${IMGDIR}  ] || install -d ${IMGDIR}
	install -m 755 ${EXES} ${IMGDIR}
	@echo Install concluded.

.PHONY:	clean distclean
distclean clean:
	@echo Cleaning up...
	-rm -f *.o *.s *~ core a.out build-stamp $(LIB) $(EXES)
	@echo All cleaned.
