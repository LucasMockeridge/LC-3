#include "CPU.hh"
#include <cstdlib>
#include <iostream>
#include <signal.h>
#include <iomanip>

#include <unistd.h>
#include <fcntl.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/termios.h>
#include <sys/mman.h>

using std::setw, std::cout, std::cerr, std::exit;

struct termios original_tio;

void disableBuffering()
{
    tcgetattr( STDIN_FILENO, &original_tio) ;
    struct termios new_tio = original_tio;
    new_tio.c_lflag &= ~ICANON & ~ECHO;
    tcsetattr( STDIN_FILENO, TCSANOW, &new_tio );
}

void restoreBuffering()
{
    tcsetattr( STDIN_FILENO, TCSANOW, &original_tio );
}

void handleInterrupt( int signal )
{
	restoreBuffering();
	cout << "\n";
	exit( -2 );
}

int main( int argc, char* argv[] )
{
	if ( argc == 1 )
	{
		cerr << "usage: ./main bin1 [bin2 ...]\n" << setw( 59 ) << "bin1, bin2, etc.: path to an assembled LC-3 program\n";
		return 1;
	}
	signal( SIGINT, handleInterrupt );
	disableBuffering();
	CPU cpu;
	cpu.loadPrograms( argc, argv );
	while ( !cpu.isHalted() )
	{
		unsigned short instr = cpu.fetchInstr();
		cpu.handleInstr( instr );
	}
	restoreBuffering();
	return 0;
}
