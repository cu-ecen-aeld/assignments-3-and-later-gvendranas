#!/bin/sh

case "$1" in
    start)
        echo "Starting aesdsocket server"
        start-stop-daemon -S --oknodo -n aesdsock -a /usr/bin/aesdsocket -- -d --verbose
        ;;
    stop)
        echo "Terminating server"
        start-stop-daemon -K --oknodo -n aesdsocket --verbose
        ;;
    test)
        echo "Testing server"
        start-stop-daemon --test -S --oknodo -n aesdsocket -a /usr/bin/aesdsocket --verbose
        ;;
    *)
        echo "Invalid Option!"
        exit 1
        ;;
esac

exit 0