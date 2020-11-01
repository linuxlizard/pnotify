# Debian-ish
# sudo apt-get install linux-headers-$(uname -r)
#
# RedHat-ish
# sudo dnf install kernel-devel kernel-headers

obj-m += pnotify.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
