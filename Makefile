#
# Makefile -- 
#

PROG		= DecisionUsb
VERSION		= 1.0
NAME		= $(PROG)-$(VERSION)

ifneq (,)
This makefile requires GNU Make.
endif

.SUFFIXES:


#
# fsclient output files
#

OBJS			= dcihid.o main.o
EXES			= DecisionUsb
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

.PHONY:		install
install:	$(EXES)
	[ -d ${IMGDIR}  ] || install -d ${IMGDIR}
	install -m 755 ${EXES} ${IMGDIR}

.PHONY:	deploy
deploy:	$(EXES)
	[ -d ${EXEDIR}  ] || install -d ${EXEDIR}
	install -m 755 ${EXES} ${EXEDIR}

.PHONY:	clean distclean
distclean clean:
	-rm -f *.o *.s *~ core a.out build-stamp $(LIB) $(EXES)
