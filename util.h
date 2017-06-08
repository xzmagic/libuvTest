#ifndef _KMLC_UTILS_H_
#define _KMLC_UTILS_H_
#include <cstdint>
#include <cstring>
#include <cstdarg>
#include <vector>
#include <string>
#include <ctime>
#include <chrono>
#include <map>
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

#define swap16(A) ((((uint16)(A)&0xff00) >> 8) | (((uint16)(A)&0x00ff) << 8))
#define swap32(A)                                                                                           \
    ((((uint32)(A)&0xff000000) >> 24) | (((uint32)(A)&0x00ff0000) >> 8) | (((uint32)(A)&0x0000ff00) << 8) | \
     (((uint32)(A)&0x000000ff) << 24))

#define swap64(A) \
    ((uint64)(swap32((uint32)(((A)&0x00000000ffffffff)))) << 32 | (swap32((uint32)(((A)&0xffffffff00000000) >> 32))))

int isNetOrder();

uint64 hton64(uint64 h);

uint64 ntoh64(uint64 n);

uint32 hton32(uint32 h);

uint32 ntoh32(uint32 n);

uint16 hton16(uint16 h);

uint16 ntoh16(uint16 n);

// struct in_addr {
//     unsigned long s_addr;  // load with inet_pton()
// };
// struct sockaddr_in {
//     short sin_family;         // e.g. AF_INET, AF_INET6
//     unsigned short sin_port;  // e.g. htons(3490)
//     struct in_addr sin_addr;  // see struct in_addr, below
//     char sin_zero[8];         // zero this if you want to
// };
// struct sockaddr {
//     unsigned short sa_family;  // address family, AF_xxx
//     char sa_data[14];          // 14 bytes of protocol address
// };

std::string addr2Str(struct sockaddr &sockAddr);
struct sockaddr str2Addr(std::string addr);
struct sockaddr toAddr(std::string ip, int port);

std::string format(const char *fmt, ...);

void print(std::string info, std::string color = "cyan");
int crc32(unsigned char *buffer, uint32_t bufsize);

#endif