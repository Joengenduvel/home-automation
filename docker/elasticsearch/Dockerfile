#FROM hypriot/rpi-alpine
FROM alpine
RUN echo "vm.max_map_count=262144" >> /etc/sysctl.d/00-alpine.conf
RUN apk add --no-cache openrc elasticsearch
RUN touch /run/openrc/softlevel

Entrypoint ["rc-service","elasticsearch","start"]