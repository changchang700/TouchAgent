#ifndef HV_SOCKET_H_
#define HV_SOCKET_H_

#include "hexport.h"
#include "hplatform.h"

#define ENABLE_UDS

#ifdef ENABLE_UDS
    #include <sys/un.h> // import struct sockaddr_un
#endif

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif

#define LOCALHOST   "127.0.0.1"
#define ANYADDR     "0.0.0.0"

BEGIN_EXTERN_C

static inline int socket_errno() {
#ifdef OS_WIN
    return WSAGetLastError();
#else
    return errno;
#endif
}
HV_EXPORT const char* socket_strerror(int err);

#ifdef OS_WIN
typedef int socklen_t;
static inline int blocking(int sockfd) {
    unsigned long nb = 0;
    return ioctlsocket(sockfd, FIONBIO, &nb);
}
static inline int nonblocking(int sockfd) {
    unsigned long nb = 1;
    return ioctlsocket(sockfd, FIONBIO, &nb);
}
#define poll        WSAPoll
#undef  EAGAIN
#define EAGAIN      WSAEWOULDBLOCK
#undef  EINPROGRESS
#define EINPROGRESS WSAEINPROGRESS
#undef  ENOTSOCK
#define ENOTSOCK    WSAENOTSOCK
#else
#define blocking(s)     fcntl(s, F_SETFL, fcntl(s, F_GETFL) & ~O_NONBLOCK)
#define nonblocking(s)  fcntl(s, F_SETFL, fcntl(s, F_GETFL) |  O_NONBLOCK)
typedef int         SOCKET;
#define INVALID_SOCKET  -1
#define closesocket close
#endif

//-----------------------------sockaddr_u----------------------------------------------
typedef union {
    struct sockaddr     sa;
    struct sockaddr_in  sin;
    struct sockaddr_in6 sin6;
#ifdef ENABLE_UDS
    struct sockaddr_un  sun;
#endif
} sockaddr_u;

// @param host: domain or ip
// @retval 0:succeed
HV_EXPORT int Resolver(const char* host, sockaddr_u* addr);

static inline const char* sockaddr_ip(sockaddr_u* addr, char *ip, int len) {
    if (addr->sa.sa_family == AF_INET) {
        return inet_ntop(AF_INET, &addr->sin.sin_addr, ip, len);
    }
    else if (addr->sa.sa_family == AF_INET6) {
        return inet_ntop(AF_INET6, &addr->sin6.sin6_addr, ip, len);
    }
    return ip;
}

static inline uint16_t sockaddr_port(sockaddr_u* addr) {
    uint16_t port = 0;
    if (addr->sa.sa_family == AF_INET) {
        port = htons(addr->sin.sin_port);
    }
    else if (addr->sa.sa_family == AF_INET6) {
        port = htons(addr->sin6.sin6_port);
    }
    return port;
}

static inline int sockaddr_set_ip(sockaddr_u* addr, const char* host) {
    if (!host || *host == '\0') {
        addr->sin.sin_family = AF_INET;
        addr->sin.sin_addr.s_addr = htonl(INADDR_ANY);
        return 0;
    }
    return Resolver(host, addr);
}

static inline void sockaddr_set_port(sockaddr_u* addr, int port) {
    if (addr->sa.sa_family == AF_INET) {
        addr->sin.sin_port = ntohs(port);
    }
    else if (addr->sa.sa_family == AF_INET6) {
        addr->sin6.sin6_port = ntohs(port);
    }
}

static inline int sockaddr_set_ipport(sockaddr_u* addr, const char* host, int port) {
    int ret = sockaddr_set_ip(addr, host);
    if (ret != 0) return ret;
    sockaddr_set_port(addr, port);
    return 0;
}

//#define INET_ADDRSTRLEN   16
//#define INET6_ADDRSTRLEN  46
#ifdef ENABLE_UDS
#define SOCKADDR_STRLEN     sizeof(((struct sockaddr_un*)(NULL))->sun_path)
static inline void sockaddr_set_path(sockaddr_u* addr, const char* path) {
    addr->sa.sa_family = AF_UNIX;
    strncpy(addr->sun.sun_path, path, sizeof(addr->sun.sun_path));
}
#else
#define SOCKADDR_STRLEN     64 // ipv4:port | [ipv6]:port
#endif

static inline socklen_t sockaddr_len(sockaddr_u* addr) {
    if (addr->sa.sa_family == AF_INET) {
        return sizeof(struct sockaddr_in);
    }
    else if (addr->sa.sa_family == AF_INET6) {
        return sizeof(struct sockaddr_in6);
    }
#ifdef ENABLE_UDS
    else if (addr->sa.sa_family == AF_UNIX) {
        return sizeof(struct sockaddr_un);
    }
#endif
    return sizeof(sockaddr_u);
}

