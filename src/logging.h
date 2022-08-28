#ifndef __HM_logging_H__
#define __HM_logging_H__

namespace Hoymiles
{
//#define HM_LOG_SERIAL 1

#ifdef HM_LOG_SERIAL
  #define PRINTSER(...) Serial.print(__VA_ARGS__)
  #define PRINTSERF(...) Serial.printf(__VA_ARGS__)
  #define PRINTSERLN(...) Serial.println(__VA_ARGS__)
#else
  #define PRINTSER(...)
  #define PRINTSERF(...)
  #define PRINTSERLN(...)
#endif

// available levels
#define HM_LOG_ERROR       1
#define HM_LOG_WARN        2
#define HM_LOG_INFO        3
#define HM_LOG_DEBUG       4
#define HM_LOG_VERBOSE     5

// default level
#ifndef HM_SERIAL_LOG_LEVEL
#define HM_SERIAL_LOG_LEVEL HM_LOG_INFO
#endif

#if HM_SERIAL_LOG_LEVEL >= HM_LOG_ERROR
    #define PERR(...) PRINTSER(F("E: ")); PRINTSER(__VA_ARGS__);
    #define PERRF(...) PRINTSER(F("E: ")); PRINTSERF(__VA_ARGS__);
    #define PERRLN(...) PRINTSER(F("E: ")); PRINTSERLN(__VA_ARGS__);
#else
    #define PERR(...)
    #define PERRLN(...)
#endif

#if HM_SERIAL_LOG_LEVEL >= HM_LOG_WARN
    #define PWARN(...) PRINTSER(F("W: ")); PRINTSER(__VA_ARGS__);
    #define PWARNF(...) PRINTSER(F("W: ")); PRINTSERF(__VA_ARGS__);
    #define PWARNLN(...) PRINTSER(F("W: ")); PRINTSERLN(__VA_ARGS__);
#else
    #define PWARN(...)
    #define PWARNF(...)
    #define PWARNLN(...)
#endif

#if HM_SERIAL_LOG_LEVEL >= HM_LOG_INFO
    #define PINFO(...) PRINTSER(F("I: ")); PRINTSER(__VA_ARGS__);
    #define PINFOF(...) PRINTSER(F("I: ")); PRINTSERF(__VA_ARGS__);
    #define PINFOLN(...) PRINTSER(F("I: ")); PRINTSERLN(__VA_ARGS__);
#else
    #define PINFO(...)
    #define PINFOF(...)
    #define PINFOLN(...)
#endif

#if HM_SERIAL_LOG_LEVEL >= HM_LOG_DEBUG
    #define PDBG(...) PRINTSER(F("D: ")); PRINTSER(__VA_ARGS__);
    #define PDBGF(...) PRINTSER(F("D: ")); PRINTSERF(__VA_ARGS__);
    #define PDBGLN(...) PRINTSER(F("D: ")); PRINTSERLN(__VA_ARGS__);
#else
    #define PDBG(...)
    #define PDBGF(...)
    #define PDBGLN(...)
#endif

#if HM_SERIAL_LOG_LEVEL >= HM_LOG_VERBOSE
    #define PVERB(...) PRINTSER(F("V: ")); PRINTSER(__VA_ARGS__);
    #define PVERBF(...) PRINTSER(F("V: ")); PRINTSERF(__VA_ARGS__);
    #define PVERBLN(...) PRINTSER(F("V: ")); PRINTSERLN(__VA_ARGS__);
#else
    #define PVERB(...)
    #define PVERBF(...)
    #define PVERBLN(...)
#endif
}

#endif // __HM_logging_H__