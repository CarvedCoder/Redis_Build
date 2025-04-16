#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>

static void msg(const char *msg) {
    fprintf(stderr, "%s\n", msg);
}

static void die(const char *msg) {
    int err = errno;
    fprintf(stderr, "[%d]%s\n", err, msg);
}

static void do_something(int connfd) {
    char rbuf[64] = {};
    ssize_t n = read(connfd, rbuf, sizeof(rbuf) - 1);
    if (n < 0) {
        msg("read() error");
        return;
    }
    printf("client says: %s\n", rbuf);

    char wbuf[] = "world";
    write(connfd, wbuf, strlen(wbuf));
}

int main() {
    // Create fd using socket() contains IPV4 and TCP connections

    int fd = socket(AF_INET,SOCK_STREAM, 0);

    // SO_REUSEADDR = val -> to bind to the same IP port after a restart (when it's 1)

    int val = 1;
    setsockopt(fd,SOL_SOCKET,SO_REUSEADDR, &val, sizeof(val));

    // address for binding

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234); // port
    addr.sin_addr.s_addr = htonl(0); // wildcard ip 0.0.0.0

    // binding fd with the port address

    int rv = bind(fd, (const struct sockaddr *) &addr, sizeof(addr));
    if (rv) {
        die("bind()");
    }

    // Listening to the client i.e making connections , SOMAXCONN is the size of queue (4096 on linux)

    rv = listen(fd,SOMAXCONN);
    if (rv) {
        die("listen()");
    }

    // Accepting all the client requests

    while (true) {
        struct sockaddr_in client_addr = {};
        socklen_t addrlen = sizeof(client_addr);
        int connfd = accept(fd, (struct sockaddr *) &client_addr, &addrlen);

        if (connfd < 0) {
            continue;
        }
        do_something(connfd);
        close(connfd);
    }
}
