from ubuntu:22.04@sha256:ed1544e454989078f5dec1bfdabd8c5cc9c48e0705d07b678ab6ae3fb61952d2
ENV DEBIAN_FRONTEND noninteractive

RUN apt-get -y update --fix-missing && \
    apt-get -y install xinetd
RUN groupadd -r pwn && useradd -r -g pwn pwn

RUN echo '#!/bin/bash\n\
service xinetd restart && /bin/sleep infinity' > /etc/init.sh
RUN echo 'service pwn\n\
{\n\
  type = UNLISTED\n\
  disable = no\n\
  socket_type = stream\n\
  protocol = tcp\n\
  wait = no\n\
  user = pwn\n\
  bind = 0.0.0.0\n\
  port = 9999\n\
  server = /home/pwn/db\n\
}' > /etc/xinetd.d/pwn
RUN chmod 500 /etc/init.sh
RUN chmod 444 /etc/xinetd.d/pwn
RUN chmod 1733 /tmp /var/tmp /dev/shm

RUN echo "flag{3d63_c4535_c4u53_7r0ubl35}" > /flag

WORKDIR /home/pwn
ADD --chmod=550 db .

RUN chown -R root:pwn /home/pwn
RUN service xinetd restart

CMD ["/bin/bash"]