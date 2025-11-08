#ifndef __C_RETURN_TYPE_H__
#define __C_RETURN_TYPE_H__

/// EXTEND FOR C++ //////////////////////////////////////////////////
#ifdef __cplusplus                                                ///
extern "C" {                                                      ///
#endif                                                            ///
/////////////////////////////////////////////////////////////////////

/// #pragma message("[include] cReturnType.h")

#include <stdio.h>
#include <stdint.h>

#include "../../include/projectConfig.h"

/// Default return type
typedef int32_t def;

//
// ──────────────────────────────────────────────────────────────
//   String representations for errors
// ──────────────────────────────────────────────────────────────
//

#define STR_STATUS_OKE                     "STATUS_OKE;"
#define STR_STATUS_ERR                     "STATUS_ERR"

#define STR_STATUS_ERR_NULL                "STATUS_ERR_NULL"
#define STR_STATUS_ERR_MALLOC_FAILED       "STATUS_ERR_MALLOC_FAILED"
#define STR_STATUS_ERR_TIMEOUT             "STATUS_ERR_TIMEOUT"
#define STR_STATUS_ERR_BUSY                "STATUS_ERR_BUSY"
#define STR_STATUS_ERR_INVALID_ARG         "STATUS_ERR_INVALID_ARG"
#define STR_STATUS_ERR_OVERFLOW            "STATUS_ERR_OVERFLOW"
#define STR_STATUS_ERR_UNDERFLOW           "STATUS_ERR_UNDERFLOW"
#define STR_STATUS_ERR_NOT_FOUND           "STATUS_ERR_NOT_FOUND"
#define STR_STATUS_ERR_ALREADY_EXISTS      "STATUS_ERR_ALREADY_EXISTS"
#define STR_STATUS_ERR_NOT_IMPLEMENTED     "STATUS_ERR_NOT_IMPLEMENTED"
#define STR_STATUS_ERR_UNSUPPORTED         "STATUS_ERR_UNSUPPORTED"
#define STR_STATUS_ERR_IO                  "STATUS_ERR_IO"
#define STR_STATUS_ERR_PERMISSION          "STATUS_ERR_PERMISSION"
#define STR_STATUS_ERR_CRC                 "STATUS_ERR_CRC"
#define STR_STATUS_ERR_INIT_FAILED         "STATUS_ERR_INIT_FAILED"
#define STR_STATUS_ERR_PSRAM_FAILED        "STATUS_ERR_PSRAM_FAILED"

// ─ SDL / Xorg / Platform-specific
#define STR_STATUS_ERR_SDL_INIT_FAILED     "STATUS_ERR_SDL_INIT_FAILED"
#define STR_STATUS_ERR_SDL_WINDOW_FAILED   "STATUS_ERR_SDL_WINDOW_FAILED"
#define STR_STATUS_ERR_SDL_RENDER_FAILED   "STATUS_ERR_SDL_RENDER_FAILED"
#define STR_STATUS_ERR_SDL_TTF_FAILED      "STATUS_ERR_SDL_TTF_FAILED"
#define STR_STATUS_ERR_SDL_IMAGE_FAILED    "STATUS_ERR_SDL_IMAGE_FAILED"

#define STR_STATUS_ERR_XORG_DISPLAY_FAILED "STATUS_ERR_XORG_DISPLAY_FAILED"
#define STR_STATUS_ERR_XORG_WINDOW_FAILED  "STATUS_ERR_XORG_WINDOW_FAILED"
#define STR_STATUS_ERR_XORG_DRAW_FAILED    "STATUS_ERR_XORG_DRAW_FAILED"

#define STR_STATUS_ERR_FILE_NOT_FOUND      "STATUS_ERR_FILE_NOT_FOUND"
#define STR_STATUS_ERR_FILE_READ_FAILED    "STATUS_ERR_FILE_READ_FAILED"
#define STR_STATUS_ERR_FILE_WRITE_FAILED   "STATUS_ERR_FILE_WRITE_FAILED"
#define STR_STATUS_ERR_PATH_INVALID        "STATUS_ERR_PATH_INVALID"

//
// ──────────────────────────────────────────────────────────────
//   Enumerated return codes
// ──────────────────────────────────────────────────────────────
//


enum DEFAULT_RETURN_STATUS {
    STATUS_OKE                     = 0,    ///< Success
    STATUS_ERR                     = -1,   ///< Generic error
    STATUS_ERR_NULL                = -2,   ///< Null pointer
    STATUS_ERR_MALLOC_FAILED       = -3,   ///< Memory allocation failed
    STATUS_ERR_TIMEOUT             = -4,   ///< Timeout occurred
    STATUS_ERR_BUSY                = -5,   ///< Resource busy
    STATUS_ERR_INVALID_ARG         = -6,   ///< Invalid argument
    STATUS_ERR_OVERFLOW            = -7,   ///< Buffer overflow
    STATUS_ERR_UNDERFLOW           = -8,   ///< Buffer underflow
    STATUS_ERR_NOT_FOUND           = -9,   ///< Resource not found
    STATUS_ERR_ALREADY_EXISTS      = -10,  ///< Resource already exists
    STATUS_ERR_NOT_IMPLEMENTED     = -11,  ///< Feature not implemented
    STATUS_ERR_UNSUPPORTED         = -12,  ///< Operation unsupported
    STATUS_ERR_IO                  = -13,  ///< Input/output error
    STATUS_ERR_PERMISSION          = -14,  ///< Permission denied
    STATUS_ERR_CRC                 = -15,  ///< CRC mismatch
    STATUS_ERR_INIT_FAILED         = -16,  ///< Initialization failed
    STATUS_ERR_PSRAM_FAILED        = -17,  ///< PSRAM init failed

    // SDL-related
    STATUS_ERR_SDL_INIT_FAILED     = -30,  ///< SDL initialization failed
    STATUS_ERR_SDL_WINDOW_FAILED   = -31,  ///< SDL window creation failed
    STATUS_ERR_SDL_RENDER_FAILED   = -32,  ///< SDL renderer creation failed
    STATUS_ERR_SDL_TTF_FAILED      = -33,  ///< SDL_ttf font load/render failed
    STATUS_ERR_SDL_IMAGE_FAILED    = -34,  ///< SDL_image failed to load file

    // Xorg-related
    STATUS_ERR_XORG_DISPLAY_FAILED = -40,  ///< X display connection failed
    STATUS_ERR_XORG_WINDOW_FAILED  = -41,  ///< X window creation failed
    STATUS_ERR_XORG_DRAW_FAILED    = -42,  ///< X drawing operation failed

    // File & path
    STATUS_ERR_FILE_NOT_FOUND      = -50,  ///< File not found
    STATUS_ERR_FILE_READ_FAILED    = -51,  ///< File read error
    STATUS_ERR_FILE_WRITE_FAILED   = -52,  ///< File write error
    STATUS_ERR_PATH_INVALID        = -53,  ///< Invalid file path
};

//
// ──────────────────────────────────────────────────────────────
//   Helper: Convert error code to string
// ──────────────────────────────────────────────────────────────
//

/// @brief Get a string for a given return code.
/// @param ret Error/status code
/// @return String representation (never NULL)
static inline const char * DEFAULT_RETURN_STATUS_STR(int ret)
{
    switch (ret) {
        case STATUS_OKE:                     return STR_STATUS_OKE;
        case STATUS_ERR:                     return STR_STATUS_ERR;
        case STATUS_ERR_NULL:                return STR_STATUS_ERR_NULL;
        case STATUS_ERR_MALLOC_FAILED:       return STR_STATUS_ERR_MALLOC_FAILED;
        case STATUS_ERR_TIMEOUT:             return STR_STATUS_ERR_TIMEOUT;
        case STATUS_ERR_BUSY:                return STR_STATUS_ERR_BUSY;
        case STATUS_ERR_INVALID_ARG:         return STR_STATUS_ERR_INVALID_ARG;
        case STATUS_ERR_OVERFLOW:            return STR_STATUS_ERR_OVERFLOW;
        case STATUS_ERR_UNDERFLOW:           return STR_STATUS_ERR_UNDERFLOW;
        case STATUS_ERR_NOT_FOUND:           return STR_STATUS_ERR_NOT_FOUND;
        case STATUS_ERR_ALREADY_EXISTS:      return STR_STATUS_ERR_ALREADY_EXISTS;
        case STATUS_ERR_NOT_IMPLEMENTED:     return STR_STATUS_ERR_NOT_IMPLEMENTED;
        case STATUS_ERR_UNSUPPORTED:         return STR_STATUS_ERR_UNSUPPORTED;
        case STATUS_ERR_IO:                  return STR_STATUS_ERR_IO;
        case STATUS_ERR_PERMISSION:          return STR_STATUS_ERR_PERMISSION;
        case STATUS_ERR_CRC:                 return STR_STATUS_ERR_CRC;
        case STATUS_ERR_INIT_FAILED:         return STR_STATUS_ERR_INIT_FAILED;
        case STATUS_ERR_PSRAM_FAILED:        return STR_STATUS_ERR_PSRAM_FAILED;

        case STATUS_ERR_SDL_INIT_FAILED:     return STR_STATUS_ERR_SDL_INIT_FAILED;
        case STATUS_ERR_SDL_WINDOW_FAILED:   return STR_STATUS_ERR_SDL_WINDOW_FAILED;
        case STATUS_ERR_SDL_RENDER_FAILED:   return STR_STATUS_ERR_SDL_RENDER_FAILED;
        case STATUS_ERR_SDL_TTF_FAILED:      return STR_STATUS_ERR_SDL_TTF_FAILED;
        case STATUS_ERR_SDL_IMAGE_FAILED:    return STR_STATUS_ERR_SDL_IMAGE_FAILED;

        case STATUS_ERR_XORG_DISPLAY_FAILED: return STR_STATUS_ERR_XORG_DISPLAY_FAILED;
        case STATUS_ERR_XORG_WINDOW_FAILED:  return STR_STATUS_ERR_XORG_WINDOW_FAILED;
        case STATUS_ERR_XORG_DRAW_FAILED:    return STR_STATUS_ERR_XORG_DRAW_FAILED;

        case STATUS_ERR_FILE_NOT_FOUND:      return STR_STATUS_ERR_FILE_NOT_FOUND;
        case STATUS_ERR_FILE_READ_FAILED:    return STR_STATUS_ERR_FILE_READ_FAILED;
        case STATUS_ERR_FILE_WRITE_FAILED:   return STR_STATUS_ERR_FILE_WRITE_FAILED;
        case STATUS_ERR_PATH_INVALID:        return STR_STATUS_ERR_PATH_INVALID;

        default:                      return "STATUS_ERR_UNKNOWN";
    }
}


/// EXTEND FOR C++ //////////////////////////////////////////////////
#ifdef __cplusplus                                                ///
}                                                                 ///
#endif                                                            ///
/////////////////////////////////////////////////////////////////////
#endif /* __C_RETURN_TYPE_H__ */