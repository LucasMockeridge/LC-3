#include "platform.hh"

#if WINDOWS

DWORD fdwMode, fdwOldMode;
HANDLE hStdin = GetStdHandle( STD_INPUT_HANDLE );

void disableBuffering()
{
	GetConsoleMode( hStdin, &fdwOldMode );
	fdwMode = fdwOldMode
			^ ENABLE_ECHO_INPUT
			^ ENABLE_LINE_INPUT;
	SetConsoleMode( hStdin, fdwMode );
	FlushConsoleInputBuffer( hStdin );
}

void restoreBuffering()
{
	SetConsoleMode( hStdin, fdwOldMode );
}

#else

struct termios originalTio;

void disableBuffering()
{
	tcgetattr( STDIN_FILENO, &originalTio );
	struct termios newTio = originalTio;
	newTio.c_lflag &= ~ICANON & ~ECHO;
	tcsetattr( STDIN_FILENO, TCSANOW, &newTio );
}

void restoreBuffering()
{
	tcsetattr( STDIN_FILENO, TCSANOW, &originalTio );
}

#endif

void handleInterrupt( int signal )
{
	restoreBuffering();
	std::cout << "\n";
	std::exit( -2 );
}
