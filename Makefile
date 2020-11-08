# Debian-ish
# sudo apt-get install linux-headers-$(uname -r)
#
# Raspberry Pi
# sudo apt-get install raspberrypi-kernel-headers
#
# RedHat-ish
# sudo dnf install kernel-devel kernel-headers

obj-m += pnotify.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	$(RM) inotify-example.o inotify-example

CFLAGS += -g -Wall
inotify-example:inotify-example.o
