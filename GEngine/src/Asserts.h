//
// Created by gaetz on 29/07/2021.
//

#ifndef ASSERTS_H
#define ASSERTS_H


#include <string>
#include "Defines.h"

using std::string;

#define GASSERTIONS_ENABLED

// Break function
// Under windows, clang emulates msvc compiler
#ifdef GASSERTIONS_ENABLED

namespace engine {
    // Report when an assertion does no work
    GAPI void reportAssertionFailure(const string& expression, const string& message, const char* codeFile, i32 codeLine);
}

    #if _MSC_VER
        #include <intrin.h>
        #define debugBreak() __debugbreak()
    #else
        #define debugBreak() __builtin_trap()
    #endif

    // Assertions for all builds
    #define GASSERT(expr)                                               \
        {                                                               \
            if(expr) {                                                  \
            } else {                                                    \
                engine::reportAssertionFailure(#expr, "", __FILE__, __LINE__);  \
                debugBreak();                                           \
            }                                                           \
        }

    // Assertions with message for all builds
    #define GASSERT_MSG(expr, message)                                      \
        {                                                                   \
            if(expr) {                                                      \
            } else {                                                        \
                engine::reportAssertionFailure(#expr, message, __FILE__, __LINE__); \
                debugBreak();                                               \
            }                                                               \
        }

    #endif

    // Debug build assertion
    #ifdef GDEBUG
        #define GASSERT_DEBUG(expr)                                         \
            {                                                               \
                if(expr) {                                                  \
                } else {                                                    \
                    engine::reportAssertionFailure(#expr, "", __FILE__, __LINE__);  \
                    debugBreak();                                           \
                }                                                           \
            }
    #else
        #define GASSERT_DEBUG(expr)     // Does nothing
    #endif
#else
    #define GASSERT(expr)               // Does nothing
    #define GASSERT_MSG(expr, message)  // Does nothing
    #define GASSERT_DEBUG(expr)         // Does nothing
#endif