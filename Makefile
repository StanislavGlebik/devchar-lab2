obj-m+=ker.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	rm -rf *.o *.ko *.mod.c *.symvers *.order
	rm -rf .ker.ko.cmd .ker.mod.o.cmd .ker.o.cmd .tmp_versions

