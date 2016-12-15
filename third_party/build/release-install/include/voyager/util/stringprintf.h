#ifndef VOYAGER_UTIL_STRINGPRINTF_H_
#define VOYAGER_UTIL_STRINGPRINTF_H_

#include <stdio.h>
#include <stdarg.h>

#include <string>

namespace voyager {

// Lower-level routine that takes a va_list and appends to a specified
// string. All other routines are just convenience wrappers around it.
extern void StringAppendV(std::string* dst, const char* format, va_list ap);

// Return a C++ string.
extern std::string StringPrintf(const char* format, ...);

// Store result into a supplied string and return it.
// The previous dst will be clear.
extern const std::string& SStringPrintf(std::string* dst,
                                        const char* format, ...);

// Append result into a supplied string.
extern void StringAppendF(std::string* dst, const char* format, ...);

}  // namespace voyager

#endif  // VOYAGER_UTIL_STRINGPRINTF_H_
