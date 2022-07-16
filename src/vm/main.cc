#include "CPU.hh"
#include "platform.hh"
#include <iostream>
#include <signal.h>
#include <iomanip>

int main( int argc, char* argv[] )
{
	if ( argc == 1 )
	{
		std::cerr << "usage: ./main bin1 [bin2 ...]\n" << std::setw( 59 ) << "bin1, bin2, etc.: path to an assembled LC-3 program\n";
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
