HOWTO Use Multi-ASIC VS KVM

1. Original Wiki page:
    https://github.com/Azure/sonic-buildimage/blob/master/platform/vs/README.vsvm.md

2. Download KVM images & build log
    wget https://sonic-jenkins.westus2.cloudapp.azure.com/job/vs/job/buildimage-multiasic-vs-image-201911/lastSuccessfulBuild/artifact/target/sonic-multiasic-vs.img.gz
    https://sonic-jenkins.westus2.cloudapp.azure.com/job/vs/job/buildimage-multiasic-vs-image-201911/lastSuccessfulBuild/artifact/target/sonic-vs.img.gz.log

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
    falu@ubuntu:~/private/sonic/sonic-buildimage$ git diff platform/vs/sonic_multiasic.xml
    diff --git a/platform/vs/sonic_multiasic.xml b/platform/vs/sonic_multiasic.xml
    index b571b512..ad2a6c8f 100644
    --- a/platform/vs/sonic_multiasic.xml
    +++ b/platform/vs/sonic_multiasic.xml
    @@ -21,7 +21,7 @@
         <emulator>/usr/bin/qemu-system-x86_64</emulator>
         <disk type='file' device='disk'>
           <driver name='qemu' type='qcow2' cache='writeback'/>
    -      <source file='/data/sonic/sonic-buildimage/target/sonic-vs.img'/>
    +      <source file='/home/falu/private/sonic/sonic-buildimage/gns3/sonic-multiasic-vs.img'/>
           <target bus='virtio' dev='vda'/>
         </disk>
         <serial type='tcp'>
    @@ -104,7 +104,7 @@
       </devices>
       <seclabel type='dynamic' model='apparmor' relabel='yes'/>
       <qemu:commandline>
    -   <qemu:arg value='-redir'/>
    -   <qemu:arg value='tcp:3040::22'/>
    +   <qemu:arg value='-netdev'/>
    +   <qemu:arg value='user,id=ethernet.0,hostfwd=tcp::3040-:22'/>
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

8. Checking ASICs status
```
    admin@sonic:~$ show platform summary
    Warning: failed to retrieve PORT table from ConfigDB!
    Platform: x86_64-kvm_x86_64-r0
    HwSKU: None
    ASIC: vs
    ASIC Count: 6
```

9. Shutdown VM
```
    virsh # shutdown sonic
```
10. How to build a multi-ASIC VM KVM image

