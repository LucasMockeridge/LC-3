#include "Assembler.hh"
#include <stdexcept>

enum Opcode
{
	BR,
	ADD,
	LD,
	ST,
	JSR,
	AND,
	LDR,
	STR,
	RTI,
	NOT,
	LDI,
	STI,
	JMP,
	RES,
	LEA,
	TRAP
};

enum Branches
{
	BRp = 0x200,
	BRz = 0x400,
	BRn = 0x800,
	BRzp = 0x600,
	BRnp = 0xA00,
	BRnz = 0xC00,
	BRnzp = 0xE00
};

enum Traps
{
	GETC = 0x20,
	OUT,
	PUTS,
	IN,
	PUTSP,
	HALT
};

Assembler::Assembler( const char* name ) : filename( name ) { }

std::istream&
Assembler::getCommand( std::istream& stream, std::string& str )
{
	char c;
	str.clear();
	while ( stream.get( c ) && c != '\n' )
	{
		if ( c == ';' )
		{
			while ( stream.get() != '\n' );
			break;	
		}
		else
		{
			str.push_back( c );
		}
	}
	return stream;
}

bool
Assembler::isNotWhiteSpace( const std::string& str )
{
	for ( const char& c : str )
	{
		if ( c != ' ' && c != '\t' )
		{
			return true;
		}
	}
	return false;
}

std::vector<std::string>
Assembler::makeTokens( const std::string& str ) 
{
	std::vector<std::string> v;
	std::string s;
	bool inString = false;
	for ( const char& c : str )
	{
		if ( c == '\'' || c == '\"' )
		{
			inString = !inString;
		}
		else if ( ( c != '\t' && c != ' ' && c != ',' ) || inString )
		{
			s.push_back( c );
		}
		else
		{
			if ( s.length() > 0 && !inString )
			{
				v.push_back( s );
				s.clear();
			}
		}
	}	
	if ( s.length() > 0 && !inString )
	{
		v.push_back( s );
	}
	return v;
}

void
Assembler::toTokens()
{
	std::ifstream f;
	f.open( filename );
	std::string str;
	while ( getCommand( f, str ) )
	{
		if ( isNotWhiteSpace( str ) )
		{
			tokens.push_back( makeTokens( str ) );
		}
	}
}

void
Assembler::checkLiteral( const std::string& s )
{
	try 
	{ 
		std::stoi( s, 0, 0 );
	}
	catch( std::invalid_argument const& ex )
	{
		try
		{
			std::stoi( s, 0, 2 );	
		}
		catch( std::invalid_argument const& ex )
		{
			throw std::runtime_error( "Invalid literal: " + s ); 
		}
	}
}

int
Assembler::convertNumber( const std::string& s )
{
	std::string str = s;
	bool binary = false;
	if ( s[0] == '#' )
	{
		str = s.substr( 1, s.length() - 1 );
	}
	else if ( s[0] == 'x' )
	{
		str = "0x" + s.substr( 1, s.length() - 1 );
	}
	else if ( s[0] == 'b' )
	{
		str = s.substr( 1, s.length() - 1 );
		binary = true;
	}
	checkLiteral( str );
	return binary ? std::stoi( str, 0, 2 ) : std::stoi( str, 0, 0 );
}

void
Assembler::checkOrig()
{
	const std::vector<std::string> fst = tokens[0];
	if ( fst[0] != ".ORIG" )
	{
		throw std::runtime_error( "First command must be an .ORIG directive" );
	}
	else if ( fst.size() != 2 )
	{
		throw std::runtime_error( ".ORIG requires one argument" );
	}
	const int addr = convertNumber( fst[1] );
	if ( addr < 0x3000 || addr > 0xFDFF )
	{
		throw std::runtime_error( "Address out of bounds" );
	}
	start = addr;
}

void
Assembler::checkEnd()
{
	const std::vector<std::string> end = tokens[tokens.size() - 1];
	if ( end[0] != ".END" )
	{
		throw std::runtime_error( "Last command must be an .END directive" );
	}
	else if ( end.size() != 1 )
	{
		throw std::runtime_error( ".END requires no arguments" );
	}
}

void
Assembler::buildTable()
{
	int j = start;
	for ( std::vector<std::vector<std::string>>::size_type i = 1; i < tokens.size(); i++ )
	{
		const std::vector<std::string> v = tokens[i];
		if ( v[0][v[0].length() - 1] == ':' )
		{
			const std::string label = v[0].substr( 0, v[0].length() - 1 );
			symbolTable.insert( std::pair<std::string, int>( label, j ) );
		}
		if ( v[0][0] == '.' || ( v.size() > 1 && v[1][0] == '.' ) )
		{
			const std::string dir = v[0][0] == '.' ? v[0] : v[1];
			if ( dir == ".FILL" )
			{
				j++;
			}
			else if ( dir == ".BLKW" )
			{
				j += std::stoi( v[v.size() - 1] );
			}
			else if ( dir == ".STRINGZ" )
			{
				j += v[v.size() - 1].length() + 1;	
			}
			else if ( dir != ".ORIG" && dir != ".END" )
			{
				throw std::runtime_error( "Invalid directive " + dir + " at command " + std::to_string( i ) );
			}
		}
		else
		{
			j++;
		}
	}
}

