#pragma once

#include <cstring>
#include <cstdio>
#include <cerrno>
#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/eventfd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#include <string>

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "conf/Conf.h"


#define MAX_LINE 4096

typedef struct sockaddr SA;
typedef struct epoll_event SE;

int Socket(int family,int type,int protocol);
int Bind(int sockfd,SA *myaddr,socklen_t addrlen);
int Listen(int sockfd,int backlog);
int Accept(int sockfd,SA *cliaddr,socklen_t *addrlen);
int Fcntl(int fd,int cmd);
int Fcntl(int fd,int cmd,long arg);
int SetNonBlock(int sockfd);
int EpollCreate(int size);
int EpollCtl(int epfd, int op, int fd, SE *event);
int EpollWait(int epfd, SE *events, int maxevents, int timeout);
ssize_t Readn(int fd, std::string &inbuffer, bool &zero);
ssize_t writen(int fd,const void *vptr,size_t n);
int Open(const char *pathname,int oflags,mode_t mode);
int Close(int sockfd);
int SetSockOpt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
int TcpListen(const char *hostname, const char *service, socklen_t *addrlenp);
int Eventfd(unsigned int initval,int flags);
ssize_t SslReadn(SSL* ssl, std::string &inbuffer, bool &zero);
ssize_t SslWriten(SSL* ssl, const void *vptr, size_t n);
