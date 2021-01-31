
1, Remove all docker images
'''
sudo docker kill $(sudo docker ps -q);sudo docker rm $(sudo docker ps -a -q);sudo docker rmi $(sudo docker images -q)
'''
2, Config the default bridge (docker0) network
You can configure the default bridge network by providing the bip option along with the desired subnet in the daemon.json (default location at /etc/docker/daemon.json on Linux) file as follows:
'''
{
  "bip": "172.26.0.1/16"
}
'''
Then restart the docker daemon:
'''
sudo systemctl restart docker
'''
3, Save an image to a tar.gz file using gzip
'''
docker save myimage:latest | gzip > myimage_latest.tar.gz
'''

