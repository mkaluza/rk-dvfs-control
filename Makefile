# Makefile.local should contain KDIR variable pointing to kernel source tree
# it can also include all other settings specific to you
-include Makefile.local

obj-m += dvfs.o

all: dvfs

dvfs:
	make -C $(KDIR) M=$(PWD) CFLAGS_MODULE=-fno-pic dvfs.ko

monitor_clean:
	make -C $(KDIR) M=$(PWD) clean
