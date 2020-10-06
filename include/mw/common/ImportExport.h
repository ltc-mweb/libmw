#pragma once

#if defined(_MSC_VER)
    //  Microsoft 
    #define MWEXPORT __declspec(dllexport)
    #define MWIMPORT __declspec(dllimport)
#elif defined(__GNUC__)
    //  GCC
    #define MWEXPORT __attribute__((visibility("default")))
    #define MWIMPORT
#else
    #define MWEXPORT
    #define MWIMPORT
#endif