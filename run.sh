#!/bin/bash

run_http()
{
	ps -a | grep http_server | awk '{ print $1 }'
	./http_server 8082 . 1 & export HTTP_PID=$!
	echo "Started HTTP Server with pid $HTTP_PID"
}

run_websocket()
{
	ps -a | grep websocket_srv | awk '{ print $1 }'
	./websocket_srv 8083 1 & export SOCK_PID=$!
	echo "Started Websocket Server with pid $SOCK_PID"
}

exit_handler()
{
	./stop.sh
	exit
}

trap exit_handler SIGINT SIGTERM

run_http
run_websocket

while true
do
	sleep 1
done
