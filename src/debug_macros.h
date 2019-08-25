#ifndef DEBUG_MACROS
#define DEBUG_MACROS

#ifdef SERIAL_DEBUG_ENABLED
    #define DebugPrintBegin(speed)  \
        Serial.begin(speed);
    #define DebugPrintFull(...)     \
        Serial.print(millis());     \
        Serial.print(":\t");        \
        Serial.print(__FILE__);     \
        Serial.print(' ');          \
        Serial.print(__func__);     \
        Serial.print(" (l");        \
        Serial.print(__LINE__);     \
        Serial.print("):\t");       \
        Serial.print(__VA_ARGS__)
    #define DebugPrintlnFull(...)   \
        Serial.print(millis());     \
        Serial.print(":\t");        \
        Serial.print(__FILE__);     \
        Serial.print(' ');          \
        Serial.print(__func__);     \
        Serial.print(" (l");        \
        Serial.print(__LINE__);     \
        Serial.print("):\t");       \
        Serial.println(__VA_ARGS__)
    #define DebugPrint(...)         \
        Serial.print(__VA_ARGS__)
    #define DebugPrintln(...)       \
        Serial.println(__VA_ARGS__)
#else
    #define DebugPrintBegin(speed)
    #define DebugPrintFull(...)
    #define DebugPrintlnFull(...)
    #define DebugPrint(...)
    #define DebugPrintln(...)
#endif

#endif