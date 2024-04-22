#include <syslog.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/stat.h>

// Constants
#define MAX_BUFFER 4096 // No magic numbers in the code

// Global variables
static bool signal_recived = false;

static bool read_packet(int sock, int fd)
{
  char buffer[MAX_BUFFER];
  bool end_of_rx = false;
  ssize_t num_rx_bytes = 0;
  char *end_of_line = NULL;
  off_t start_offset = lseek(fd, 0, SEEK_END);

  while (!end_of_rx && !signal_recived) {

    // Revicing data
    num_rx_bytes = recv(sock, buffer, sizeof(buffer), 0);

    // Checking data reception result
    if (num_rx_bytes == -1) {
      syslog(LOG_ERR, "Error in recv!: %s", strerror(errno));
      return false;
    } else if (num_rx_bytes == 0) {
      // End of the reception
      syslog(LOG_ERR, "Receiving data");
      return false;
    }

    // Looking for EOL in the received buffer
    end_of_line = memchr(buffer, '\n', num_rx_bytes);
    if (end_of_line) {
      // truncate, keeping new line
      num_rx_bytes =  end_of_line - buffer + 1;
      end_of_rx = true;
    }

    // Write data in the file descriptor as requested in the exercice
    if (write(fd, buffer, num_rx_bytes) == -1) {
      ftruncate(fd, start_offset);
      return false;
    }
  }

  if (signal_recived) {
    return false;
  }

  return true;
}

static void send_response(int sock, int fd)
{
    ssize_t numbytes = 0;
    char buffer[MAX_BUFFER];

    // Setting file descriptor in 0
    if ((lseek(fd, 0, SEEK_SET)) == -1) 
    {
        return;
    }

    while (!signal_recived) 
    {
        numbytes = read(fd, buffer, sizeof(buffer));
        if (numbytes == 0) 
        {
            // No data read
            return;
        } else if (numbytes == -1) 
        {
            // Some issue reading file descriptor
            syslog(LOG_ERR, "Could not read!: %s", strerror(errno));
            return;
        }
        // Writing data
        if (send(sock, buffer, numbytes, 0) == -1) 
        {
            // Some issue sending data
            syslog(LOG_ERR, "Could not write!: %s", strerror(errno));
            return;
        }
    }
}

static void signal_handler(int x)
{
  signal_recived = true;
}

int main(int argc, char **argv)
{
    int i = 0;
    int sock = -1;
    int rc = 0;
    int child = -1;
    int retval = -1;
    socklen_t salen;
    int file_desc = -1;
    char ipaddr[40];
    struct sigaction sigact;
    int val=1;    
    struct sockaddr sa;
    bool it_is_a_daemon = false;
    struct addrinfo *ai = NULL;
    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    openlog(NULL, 0, LOG_USER);

    // Check in in puts arguments whether it has -d argument meanning it is a daemon
    for (i=1; i < argc; ++i) {
        if (!strcmp(argv[i], "-d")) {
            it_is_a_daemon = true;
        }
    }

    // Getting a socket
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        syslog(LOG_ERR, "Error opening file!: %s", strerror(errno));
        goto something_failed;
    }

    // Configure socket options
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) == -1) {
        syslog(LOG_ERR, "Error in setsockopt!: %s", strerror(errno));
        goto something_failed;
    }

    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if ((rc = getaddrinfo(NULL, "9000", &hints, &ai)) != 0) {
        goto something_failed;
    }
    if (!ai) {
        goto something_failed;
    }
    if (bind(sock, ai->ai_addr, ai->ai_addrlen) == -1) {
        goto something_failed;
    }

    // Let0s do what a Daemon need to be so
    if (it_is_a_daemon) {
        pid_t childpid = fork();
        if (childpid != 0) {
        // quit parent
        _exit(0);
        }
        setsid();
        chdir("/");
        if ((freopen("/dev/null", "r", stdin) == NULL) || (freopen("/dev/null", "w", stdout) == NULL) || (freopen("/dev/null", "r", stderr) == NULL)) {
            goto something_failed;
        }
    }

    // Opening the file descriptor
    file_desc = open("/var/tmp/aesdsocketdata", O_CREAT | O_TRUNC | O_RDWR, 0644);
    if (file_desc == -1) {
        goto something_failed;
    }

    // Listening the file descriptor
    if (listen(sock, 10) == -1) {
        goto something_failed;
    }

    // Fill sigact with zeros
    memset(&sigact, 0, sizeof(sigact));
    sigact.sa_handler = signal_handler;
    sigaction(SIGTERM, &sigact, NULL);
    sigaction(SIGINT, &sigact, NULL);
    

    while(!signal_recived) {
        salen = sizeof(sa);
        if ((child = accept(sock, &sa, &salen)) == -1) {
        continue;
        }

        if (inet_ntop(AF_INET, &((struct sockaddr_in *)&sa)->sin_addr, ipaddr, sizeof(ipaddr)) == NULL) {
            strncpy(ipaddr, "???", sizeof(ipaddr));
        }

        if (!read_packet(child, file_desc)) {
        } else {
            send_response(child, file_desc);
        }

        close(child);
        child = -1;
        // Per specs
        syslog(LOG_DEBUG, "Closed connection from %s", ipaddr);
    }

    if (signal_recived) {
        syslog(LOG_ERR, "Signal received");
    }

    if (child != -1) {
        close(child);
        child = -1;
        // Per specs
        syslog(LOG_DEBUG, "Closed connection from %s", ipaddr);
    }

  retval = 0;

something_failed:
    if (ai) {
        freeaddrinfo(ai);
    }
    if (file_desc != -1) {
        close(file_desc);
    }
    if (sock != -1) {
        close(sock); 
    }
    if (child != -1) {
        close (child);
    }    
    closelog();
    return retval;
}