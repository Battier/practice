FROM debian:buster
RUN apt-get update -q \
  && apt install -y -qq apt-mirror \
  && apt-get autoremove \
  && rm -rf /var/cache/apt/* \
  && mkdir -p /mnt/mirror/debian \
  && touch /var/log/cron.log
COPY files/entrypoint.sh /opt/entrypoint.sh
COPY files/mirror.list /etc/apt/mirror.list
RUN chmod u+x /opt/entrypoint.sh
VOLUME ["/mnt/mirror/debian"]
ENTRYPOINT ["/opt/entrypoint.sh"]
