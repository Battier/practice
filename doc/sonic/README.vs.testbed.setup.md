# VS KVM Testbed Setup

## 1. Host information 
```
    OS: Ubuntu 20.04 VM, Enabled hypervisor application
    CPU: 4 CPU cores
    Memory: 8GB memory
    Disk: 170GB
```
## 2. Download cEOS image
1. Create folder for EOS image
```
    $ mkdir -p ~/veos-vm/images/
```
2. Download [cEOS image from Arista](https://www.arista.com/en/support/software-download)
3. Import the cEOS image, please use **ceosimage:4.23.2F** as **[REPOSITORY[:TAG]]** when import docker image.
```
    $ mkdir -p ~/veos-vm/images/
    $ docker import cEOS64-lab-4.25.1F.tar.xz ceosimage:4.23.2F
    $ docker images
    REPOSITORY                                     TAG                 IMAGE ID            CREATED             SIZE
    ceosimage                                      4.23.2F             d53c28e38448        2 hours ago         1.82GB
```
## 3. Install tools
```
    $ sudo apt update
    $ sudo apt install cpu-checker qemu-kvm libvirt-daemon-system libvirt-clients bridge-utils virtinst virt-manager openssh-client openssh-server -y
    $ sudo systemctl is-active libvirtd
    $ sudo usermod -aG libvirt $USER
    $ sudo usermod -aG kvm $USER
```
## 4. Download [docker-sonic-mgmt image](https://sonic-jenkins.westus2.cloudapp.azure.com/job/bldenv/job/docker-sonic-mgmt/lastSuccessfulBuild/artifact/sonic-buildimage/target/docker-sonic-mgmt.gz) and [sonic-vs image](https://sonic-jenkins.westus2.cloudapp.azure.com/job/vs/job/buildimage-vs-image/lastSuccessfulBuild/artifact/target/sonic-vs.img.gz) 
```
    $ mkdir -p ~/sonic-vm/images
    $ cd ~/sonic-vm/images
    $ wget https://sonic-jenkins.westus2.cloudapp.azure.com/job/bldenv/job/docker-sonic-mgmt/lastSuccessfulBuild/artifact/sonic-buildimage/target/docker-sonic-mgmt.gz
    $ wget https://sonic-jenkins.westus2.cloudapp.azure.com/job/vs/job/buildimage-vs-image/lastSuccessfulBuild/artifact/target/sonic-vs.img.gz
```
## 5. Unzip `sonic-vs.img.gz` image
```
    $ cd ~/sonic-vm/images
    $ gunzip sonic-vs.img.gz
```
## 6. Load `docker-sonic-mgmt.gz` image
```
    $ cd ~/sonic-vm/images
    $ docker load < docker-sonic-mgmt.gz
```
## 7. Clone [sonic-mgmt](https://github.com/Azure/sonic-mgmt) and initialize the management bridge network

```
    $ cd ~
    $ git clone https://github.com/Azure/sonic-mgmt
    $ cd sonic-mgmt/ansible
    $ sudo ./setup-management-network.sh
```
## 8. Setup `sonic-mgmt` docker
All testbed configuration steps and tests are run from a `sonic-mgmt` docker container.
1. Skip downloading the image when the cEOS image is not imported locally, set `skip_ceos_image_downloading` to `true` in `sonic-mgmt/ansible/group_vars/all/ceos.yml`.

```
    diff --git a/ansible/group_vars/all/ceos.yml b/ansible/group_vars/all/ceos.yml
    index efa305a5..defb0c61 100644
    --- a/ansible/group_vars/all/ceos.yml
    +++ b/ansible/group_vars/all/ceos.yml
    @@ -1,4 +1,4 @@
     ceos_image_filename: cEOS64-lab-4.23.2F.tar.xz
     ceos_image_orig: ceosimage:4.23.2F
     ceos_image: ceosimage:4.23.2F-1
    -skip_ceos_image_downloading: false
    +skip_ceos_image_downloading: true
```
2. Run the `setup-container.sh` in the root directory of the sonic-mgmt repository:
```
    $ cd ~/sonic-mgmt
    $ mkdir ~/data
    $ ./setup-container.sh -n <container name> -d ~/data
```
3. From now on, **all steps are running inside the sonic-mgmt docker**, unless otherwise specified.
You can enter your sonic-mgmt container with the following command:
```
    $ docker exec -it <container name> bash
```
You will find your `sonic-mgmt` directory mounted at `~/data/sonic-mgmt`:
```
    $ ls ~/data/sonic-mgmt/
    LICENSE  README.md  __pycache__  ansible  docs	lgtm.yml  setup-container.sh  spytest  test_reporting  tests
```
## 9. Setup host public key in sonic-mgmt docker
In order to configure the testbed on your host automatically, Ansible needs to be able to SSH into it without a password prompt. The `setup-container` script from the previous step will setup all the necessary SSH keys for you, but there are a few more modifications needed to make Ansible work:
1. Modify `veos_vtb` to use the user name (e.g. `foo`) you want to use to login to the host machine (this can be your username on the host)
```
    falu@sonic:~/data/sonic-mgmt/ansible$ git diff
    index be33f9f3..f3b37d41 100644
    --- a/ansible/veos_vtb
    +++ b/ansible/veos_vtb
    @@ -128,7 +128,7 @@ vm_host_1:
       hosts:
         STR-ACS-VSERV-01:
           ansible_host: 172.17.0.1
    -      ansible_user: use_own_value
    +      ansible_user: falu
     
     vms_1:
       hosts:
```
2.  Create a dummy `password.txt` file under `~/data/sonic-mgmt/ansible`
    - **Note**: Here, `password.txt` is the Ansible Vault password file. Ansible allows users to use Ansible Vault to encrypt password files.
      By default, the testbed scripts require a password file. If you are not using Ansible Vault, you can create a file with a dummy password (e.g. `abc`) and pass the filename to the command line. The file name and location is created and maintained by the user.
3. **On the host,** run `sudo visudo` and add the following line at the end:
```
    falu ALL=(ALL) NOPASSWD:ALL
```
4. Verify that you can login into the host (e.g. `ssh falu@172.17.0.1`) from the `sonic-mgmt` container without any password prompt.
5. Verify that you can use `sudo` without a password prompt inside the host (e.g. `sudo bash`).
## 11. Deploy T0 topology
Now we're finally ready to deploy the topology for our testbed! Run the following command, depending on what type of EOS image you are using for your setup:
```
    $ cd ~/data/sonic-mgmt/ansible
    $ ./testbed-cli.sh -t vtestbed.csv -m veos_vtb -k ceos add-topo vms-kvm-t0 password.txt
```
Verify that the cEOS neighbors were created properly:
```
    $ docker ps
    CONTAINER ID        IMAGE                                                 COMMAND                  CREATED              STATUS              PORTS               NAMES
    575064498cbc        ceosimage:4.23.2F                                     "/sbin/init systemd.…"   About a minute ago   Up About a minute                       ceos_vms6-1_VM0103
    d71b8970bcbb        debian:jessie                                         "bash"                   About a minute ago   Up About a minute                       net_vms6-1_VM0103
    3d2e5ecdd472        ceosimage:4.23.2F                                     "/sbin/init systemd.…"   About a minute ago   Up About a minute                       ceos_vms6-1_VM0102
    28d64c74fa54        debian:jessie                                         "bash"                   About a minute ago   Up About a minute                       net_vms6-1_VM0102
    0fa067a47c7f        ceosimage:4.23.2F                                     "/sbin/init systemd.…"   About a minute ago   Up About a minute                       ceos_vms6-1_VM0101
    47066451fa4c        debian:jessie                                         "bash"                   About a minute ago   Up About a minute                       net_vms6-1_VM0101
    e07bd0245bd9        ceosimage:4.23.2F                                     "/sbin/init systemd.…"   About a minute ago   Up About a minute                       ceos_vms6-1_VM0100
    4584820bf368        debian:jessie                                         "bash"                   7 minutes ago        Up 7 minutes                            net_vms6-1_VM0100
    c929c622232a        sonicdev-microsoft.azurecr.io:443/docker-ptf:latest   "/usr/local/bin/supe…"   7 minutes ago        Up 7 minutes                            ptf_vms6-1
```
## 12. Deploy minigraph on the DUT
Once the topology has been created, we need to give the DUT an initial configuration.
1. Deploy the `minigraph.xml` to the DUT and save the configuration:

```
    $ ./testbed-cli.sh -t vtestbed.csv -m veos_vtb deploy-mg vms-kvm-t0 lab password.txt
```
2. Check VS KVM status
```
    $ virsh list
     Id   Name      State
    -------------------------
     1    vlab-01   running
```
3. Login to the VS KVM from host via console \(Username/Password: admin/password\)
```
    $ telnet 127.0.0.1 9000
```
4. Login to the SONiC KVM using Mgmt IP = `10.250.0.101` and `admin:password`.
```
    $ ssh admin@10.250.0.101
```
5. Check BGP sessions in SONiC:

```
    admin@vlab-01:~$ show ip bgp sum
    BGP router identifier 10.1.0.32, local AS number 65100
    RIB entries 12807, using 1401 KiB of memory
    Peers 8, using 36 KiB of memory
    Peer groups 2, using 112 bytes of memory
    
    Neighbor        V         AS MsgRcvd MsgSent   TblVer  InQ OutQ Up/Down  State/PfxRcd
    10.0.0.57       4 64600    3208      12        0    0    0 00:00:22     1
    10.0.0.59       4 64600    3208     593        0    0    0 00:00:22     1
    10.0.0.61       4 64600    3205     950        0    0    0 00:00:21     1
    10.0.0.63       4 64600    3204     950        0    0    0 00:00:21     1
```
## 13. Run a Pytest
Now that the testbed has been fully setup and configured, let's run a simple test to make sure everything is functioning as expected.
Switch over to the `tests` directory:
```
cd sonic-mgmt/tests
```
You're now set up and ready to use the KVM testbed!
