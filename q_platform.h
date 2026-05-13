#pragma once

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
// needed for inet_pton/inet_ntop in ws2tcpip.h
#define _WIN32_WINNT 0x0601
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#else
#include <arpa/inet.h>
#endif

const char *Sys_FindFirst (const char *path);
const char *Sys_FindNext (void);
void Sys_FindClose (void);
void Sys_DebugBreak (void);
