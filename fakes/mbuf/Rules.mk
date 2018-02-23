
LIB :=	fake_mbuf

SRCS := \
	mbuf.c \
	MbufInit.cpp \
	uipc_mbuf.c \

LOCAL_INCLUDE := -I $(TOPDIR)/include/kern_include/
