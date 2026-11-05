/* Included before every C++ file (-include).
 * libstdc++'s thread layer includes the toolchain's <pthread.h>, which,
 * if <unistd.h> came first (graphics.cc, archive.cc), turns read(),
 * close() ... into macros (cancellation points), and <istream>'s read()
 * method no longer compiles. Taken first, <pthread.h> leaves them alone.
 * The AmiCradle Team. GPL 2 or later, like Tennix. */
#ifdef __cplusplus
#include <pthread.h>
#endif
