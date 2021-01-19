# Speed up SONiC Build
## 1. Build image on US VPS
Lots of packages ( Debain tools, SONiC code, Python modules, and DEBs ) need to download when building SONiC image, and some of packages don't have any mirror at China. We can build it on US VPS server to speed up the SONiC image building.
### 1.1 VPS requirements
Recommend to use [Vultr](https://www.vultr.com/), it charges by hour.
```
    Location:  Sillicon Valley
    OS:        Ubuntu 20.04
    Disk:      ~300G
```
### 1.2 Steps to build `sonic-vs.img.gz`
#### 1.2.1 Build the necessary objects
Since my VPS doesn't support run VM, we cannot build the disk image it.
```
    ssh root@<ip>
    git clone https://github.com/Battier/practice.git;sh practice/scripts/dotfiles/setup_dotfiles.sh
    adduser falu    
    bash -c "falu ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers
    su falu
    newgrp docker
    sudo modprobe overlay
    git clone https://github.com/Azure/sonic-buildimage.git
    make init
    echo "NUM_ASIC=6" > ./device/virtual/x86_64-kvm_x86_64-r0/asic.conf
    make configure PLATFORM=vs NOSTRETCH=1
    make NOSTRETCH=1 target/sonic-vs.img.gz
```
#### 1.2.2 Download the necessary binaries
Download `target/sonic-vs.bin`, `fs.zip`,`fs.squashfs`, and `target/files/buster/onie-recovery-x86_64-kvm_x86_64-r0.iso` from VPS.
```
    scp server_ip:/home/falu/sonic-buildimage/target/sonic-vs.bin target/
    scp server_ip:/home/falu/sonic-buildimage/fs.zip ./    
    scp server_ip:/home/falu/sonic-buildimage/fs.squashfs ./    
    scp server_ip:/home/falu/sonic-buildimage/target/files/buster/onie-recovery-x86_64-kvm_x86_64-r0.iso target/files/buster/    
```
We also can use [split_transfer.sh](https://github.com/Battier/practice/blob/master/scripts/shell/split_transfer.sh) to download when the networking is unstable, but need to config for `scp` without password prompt.
```
    ssh-keygen -t rsa -b 4096
    cat ~/.ssh/id_rsa.pub | ssh server_ip 'cat >> ~/.ssh/authorized_keys'
   ./split_transfer.sh -d /home/falu/ws/sonic-buildimage/target/sonic-vs.bin -s /home/falu/sonic-buildimage/target/sonic-vs.bin -i server_ip -u falu   
```
#### 1.2.3 Generate the disk image at local
##### 1.2.3.1 Setup the build environment
Find the docker image sonic-slave-buster-<username>, and run it.
```
    falu@ubuntu:~/private/sonic/sonic-buildimage$ docker images
    REPOSITORY                                            TAG           IMAGE ID       CREATED       SIZE
    sonic-slave-buster-falu                               b83dd353c4a   b5eb2ffccbf3   3 days ago    5.2GB
    falu@ubuntu:~/private/sonic/sonic-buildimage$ docker run --rm=true --privileged --init -v /home/falu/private/sonic/sonic-buildimage:/sonic -w /sonic -it -v /var/cache/sonic/artifacts:/dpkg_cache:rw --network bridge sonic-slave-buster-falu:b83dd353c4a bash   
```
##### 1.2.3.2 Build disk image
The below commands can be found in file `target/sonic-vs.img.gz.log` on VPS server.
```
    falu@511cf4860350:/sonic$ ./onie-mk-demo.sh x86_64 vs x86_64-vs-r0 installer platform/vs/platform.conf target/sonic-vs.bin OS master.0-dirty-20210119.074008 32768 fs.zip
    falu@511cf4860350:/sonic$ sudo -E SONIC_USERNAME=admin PASSWD=YourPaSsWoRd ./scripts/build_kvm_image.sh target/sonic-vs.img target/files/buster/onie-recovery-x86_64-kvm_x86_64-r0.iso target/sonic-vs.bin 16
```

## 2. Replace package sources (Obsoleted)
### 2.1 Replace Debian mirrors
Debian is distributed (mirrored) on hundreds of servers on the Internet. Using a nearby server will probably speed up your download. [Mirrors](https://www.debian.org/mirror/list). As per my test result, here is two fastest mirrors:
```
    http://mirrors.ustc.edu.cn/debian/
    http://mirrors.163.com/debian/
```
Steps to replace Debian's mirror:
```
    $ cd sonic-buildimage/
    $ find . -type f -exec sed -i 's/debian\-archive\.trafficmanager\.net/mirrors.163.com/g' {} +
    $ find . -type f -exec sed -i 's/packages\.trafficmanager\.net/mirrors.163.com/g' {} +
    $ find . -type f -exec sed -i 's/security\.debian\.org/mirrors.163.com/g' {} +
```
### 2.2 Replace Python install packages mirrors
Unstable networking will cause intermittent **Timeout** issue, using a nearby server can decrease the failure rate of this issue, but some packages still need to load from the Python default mirror. More details please refer to [URL](https://www.cnblogs.com/yuguangtai/p/13942364.html).
```
    Set the default mirror:
        RUN pip config set global.index-url http://mirrors.aliyun.com/pypi/simple
        RUN pip config set install.trusted-host mirrors.aliyun.com
    
    Install package from the given mirror:    
        RUN pip2 install -i https://pypi.org/simple/ mockredispy==2.9.3    
```
### 2.3 Speed up clone code from Github
Useful links:
```
    https://github.com/jiji262/github-speed-up
    https://github.com/alanqian/speed-up/blob/master/github.md

```
