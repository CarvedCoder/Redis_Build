#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>

static void die(const char * msg) {
    int err = errno;
    fprintf(stderr,"[%d]%s",err,msg);
}
int main() {
    int fd = socket(AF_INET,SOCK_STREAM,0);
    if(fd < 0) {
        die("socket()");
    }

    struct sockaddr_in addr ={};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);

    int rv = connect(fd,(const struct sockaddr*)&addr,sizeof(addr));

    if (rv) {
        die("connect");
    }
    char msg [] = "hello";
    write(fd,msg,strlen(msg));

    char rbuf[64] = {};
    ssize_t n = read(fd,rbuf,sizeof(rbuf)-1);

    if (n<0) {
        die("read");
    }

    printf("server says: %s \n",rbuf);
    close(fd);

}