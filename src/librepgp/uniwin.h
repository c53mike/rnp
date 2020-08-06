#if !defined(_UNISTD_H) && defined(_MSC_VER)
#define _UNISTD_H 1

/* This is intended as a drop-in replacement for unistd.h on Windows.
 * Please add functionality as neeeded.
 * https://stackoverflow.com/a/826027/1202830
 */

#include <stdlib.h>
#include <io.h>
#include "..\librepgp\getoptwin.h" /* TODO: getopt at: https://gist.github.com/ashelly/7776712 */
#include <process.h>               /* for getpid() and the exec..() family */
#include <direct.h>                /* for _getcwd() and _chdir() */

#define srandom srand
#define random rand

/* Values for the second argument to access.
   These may be OR'd together.  */
#define R_OK 4 /* Test for read permission.  */
#define W_OK 2 /* Test for write permission.  */
//#define   X_OK    1       /* execute permission - unsupported in windows*/
#define F_OK 0 /* Test for existence.  */

#define access _access
#define dup2 _dup2
#define execve _execve
#define ftruncate _chsize
#define unlink _unlink
#define fileno _fileno
#define getcwd _getcwd
#define chdir _chdir
#define isatty _isatty
#define lseek _lseek
/* read, write, and close are NOT being #defined here, because while there are file handle
 * specific versions for Windows, they probably don't work for sockets. You need to look at
 * your app and consider whether to call e.g. closesocket(). */

#ifdef _WIN64
#define ssize_t __int64
#else
#define ssize_t long
#endif

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define NOMINMAX 1
#include <windows.h>

#if _MSC_VER >= 1900
//#include <../ucrt/stdlib.h>
#else
//#include <../include/stdlib.h>
#endif

#define strncasecmp strnicmp
#define strcasecmp stricmp

#ifndef PATH_MAX
#define PATH_MAX _MAX_PATH
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN _MAX_PATH
#endif

#endif