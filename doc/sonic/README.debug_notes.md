# VS KVM Notes
#

# Table of Contents
  * [List of Tables](#list-of-tables)
  * [1 Steps to build VS image](#1-steps-to-build-VS-image)
  * [2 Setup VS KVM](#1-setup-VS-KVM)
    * [2.1 Un-compress image](#21-un-compress-image)
    * [2.2 XML configuration file for virsh](#22-XML-configuration-file-for-virsh)
    * [2.3 Create SONiC VM via virsh](#23-create-SONiC-VM-via-virsh)
    * [2.4 Login to SONiC KVM via console](#24-login-to-SONiC-KVM-via-console)
    * [2.5 Login to VM via SSH](#25-login-to-VM-via-SSH)
      * [2.5.1 Request a new DHCP address](#251-request-a-new-DHCP-address)
      * [2.5.2 Connect via SSH](#252-connect-via-SSH)
  * [3 Build debian packages](#3-build-debian-packages)
    * [3.1 make clean a module](#31-make-clean-a-module)
    * [3.2 make a module](#32-make-a-module)
    * [3.3 Keeps slave container up](#33-keeps-slave-container-up)
  * [4 Update a module in VS](#4-Update-a-module-in-VS)
    * [4.1 Upload packages to VM](#41-Upload-packages-to-VM)
    * [4.2 Install whl in VM](#42-Install-whl-in-VM)
    * [4.3 Update packages into docker](#43-Update-packages-into-docker)
  * [5 Login Redis Database](#5-login-Redis-Database)
  * [6 Logging](#6-logging)
    * [6.1 docker log](#61-docker-log)
    * [6.2 syslog](#62-syslog)
  * [7 GNS3](#7-gns3)

# 1 Steps to build VS image
```
    make init

    //the ASIC default number is 1, please run the below command if you want to run Multi-ASIC simulation.
    echo "NUM_ASIC=6" > ./device/virtual/x86_64-kvm_x86_64-r0/asic.conf

    make configure PLATFORM=vs NOSTRETCH=1
    make NOSTRETCH=1 target/sonic-vs.img.gz
```
# 2 Setup VS KVM
## 2.1 Un-compress image
```
    falu@falu-ubuntu:~/ws/kvm$ gunzip sonic-multiasic-vs.img.gz
```
## 2.2 XML configuration file for virsh
Set the VM image location and replace the invalid QEMU option '-redir' in XML file sonic.xml under 'platform/vs'
My XML file is /localdata/falu/kvm/sonic.xml. 
```
    --- a/platform/vs/sonic.xml
    +++ b/platform/vs/sonic.xml
    @@ -22,7 +22,7 @@
         <emulator>/localdata/falu/share/bin/qemu-system-x86_64</emulator>
         <disk type='file' device='disk'>
           <driver name='qemu' type='qcow2' cache='writeback'/>
    -      <source file='/data/sonic/sonic-buildimage/target/sonic-vs.img'/>
    +      <source file='/localdata/falu/test/target/sonic-vs.img'/>
           <target bus='virtio' dev='vda'/>
         </disk>
         <serial type='tcp'>
    @@ -57,14 +57,6 @@
         <interface type='ethernet' />
    -    <interface type='ethernet' />
    -    <interface type='ethernet' />
    -    <interface type='ethernet' />
    -    <interface type='ethernet' />
         <controller type='usb' index='0'/>
         <memballoon model='virtio'>
           <alias name='balloon0'/>
    @@ -73,7 +65,9 @@
       </devices>
       <seclabel type='dynamic' model='apparmor' relabel='yes'/>
       <qemu:commandline>
    -   <qemu:arg value='-redir'/>
    -   <qemu:arg value='tcp:3040::22'/>
    +   <qemu:arg value='-device'/>
    +   <qemu:arg value='e1000,netdev=net0'/>
    +   <qemu:arg value='-netdev'/>
    +   <qemu:arg value='user,id=net0,hostfwd=tcp::5555-:22'/>
       </qemu:commandline> 
     </domain>
```

## 2.3 Create SONiC VM via virsh
```
    $ sudo virsh
    virsh # create sonic.xml
```

## 2.4 Login to SONiC KVM via console, and resize console width ( username/password: admin/YourPaSsWoRd )
```
    falu@falu-ubuntu:~/ws/kvm$ telnet 127.0.0.1 7000
    Trying 127.0.0.1...
    Connected to 127.0.0.1.
    Escape character is '^]'.
    sonic login: admin
    Password:
    admin@sonic:~$ stty cols 256
```
## 2.5 Login to VM via SSH
### 2.5.1 Request a new DHCP address
```
    falu@falu-ubuntu:~/ws/kvm$ telnet 127.0.0.1 7000
    Trying 127.0.0.1...
    Connected to 127.0.0.1.
    Escape character is '^]'.
    sonic login: admin
    Password:
    admin@sonic:~$ stty cols 256
    admin@sonic:~$ sudo dhclient -v
```
### 2.5.2 Connect via SSH
        ```
        $ ssh -p 5555 admin@127.0.0.1
        ```
# 3 Build debian packages
## 3.1 make clean a module
Every target has a clean target, so in order to clean sonic_utilities, execute:
```
    $ make NOSTRETCH=1 target/python-wheels/sonic_utilities-1.2-py3-none-any.whl-clean
```
## 3.2 make a module
Every module is built and saved to target/ directory. So, for instance, to build only sonic_utilities, execute: 
```
    $ make NOSTRETCH=1 target/python-wheels/sonic_utilities-1.2-py3-none-any.whl
```
## 3.3 Keeps slave container up
Build debian package inside the Docker slave container, and tell the build system to keep the container alive when finished, so that we can speed up to troubleshooting compile issue. [More details](https://github.com/Azure/sonic-utilities) 
```
    $ make NOSTRETCH=1 target/python-wheels/sonic_utilities-1.2-py3-none-any.whl KEEP_SLAVE_ON=yes
    fanqilu@653d748c3f31:/sonic$ cd src/sonic-utilities/

    //To build
    fanqilu@653d748c3f31:/sonic/src/sonic-utilities$ python3 setup.py bdist_wheel

    //To run unit tests
    fanqilu@653d748c3f31:/sonic/src/sonic-utilities$ python3 setup.py test
```
# 4 Update a module in VS
## 4.1 Upload packages to VM
```
    $ scp -P 5555 src/sonic-swss/debian/swss/usr/bin/l2mcmgrd admin@127.0.0.1:~/
```
## 4.2 Install whl in VM
Please uninstall whl package first, and then install it:
```
    admin@sonic:~$ sudo pip3 uninstall sonic_utilities
    admin@sonic:~$ sudo pip3 install ./sonic_utilities-1.2-py3-none-any.whl     
```
## 4.3 Update packages into docker
Upload packages to VM, and then copy it into Docker container:
```
    sudo docker cp l2mcmgrd l2mcd:/usr/bin/l2mcmgrd
```
# 5 Login Redis Database
Here is [Database Mapping](https://github.com/Azure/SONiC/blob/master/doc/database/multi_database_instances.md).
```
    // redis -n <db>   Database number.
    // Login CFG DB
    admin@sonic:~$ redis-cli -n 4
```
# 6 Logging
## 6.1 docker log
```
    admin@sonic:~$ docker logs l2mcd    
```
## 6.2 syslog
[More details](https://github.com/Azure/sonic-utilities/blob/master/doc/Command-Reference.md)
```
    //show logging [(<process_name> [-l|--lines <number_of_lines>]) | (-f|--follow)]

    admin@sonic:~$ show logging l2mcd
```
# 7 GNS3
https://plvision.eu/rd-lab/blog/sdn/sonic-network-os-configuration

