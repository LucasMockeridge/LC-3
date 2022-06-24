#include <iostream>
#include <cstdlib>

#define WINDOWS __CYGWIN__ || _WIN32

#if WINDOWS

#include <Windows.h>

#else

#include <unistd.h>
#include <fcntl.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/termios.h>
#include <sys/mman.h>

#endif

void disableBuffering();
void restoreBuffering();
void handleInterrupt( int signal );
