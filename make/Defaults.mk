
CC=cc
CXX=c++
LD=c++

#CWARNFLAGS.gcc += -Wno-expansion-to-defined -Wno-extra -Wno-unused-but-set-variable

C_OPTIM=-O3 -fno-omit-frame-pointer

CXX_STD=-std=c++17
CXX_WARNFLAGS=-Wall -Werror -Wno-user-defined-literals

CFLAGS:=-I/usr/local/include -I$(TOPDIR)/include $(C_OPTIM) -g -DBSD_VISIBLE \
    -DKLD_MODULE -DSMP -DINVARIANTS -DINVARIANT_SUPPORT

C_ONLY_FLAGS := -I$(TOPDIR)/include/kern_include -D_KERNEL -nostdinc
CXXFLAGS:=$(CXX_STD) $(CXX_WARNFLAGS)