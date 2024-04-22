#! /bin/sh

case "$1" in
    start)
        echo "Starting aesdsocket"
        /usr/bin/aesdsocket -d
        echo "Done"
        ;;
    stop)
        echo "Stopping aesdsocket"
        killall aesdsocket
        ;;
    *)
        echo "Usage: $0 {start|stop}"
    exit 1
esac

exit 0