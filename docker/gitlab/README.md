# GitLab 

Setup a simple docker-based GitLab

## Commands

Set external_url and gitlab_shell_ssh_port:
    ```
    external_url "https://gitlab.com:8929"
    gitlab_rails['gitlab_shell_ssh_port'] = 2289
    gitlab_rails['gitlab_email_enabled'] = true    
    ```
Build with docker:

    $ docker build -t gitlab:latest .

Run this command to start Docker:
    
    $ export GITLAB_HOME=/ddd/falu/gitlab
    $ docker run --detach --cpus=8 --memory=16g --oom-kill-disable --hostname gitlab.com   --publish 8929:8929 --publish 2289:22   --name gitlab   --restart always   --volume $GITLAB_HOME/config:/etc/gitlab   --volume $GITLAB_HOME/logs:/var/log/gitlab   --volume $GITLAB_HOME/data:/var/opt/gitlab   gitlab/gitlab-ee:latest

Check Docker log:
    
    $ docker logs gitlab

Change the gitlab configure:
    
    $ docker exec -it gitlab /bin/bash
    root@gitlab:/# vi /etc/gitlab/gitlab.rb
    root@gitlab:/# gitlab-ctl reconfigure
    root@gitlab:/# exit
