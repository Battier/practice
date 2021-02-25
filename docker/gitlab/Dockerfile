FROM gitlab/gitlab-ee

COPY files/gitlab.rb /etc/gitlab/gitlab.rb

RUN apt-get update -q \
  && apt install -y sendmail \
  && sed -i '/echo "Starting services..."/a service sendmail start' /assets/wrapper
