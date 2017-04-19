# Comment/uncomment the following line to disable/enable debugging
DEBUG = y

# Add your debugging flag (or not) to CFLAGS
ifeq ($(DEBUG),y)
  DEBFLAGS = -g -O -DNL_DEBUG
else
  DEBFLAGS = -O2
endif

EXTRA_CFLAGS += $(DEBFLAGS)


obj-m += nl_kernel.o

TARGET_KO=nl_kernel.ko
TARGET_APP=nl_user

kern:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

user:
	gcc $(EXTRA_CFLAGS) -Wall -o $(TARGET_APP) nl_user.c

sign:
	$(shell /lib/modules/4.9.19/build/scripts/sign-file sha512 ~/private/test/kernel/linux-4.9.19/certs/signing_key.pem ~/private/test/kernel/linux-4.9.19/certs/signing_key.x509 $(TARGET_KO))

test: sign
	$(shell rmmod $(TARGET_KO))
	$(shell insmod $(TARGET_KO))
	dmesg | tail

all:clean kern test user

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -rf $(TARGET_APP)
