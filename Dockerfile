FROM debian:latest

COPY ./src/mini_serv.c /mini_serv.c

RUN apt-get update && apt-get install -y gcc valgrind

ENTRYPOINT [ "tail", "-f", "/dev/null" ]