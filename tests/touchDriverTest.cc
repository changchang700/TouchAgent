// C
#ifndef _GLIBCXX_NO_ASSERT
#include <cassert>
#endif
#include <cctype>
#include <cerrno>
#include <cfloat>
#include <ciso646>
#include <climits>
#include <clocale>
#include <cmath>
#include <csetjmp>
#include <csignal>
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cwchar>
#include <cwctype>

#if __cplusplus >= 201103L
#include <ccomplex>
#include <cfenv>
#include <cinttypes>
#include <cstdbool>
#include <cstdint>
#include <ctgmath>
#endif

// C++
#include <algorithm>
#include <bitset>
#include <complex>
#include <deque>
#include <exception>
#include <fstream>
#include <functional>
#include <iomanip>
#include <ios>
#include <iosfwd>
#include <iostream>
#include <istream>
#include <iterator>
#include <limits>
#include <list>
#include <locale>
#include <map>
#include <memory>
#include <new>
#include <numeric>
#include <ostream>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <streambuf>
#include <string>
#include <typeinfo>
#include <utility>
#include <valarray>
#include <vector>

#if __cplusplus >= 201103L
#include <array>
#include <atomic>
#include <chrono>
#include <codecvt>
#include <condition_variable>
#include <forward_list>
#include <future>
#include <initializer_list>
#include <mutex>
#include <random>
#include <ratio>
#include <regex>
#include <scoped_allocator>
#include <system_error>
#include <thread>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#endif

#include <arpa/inet.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#define CMD_CLOSE 0x00
#define CMD_SLOT_INFO 0x01
#define CMD_SLOT_AVAILABLE 0x02
#define CMD_TOUCH_DOWN 0x03
#define CMD_TOUCH_MOVE 0x04
#define CMD_TOUCH_UP 0x05

using namespace std;

#define SERVER "127.0.0.1"
#define BUFLEN 8  //Max length of buffer
#define PORT 2233   //The port on which to send data

struct cmd_t {
    __u8 type;  // Always needed
    __u8 id;    // Needed when type is CMD_TOUCH_MOVE || CMD_TOUCH_UP
    __u16 x;    // Needed when type is CMD_TOUCH_DOWN || CMD_TOUCH_MOVE
    __u16 y;    // Needed when type is CMD_TOUCH_DOWN || CMD_TOUCH_MOVE
};

int main(int argc, char **argv) {
    struct sockaddr_in si_other;
    cmd_t cmd;
    char cmdstr[sizeof(cmd_t) + 1] = {0};
    int s, i;
    socklen_t slen = sizeof(si_other);
    char buf[BUFLEN];
    char message[BUFLEN];

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        return -1;
    }

    memset((char *)&si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);

    if (inet_aton(SERVER, &si_other.sin_addr) == 0) {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }

    struct timeval overtime{
        .tv_usec = 10000,
        .tv_sec = 0
    };
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &overtime, sizeof(overtime));

    while (1) {
        printf("Enter operation : ");
        scanf("%d %d %d %d", &cmd.type, &cmd.id, &cmd.x, &cmd.y);
        memcpy(cmdstr, (char *)&cmd, sizeof(cmd_t));
        if (sendto(s, cmdstr, sizeof(cmd_t) + 1, 0, (struct sockaddr *)&si_other, slen) == -1) {
            printf("Send Error!\n");
            return -1;
        }
        if(cmd.type == CMD_SLOT_INFO || cmd.type == CMD_SLOT_AVAILABLE){
            memset(buf, 0, BUFLEN);
            if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *)&si_other, &slen) == -1) {
                printf("Receive Error!\n");
            }else{
                printf("%s\n", buf);
            }
        }
    }
    close(s);
    return 0;
}