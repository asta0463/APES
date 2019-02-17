LINUXDIR := ~/Documents/buildroot/output/build/linux-4796173fc58688055a99a1cef19a839174067220/

obj-m +=timermodule.o

all:
	make ARCH=arm CROSS_COMPILE=arm-linux- -C $(LINUXDIR) M=$(shell pwd)

clean:
	make -C $(LINUXDIR) M=$(shell pwd) clean
