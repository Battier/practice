HOWTO Use Multi-ASIC VS KVM

1. Original Wiki page:
```
    https://github.com/Azure/sonic-buildimage/blob/master/platform/vs/README.vsvm.md
```

2. Download KVM images & build log
```
    wget https://sonic-jenkins.westus2.cloudapp.azure.com/job/vs/job/buildimage-multiasic-vs-image-201911/lastSuccessfulBuild/artifact/target/sonic-multiasic-vs.img.gz
    https://sonic-jenkins.westus2.cloudapp.azure.com/job/vs/job/buildimage-multiasic-vs-image-201911/lastSuccessfulBuild/artifact/target/sonic-vs.img.gz.log
```

3. Install tools
```
    sudo apt install cpu-checker qemu-kvm libvirt-daemon-system libvirt-clients bridge-utils virtinst virt-manager -y
    sudo systemctl is-active libvirtd
    sudo usermod -aG libvirt $USER
    sudo usermod -aG kvm $USER
```

4. Un-compress image
```
    falu@falu-ubuntu:~/ws/kvm$ gunzip sonic-multiasic-vs.img.gz
```

5. Set the VM image location and replace the invalid QEMU option '-redir' in XML file sonic_multiasic.xml under 'platform/vs'
```
    --- a/platform/vs/sonic_multiasic.xml
    +++ b/platform/vs/sonic_multiasic.xml
    @@ -22,7 +22,7 @@
         <emulator>/usr/bin/qemu-system-x86_64</emulator>
         <disk type='file' device='disk'>
           <driver name='qemu' type='qcow2' cache='writeback'/>
    -      <source file='/data/sonic/sonic-buildimage/target/sonic-vs.img'/>
    +      <source file='/home/falu/ws/kvm/sonic-vs.img'/>
           <target bus='virtio' dev='vda'/>
         </disk>
         <serial type='tcp'>
    @@ -57,14 +57,6 @@
         <interface type='ethernet' />
         <interface type='ethernet' />
         <interface type='ethernet' />
    -    <interface type='ethernet' />
    -    <interface type='ethernet' />
    -    <interface type='ethernet' />
    -    <interface type='ethernet' />
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

6. Create SONiC VM
```
    $ sudo virsh
    virsh # create sonic_multiasic.xml
```

7. Login to SONiC KVM, and resize console width ( username/password: admin/YourPaSsWoRd )
```
    falu@falu-ubuntu:~/ws/kvm$ telnet 127.0.0.1 7000
    Trying 127.0.0.1...
    Connected to 127.0.0.1.
    Escape character is '^]'.
    sonic login: admin
    Password:
    admin@sonic:~$ stty cols 256
```
8. Login to VM via SSH
    8.1. Connect via console (see 7 above)

    8.2. Request a new DHCP address
        ```
        sudo dhclient -v
        ```
        
    8.3. Connect via SSH
        ```
        $ ssh -p 5555 admin@127.0.0.1
        ```

9. Checking ASICs status
```
    admin@sonic:~$ show platform summary
    Warning: failed to retrieve PORT table from ConfigDB!
    Platform: x86_64-kvm_x86_64-r0
    HwSKU: None
    ASIC: vs
    ASIC Count: 6
```

10. Shutdown VM
```
    virsh # shutdown sonic
```
11. How to build a multi-ASIC VS VM image.
```
    sudo bash -c "falu ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers
    newgrp docker
    sudo modprobe overlay
    git clone https://github.com/Azure/sonic-buildimage.git
    make init
    echo "NUM_ASIC=6" > ./device/virtual/x86_64-kvm_x86_64-r0/asic.conf
    make configure PLATFORM=vs NOSTRETCH=1
    make NOSTRETCH=1 target/sonic-vs.img.gz
```

