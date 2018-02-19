
SUBDIRS=\
	fakes \
	mock \
	netinet \

.DEFAULT_GOAL:=all

all: test

TOPDIR:=.
CURDIR:=.
STACK=x

OUTDIR:=$(TOPDIR)/obj
OBJDIR:=$(OUTDIR)/objects
LIBDIR:=$(OUTDIR)/lib
INSTALLDIR:=$(OUTDIR)/staging/
DEPENDDIR:=$(OUTDIR)/depend/
TESTOBJDIR:=$(OUTDIR)/test_obj
TESTDIR:=$(OUTDIR)/test

include make/Subdirs.mk


.PHONY: test

test: $(TEST_PROGS)
	@for test in $(TEST_PROGS); do \
		limits -c 0 ./$${test} || break; \
	done

.PHONY: nothing
nothing:
	@true