void
Assembler::firstPass()
{
	toTokens();
	if ( tokens.size() > 0 )
	{
		checkOrig();
		checkEnd();
		buildTable();
	}
}

void
Assembler::secondPass()
{
	const std::string inName = filename;
	const std::string outName = inName.substr( 0, inName.rfind( '.' ) ) + ".obj";
	std::ofstream f;
	f.open( outName, std::ios::binary );
	unsigned short origin = convertNumber( tokens[0][1] );
	toLittleEndian( origin );
	f.write( reinterpret_cast<const char*>( &origin ),  sizeof origin );
	for ( std::vector<std::vector<std::string>>::size_type i = 1; i < tokens.size(); i++ )
	{
		handleTokens( i, tokens[i], f );
	}
	f.close();
}

void
Assembler::handleTokens( const int& i, const std::vector<std::string>& tokens, std::ofstream& stream )
{
	if ( tokens[0][0] == '.' || ( tokens.size() > 1 && tokens[1][0] == '.' ) )
	{
		handleDirectives( tokens, stream );
	} 
	else if ( tokens[0][tokens[0].length() - 1] != ':' || ( tokens.size() > 1 && tokens[1][tokens[1].length() - 1] != ':' ) )
	{
		const int fst = tokens[0][tokens[0].length() - 1] != ':' ? 0 : 1;
		const std::string cmd = tokens[fst];
		const int opcode = getOpcode( cmd );
		const unsigned short opbit = 1 << opcode;
		int args = 1;
		if ( opbit & 0xE2 )
		{
			args = 4;
		}
		else if ( opbit & 0x4E0C )
		{
			args = 3;
		}
		else if ( opbit & 0x1011 || cmd == "TRAP" )
		{
			args = 2;
		}
		argumentsCheck( tokens[0], tokens.size(), args + fst );
		short r0, r1, r2, target;
		unsigned short val = opcode << 12;
		if ( opbit & 0x5EEE || cmd == "JSRR" )
		{
			checkRegister( tokens[fst + 1] );
			r0 = std::stoi( std::string( 1, tokens[fst + 1][1] ) );
			const int shift = cmd == "JSRR" || cmd == "JMP" ? 6 : 9;
			val |= r0 << shift;
		}
		if ( opbit & 0x2E2 )
		{
			checkRegister( tokens[fst + 2] );
			r1 = std::stoi( std::string( 1, tokens[fst + 2][1] ) );
			val |= r1 << 6;
		} 
		if ( opbit & 0x4C0D || cmd == "JSR" )
		{
			const int x = cmd == "JSR" || opcode == BR ? 1 : 2;
			target = checkSymbol( tokens[fst + x] ) ? symbolTable.at( tokens[fst + x] ) - start - i : convertNumber( tokens[fst + x] );
			const int len = cmd == "JSR" ? 11 : 9;
			checkOperand( target, len, true );			
			target &= ( 1 << len ) - 1; 
			target = cmd == "JSR" ? target | 1 << 11 : target;
			val |= target;
		}
		if ( cmd == "AND" || cmd == "ADD" )
		{
			const bool imm = !checkRegister( tokens[fst + 3] );
			r2 = imm ? convertNumber( tokens[fst + 3] ) : std::stoi( std::string( 1, tokens[fst + 3][1] ) );	
			if ( imm )
			{
				checkOperand( r2, 5, true );
				r2 &= 0x1F;
			}
			val |= r2;
			val = imm ? val | 1 << 5 : val;
		} 
		else if ( cmd == "LDR" || cmd == "STR" )
		{
			int offset = convertNumber( tokens[fst + 3] );
			checkOperand( offset, 6, true );
			offset &= 0x3F;
			val |= offset;
		} 
		else if ( cmd == "NOT" )
		{
			val |= 0x3F;
		}
		else if ( cmd == "RET" )
		{
			val |= 7 << 6;
		}
		else if ( cmd == "TRAP" )
		{
			const unsigned short tv = convertNumber( tokens[fst + 1] );
			checkOperand( tv, 8, false );
			val |= tv;
		}
		else if ( opcode == BR )
		{
			val |= getMask( cmd );	
		}
		else if ( opcode == TRAP )
		{
			val |= getVector( cmd ); 
		}

		toLittleEndian( val );
		stream.write( reinterpret_cast<const char*>( &val ),  sizeof val );	
	}
}

int
Assembler::getVector( const std::string& cmd )
{
	if ( cmd == "GETC" )
	{
		return 0x20;
	} 
	else if ( cmd == "OUT" )
	{
		return 0x21;
	}
	else if ( cmd == "PUTS" )
	{
		return 0x22;
	}
	else if ( cmd == "IN" )
	{
		return 0x23;
	}
	else if ( cmd == "PUTSP" )
	{
		return 0x24;
	}
	else
	{
		return 0x25;
	}
}

