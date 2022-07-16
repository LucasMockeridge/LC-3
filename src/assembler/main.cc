#include "Assembler.hh"
#include <iostream>
#include <iomanip>

int main( int argc, char* argv[] )
{
	if ( argc == 1 )
	{
		std::cerr << "usage: ./main a [b ...]\n" << std::setw( 52 ) << "a, b, etc.: path to an LC-3 assembly program\n";
		return 1;
	}
	for (int i = 1; i < argc; i++ )
	{
		Assembler a( argv[i] );
		a.firstPass();
		a.secondPass();
	}
	return 0;
}
