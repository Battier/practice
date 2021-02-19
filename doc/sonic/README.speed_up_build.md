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
    bash -c 'echo "falu ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers'
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
## 2. Useful SONiC build system parameters
### 2.1 DPKG Cache
DPKG cache allows the .deb files to be stored in the cache path. This allows the submodules package to be cached and restored back if its commit hash is not modified and its dependencies are not modified.
More details please kindly refer to [SONiC build system](https://github.com/Azure/SONiC/blob/master/doc/sonic-build-system/DPKG%20caching%20framework%20.pptx).
```
    SONIC_DPKG_CACHE_METHOD - Default method of deb package caching
        none    :  no caching
        rwcache :  Use cache if exists else build the source and update the cache
        wcache  :  Dont use the cache and just build the source and update the cache
        rcache  :  Use cache if exists, but dont update the cache
        cache   :  Same as rwcache
    SONIC_DPKG_CACHE_SOURCE - Stores the cache location details
 e.g.:
    make configure PLATFORM=nephos
    make SONIC_DPKG_CACHE_METHOD=cache SONIC_DPKG_CACHE_SOURCE=/home/dkpg/  target/sonic-nephos.bin
```
### 2.2 Kernel
If build kernel from source, it will spend ~1 hour.
```
    DEFAULT_KERNEL_PROCURE_METHOD - default method for obtaining kernel
        build:    build kernel from source
        download: download pre-built kernel.
 
  e.g.:
    make configure PLATFORM=nephos
    make KERNEL_PROCURE_METHOD=download  target/sonic-nephos.bin
```
### 2.3 Speed up Docker image build
#### 2.3.1 Build SONiC slave docker image on VPS
sonic-slave environment is built only once, but if sonic-slave-xxxx/Dockerfile.j2 was updated, you can rebuild it with this command:
```
On VPS
    $ make sonic-slave-build
    $ docker images
        sonic-slave-stretch        e93fd61749c   d0dc62f807cb   10 hours ago   5.38GB
    $ docker save sonic-slave-stretch:e93fd61749c | gzip >  sonic-slave-stretch_e93fd61749c.gz

On local server
    $ docker load < sonic-slave-stretch_e93fd61749c.gz
```
#### 2.3.2 Use docker buildkit
After using docker buildkit for build, this options will speed up docker image build time. 
NOTE: SONIC_USE_DOCKER_BUILDKIT will produce larger installable SONiC image, because of a docker bug (more details: https://github.com/moby/moby/issues/38903)
```
    SONIC_USE_DOCKER_BUILDKIT=y
```
#### 2.3.3 Use native dockerd for build
If set to y SONiC build container will use native dockerd instead of dind for faster build
```
    SONIC_CONFIG_USE_NATIVE_DOCKERD_FOR_BUILD=y
```
### 2.4 Useful links
```
    https://github.com/Azure/SONiC/blob/master/doc/sonic-build-system
    https://github.com/Azure/sonic-buildimage/blob/master/README.buildsystem.md
```
## 3. Replace package sources (Obsoleted)
### 3.1 Replace Debian mirrors
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
### 3.2 Replace Python install packages mirrors
Unstable networking will cause intermittent **Timeout** issue, using a nearby server can decrease the failure rate of this issue, but some packages still need to load from the Python default mirror. More details please refer to [URL](https://www.cnblogs.com/yuguangtai/p/13942364.html).
```
    Set the default mirror:
        RUN pip config set global.index-url http://mirrors.aliyun.com/pypi/simple
        RUN pip config set install.trusted-host mirrors.aliyun.com
    
    Install package from the given mirror:    
        RUN pip2 install -i https://pypi.org/simple/ mockredispy==2.9.3    
```
### 3.3 Speed up clone code from Github
Useful links:
```
    https://github.com/jiji262/github-speed-up
    https://github.com/alanqian/speed-up/blob/master/github.md

```
## 4. Improve SSD performance
```
    https://www.howtogeek.com/62761/how-to-tweak-your-ssd-in-ubuntu-for-better-performance/
    https://blog.csdn.net/github_33934628/article/details/51804747
```