int
Assembler::getMask( const std::string& cmd )
{
	if ( cmd == "BR" || cmd == "BRnzp" )
	{
		return BRnzp;
	}
	else if ( cmd == "BRp" )
	{
		return BRp;
	}
	else if ( cmd == "BRz" )
	{
		return BRz;
	} 
	else if ( cmd == "BRn" )
	{
		return BRn;
	}
	else if ( cmd == "BRzp" )
	{
		return BRzp;
	}
	else if ( cmd == "BRnp" )
	{
		return BRnp;
	} 
	else
	{
		return BRnz;
	}
}

int
Assembler::getOpcode( const std::string& cmd )
{
	if ( cmd == "BR" || cmd == "BRp" || cmd == "BRz" || cmd == "BRn"
		|| cmd == "BRzp" || cmd == "BRnp" || cmd == "BRnz" || cmd == "BRnzp" )
	{
		return BR;
	}
	else if ( cmd == "ADD" )
	{
		return ADD;
	}
	else if ( cmd == "LD" )
	{
		return LD;
	}
	else if ( cmd == "ST" )
	{
		return ST;
	}
	else if ( cmd == "JSR" || cmd == "JSRR" )
	{
		return JSR;
	}
	else if ( cmd == "AND" )
	{
		return AND;
	}
	else if ( cmd == "LDR" )
	{
		return LDR;
	}
	else if ( cmd == "STR" )
	{
		return STR;
	}
	else if ( cmd == "RTI" )
	{
		return RTI;
	}
	else if ( cmd == "NOT" )
	{
		return NOT;
	}
	else if ( cmd == "LDI" )
	{
		return LDI;
	}
	else if ( cmd == "STI" )
	{
		return STI;
	}
	else if ( cmd == "JMP" )
	{
		return JMP;
	}
	else if ( cmd == "RES" )
	{
		return RES;
	}
	else if ( cmd == "LEA" )
	{
		return LEA;
	}
	else if ( cmd == "TRAP" || cmd == "GETC" || cmd == "OUT" || cmd == "PUTS"
		|| cmd == "IN" || cmd == "PUTSP" || cmd == "HALT" )
	{
		return TRAP;
	}
	throw std::runtime_error( "Command: " + cmd + " not recognised" );
}

void
Assembler::handleDirectives( const std::vector<std::string>& tokens, std::ofstream& stream )
{
	const std::string dir = tokens[0][0] == '.' ? tokens[0] : tokens[1];
	if ( dir == ".ORIG" )
	{
		throw std::runtime_error( "More than one .ORIG" );
	}
	else if ( dir == ".FILL" )
	{
		unsigned short val = convertNumber( tokens[tokens.size() - 1] );
		toLittleEndian( val );
		stream.write( reinterpret_cast<const char*>( &val ),  sizeof val );
	}
	else if ( dir == ".BLKW" )
	{
		const unsigned short zero = 0;
		for ( int i = 0; i < std::stoi( tokens[tokens.size() - 1] ); i++ )
		{
			stream.write( reinterpret_cast<const char*>( &zero ), sizeof zero );
		}
	}
	else if ( dir == ".STRINGZ" )
	{
		const std::string str = tokens[tokens.size() - 1];
		for ( std::string::size_type i = 0; i < str.size(); i++ )
		{
			unsigned short c = str[i];
			if ( c == 92 && i+1 < str.length() && str[i+1] == 'n' )
			{
				c = 10;
				i++;
			}
			toLittleEndian( c );
			stream.write( reinterpret_cast<const char*>( &c ), sizeof c );
		}
		const unsigned short nt = '\0';
		stream.write( reinterpret_cast<const char*>( &nt ), sizeof nt );	
	}
	else if ( dir != ".END" )
	{
		throw std::runtime_error( "Unknown directive: " + dir );
	}
}

void
Assembler::toLittleEndian( unsigned short& x )
{
	x = x << 8 | x >> 8;
}

bool
Assembler::checkSymbol( const std::string& symbol )
{
	try
	{
		symbolTable.at( symbol );
		return true;
	}
	catch( const std::out_of_range& err )
	{
		return false;
	}
}

void
Assembler::checkOperand( const int& operand, const int& bits, const bool& isSigned )
{
	bool predicate = isSigned ? operand < -( 2 << ( bits - 1 ) ) || operand > ( 2 << ( bits - 1 ) ) - 1 : operand < 0 || operand > ( 2 << ( bits ) ) - 1;
	if ( predicate )
	{
		throw std::runtime_error( "Operand: " + std::to_string( operand ) + " cannot be represented in " + std::to_string( bits ) + " bits" );
	}
}

bool
Assembler::checkRegister( const std::string& reg )
{
	return !( reg.length() != 2 || reg[0] != 'R' || std::stoi( std::string( 1, reg[1] ) ) > 7 );
}

void
Assembler::registerCheck( const std::string& reg )
{
	if ( !checkRegister( reg ) )
	{
		throw std::runtime_error( "Invalid register: " + reg );
	}
}

void
Assembler::argumentsCheck( const std::string& cmd, const int& length, const int& n )
{
	if ( length != n )
	{
		throw std::runtime_error( std::to_string( length ) + " arguments provided to " + cmd + ", " + std::to_string( n ) + " arguments required" );
	}
}
