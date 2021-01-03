#!/bin/bash
USERNAME='default'
PASSWORD='123'
_V=0

while getopts "vp:u:" OPTION
do
    case $OPTION in
        v)
	    _V=1
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
SUDO "apt -y install vim vim-gtk3 gitk python2 python3 net-tools make gcc"
SUDO update-alternatives --install /usr/bin/python python /usr/bin/python3 1
SUDO add-apt-repository universe
SUDO apt-get update
SUDO apt -y install python2

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

echo "Done"