static inline const char* sockaddr_str(sockaddr_u* addr, char* buf, int len) {
    char ip[SOCKADDR_STRLEN] = {0};
    uint16_t port = 0;
    if (addr->sa.sa_family == AF_INET) {
        inet_ntop(AF_INET, &addr->sin.sin_addr, ip, len);
        port = htons(addr->sin.sin_port);
        snprintf(buf, len, "%s:%d", ip, port);
    }
    else if (addr->sa.sa_family == AF_INET6) {
        inet_ntop(AF_INET6, &addr->sin6.sin6_addr, ip, len);
        port = htons(addr->sin6.sin6_port);
        snprintf(buf, len, "[%s]:%d", ip, port);
    }
#ifdef ENABLE_UDS
    else if (addr->sa.sa_family == AF_UNIX) {
        snprintf(buf, len, "%s", addr->sun.sun_path);
    }
#endif
    return buf;
}

static inline void sockaddr_print(sockaddr_u* addr) {
    char buf[SOCKADDR_STRLEN] = {0};
    sockaddr_str(addr, buf, sizeof(buf));
    puts(buf);
}

#define SOCKADDR_LEN(addr)      sockaddr_len((sockaddr_u*)addr)
#define SOCKADDR_STR(addr, buf) sockaddr_str((sockaddr_u*)addr, buf, sizeof(buf))
#define SOCKADDR_PRINT(addr)    sockaddr_print((sockaddr_u*)addr)
//=====================================================================================

// socket -> setsockopt -> bind
// @param type: SOCK_STREAM(tcp) SOCK_DGRAM(udp)
// @return sockfd
HV_EXPORT int Bind(int port, const char* host DEFAULT(ANYADDR), int type DEFAULT(SOCK_STREAM));

// Bind -> listen
// @return listenfd
HV_EXPORT int Listen(int port, const char* host DEFAULT(ANYADDR));

// @return connfd
// Resolver -> socket -> nonblocking -> connect
HV_EXPORT int Connect(const char* host, int port, int nonblock DEFAULT(0));
// Connect(host, port, 1)
HV_EXPORT int ConnectNonblock(const char* host, int port);
// Connect(host, port, 1) -> select -> blocking
#define DEFAULT_CONNECT_TIMEOUT 5000 // ms
HV_EXPORT int ConnectTimeout(const char* host, int port, int ms DEFAULT(DEFAULT_CONNECT_TIMEOUT));

#ifdef ENABLE_UDS
HV_EXPORT int BindUnix(const char* path, int type DEFAULT(SOCK_STREAM));
HV_EXPORT int ListenUnix(const char* path);
HV_EXPORT int ConnectUnix(const char* path, int nonblock DEFAULT(0));
HV_EXPORT int ConnectUnixNonblock(const char* path);
HV_EXPORT int ConnectUnixTimeout(const char* path, int ms DEFAULT(DEFAULT_CONNECT_TIMEOUT));
#endif

// Just implement Socketpair(AF_INET, SOCK_STREAM, 0, sv);
HV_EXPORT int Socketpair(int family, int type, int protocol, int sv[2]);

static inline int tcp_nodelay(int sockfd, int on DEFAULT(1)) {
    return setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (const char*)&on, sizeof(int));
}

static inline int tcp_nopush(int sockfd, int on DEFAULT(1)) {
#ifdef TCP_NOPUSH
    return setsockopt(sockfd, IPPROTO_TCP, TCP_NOPUSH, (const char*)&on, sizeof(int));
#elif defined(TCP_CORK)
    return setsockopt(sockfd, IPPROTO_TCP, TCP_CORK, (const char*)&on, sizeof(int));
#else
    return -10;
#endif
}

static inline int tcp_keepalive(int sockfd, int on DEFAULT(1), int delay DEFAULT(60)) {
    if (setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (const char*)&on, sizeof(int)) != 0) {
        return socket_errno();
    }

#ifdef TCP_KEEPALIVE
    return setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPALIVE, (const char*)&delay, sizeof(int));
#elif defined(TCP_KEEPIDLE)
    // TCP_KEEPIDLE     => tcp_keepalive_time
    // TCP_KEEPCNT      => tcp_keepalive_probes
    // TCP_KEEPINTVL    => tcp_keepalive_intvl
    return setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPIDLE, (const char*)&delay, sizeof(int));
#else
    return 0;
#endif
}

static inline int udp_broadcast(int sockfd, int on DEFAULT(1)) {
    return setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, (const char*)&on, sizeof(int));
}

// send timeout
static inline int so_sndtimeo(int sockfd, int timeout) {
#ifdef OS_WIN
    return setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(int));
#else
    struct timeval tv = {timeout/1000, (timeout%1000)*1000};
    return setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
#endif
}

// recv timeout
static inline int so_rcvtimeo(int sockfd, int timeout) {
#ifdef OS_WIN
    return setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(int));
#else
    struct timeval tv = {timeout/1000, (timeout%1000)*1000};
    return setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif
}

END_EXTERN_C

#endif // HV_SOCKET_H_
