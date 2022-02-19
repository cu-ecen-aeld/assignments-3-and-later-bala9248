#!/bin/sh
#Busybox init script for starting and stopping aesdsocket in Daemon mode

case "$1" in 
	start)
	  echo "starting aesdsocket"
	  start-stop-daemon -S -n aesdsocket -a /usr/bin/aesdsocket -- -d
	  ;;
	  
	stop)
	  echo "Stopping aesdsocket"
	  start-stop-daemon -K -n aesdsocket
	  ;;
	*)
	  echo "Usage: $0 {start|stop}"
	exit 1
esac

exit 0

