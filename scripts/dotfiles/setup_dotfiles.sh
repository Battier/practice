#!/bin/bash
USERNAME='default'
PASSWORD='123'
_V=0
setup_vim=0

while getopts "Vvp:u:" OPTION
do
    case $OPTION in
        v)
            _V=1
            ;;

        p)
            PASSWORD=$OPTARG
            ;;

        u)
            USERNAME=$OPTARG
            ;;

        V)
            setup_vim=1
            ;;

        \? )
            echo "Invalid Option: -$OPTARG" 1>&2
            exit 1
      ;;
    esac
done
shift $((OPTIND -1))

log ()
{
    if [ $_V -eq 1 ]; then
        echo "$@"
    fi
}


SUDO ()
{
    log $@
    echo $PASSWORD | sudo -S $@
}

SUDO "apt update"
SUDO "apt upgrade"
SUDO "apt -y install vim vim-gtk3 gitk python2 python3 net-tools make gcc curl cpu-checker qemu-kvm libvirt-daemon-system libvirt-clients bridge-utils virtinst virt-manager openssh-client openssh-server"
SUDO update-alternatives --install /usr/bin/python python /usr/bin/python3 1
SUDO add-apt-repository universe
SUDO apt-get update
SUDO apt -y install python2
SUDO systemctl is-active libvirtd
SUDO usermod -aG libvirt $USER
SUDO usermod -aG kvm $USER


curl https://bootstrap.pypa.io/get-pip.py --output ~/get-pip.py
SUDO python2 ~/get-pip.py
SUDO rm -rf ~/get-pip.py

SUDO python2 -m pip install -U pip==9.0.3
SUDO pip install --force-reinstall --upgrade jinja2>=2.10
SUDO pip install j2cli


SUDO apt-get install \
    apt-transport-https \
    ca-certificates \
    curl \
    gnupg-agent \
    software-properties-common

curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo apt-key add -
apt-key fingerprint 0EBFCD88

sudo add-apt-repository "deb [arch=amd64] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable"

SUDO apt-get update
SUDO apt-get -y install docker-ce docker-ce-cli containerd.io

SUDO apt-get install -y openvswitch-switch
SUDO curl -L https://github.com/openvswitch/ovs/raw/master/utilities/ovs-docker -o /usr/bin/ovs-docker
SUDO chmod a+x /usr/bin/ovs-docker
SUDO groupadd docker
SUDO usermod -aG docker $USER
newgrp docker
SUDO chown "$USER":"$USER" /home/"$USER"/.docker -R
SUDO chmod g+rwx "$HOME/.docker" -R
SUDO systemctl enable docker

if [ $setup_vim -eq 1 ]; then
    echo "Setup VIM Config"
    git clone --depth=1 https://github.com/Battier/vimrc.git ~/.vim_runtime
    cp ./vim/plugin/my_configs.vim ~/.vim_runtime/my_configs.vim
    cd ~/.vim_runtime
    python update_plugins.py
    sh ~/.vim_runtime/install_awesome_vimrc.sh
fi
echo "Done"
