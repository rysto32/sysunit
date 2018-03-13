
CC=cc
CXX=c++
LD=c++

#CWARNFLAGS.gcc += -Wno-expansion-to-defined -Wno-extra -Wno-unused-but-set-variable

C_OPTIM=-O3 -fno-omit-frame-pointer

# Compiling the UT code with optimization enabled takes several seconds per
# test file and doesn't give a lot of benefit, so don't bother with -O
CXX_OPTIM=-fno-omit-frame-pointer

CXX_STD=-std=c++17
CXX_WARNFLAGS=-Wall -Werror -Wno-user-defined-literals

CFLAGS:=-I/usr/local/include -I$(TOPDIR)/include -g -DBSD_VISIBLE \
    -DKLD_MODULE -DSMP -DINVARIANTS -DINVARIANT_SUPPORT -Werror

C_ONLY_FLAGS := -I$(TOPDIR)/include/kern_include $(C_OPTIM)  -nostdinc \
    -D_KERNEL_UT -D_KERNEL_UT_MALLOC -D_KERNEL_UT_SYSTM_LIBKERN -D_KERNEL_UT_PAUSE \
    -Wno-incompatible-library-redeclaration -Wno-address-of-packed-member \
    -Wno-format-invalid-specifier -Wno-format

CXXFLAGS:=$(CXX_STD) $(CXX_WARNFLAGS) $(CXX_OPTIM)

LDFLAGS := -Wl,-L,/usr/local/lib
