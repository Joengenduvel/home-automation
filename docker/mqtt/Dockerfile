FROM hypriot/rpi-alpine
RUN apk add --no-cache mosquitto

ADD mosquitto.conf /etc/mosquitto/

EXPOSE 1883 9001

ENTRYPOINT ["mosquitto"]
