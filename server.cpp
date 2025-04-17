#include <cstdint>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>

    // read all the inflight data
static int32_t read_full(int fd,char*buf,size_t n) {
    while (n>0) {
        ssize_t rv = read(fd,buf,n);
        if (rv <= 0) {
            return -1;
        }
        assert((size_t)rv <= n);
        n -= (size_t) rv;
        buf += rv;
    }
    return 0;
}

    // Write all the data
static int32_t write_all(int fd,const char *buf,size_t n) {
    while (n>0) {
        ssize_t rv = write(fd,buf,n);
        if (rv <= 0) {
            return -1;
        }
        assert((size_t)rv <= n);
        n-=(size_t)rv;
        buf+=rv;
    }
    return 0;
}

static void msg(const char *msg) {
    fprintf(stderr, "%s\n", msg);
}

static void die(const char *msg) {
    int err = errno;
    fprintf(stderr, "[%d]%s\n", err, msg);
}

const size_t K_MAX_MSG = 4096;

static int32_t one_request(int connfd) {
    // 4 -> size of int i.e no.of requests
    char rbuf[4 + K_MAX_MSG] = {};
    errno = 0;
    int32_t err = read_full(connfd,rbuf,4);
    if (err) {
        msg(errno == 0? "EOF" : "read() error");
        return err;
    }
    uint32_t len = 0;
    memcpy(&len,rbuf,4);
    if (len > K_MAX_MSG) {
        msg("Too long");
        return -1;
    }
    err = read_full(connfd,&rbuf[4],len);
    if (err) {
        msg("read() error");
        return err;
    }
    fprintf(stderr,"client says : %.*s\n",len,&rbuf[4]);
    // reply world to every single request
    const char reply[] = "world";
    char wbuf[4+sizeof(reply)];
    len = (uint32_t)strlen(reply);
    memcpy(wbuf,&len,4);
    memcpy(&wbuf[4],reply,len);
    return write_all(connfd,wbuf,4+len);
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
        // accept multiple requests from a single client
        while (true){
            int32_t err = one_request(connfd);
            if (err) {
                break;
            }
        }
        close(connfd);
    }
    return 0;
}
