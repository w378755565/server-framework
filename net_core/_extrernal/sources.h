#pragma once

// export dll function
#define Export _declspec(dllexport)

// import dll function
#define Import _declspec(dllimport)

#define THandler void*

// define data type
typedef signed char    int8;
typedef signed short   int16;

typedef unsigned char  uint8;
typedef unsigned short uint16;

#if defined (_WIN32) || defined (WIN32)

#  ifndef _WIN64
// 32 bit
typedef signed   int  int32;
typedef unsigned int  uint32;
#  else
typedef signed   long int32;
typedef unsigned long uint32;
#  endif
typedef signed __int64   int64;
typedef unsigned __int64 uint64;
#else
#  define BDAQCALL
typedef signed int         int32;
typedef unsigned int       uint32;
typedef signed long long   int64;
typedef unsigned long long uint64;
typedef void *             HANDLE;
#endif

enum LoggerOut {
    File,
    Console
};

enum CPPVersion {
    CPP11,
};

// thread call back function 
typedef void* (*threadcall)(void*);

