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
	int x = 1;
	int y = 0;
	while ( getCommand( f, str ) )
	{
		if ( isNotWhiteSpace( str ) )
		{
			Command cmd = Command( str, x, y );
			tokens.push_back( cmd );
			y++;
		}
		x++;
	}
}

void
Assembler::checkLiteral( const Command& cmd, const std::string& s )
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
			errorMessage( cmd, "Invalid literal: " + s ); 
		}
	}
}

int
Assembler::convertNumber( const Command& cmd, const std::string& s )
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
	checkLiteral( cmd, str );
	return binary ? std::stoi( str, 0, 2 ) : std::stoi( str, 0, 0 );
}

void
Assembler::checkOrig()
{
	const std::vector<std::string> fst = tokens[0].tokens;
	if ( fst[0] != ".ORIG" )
	{
		errorMessage( tokens[0], "First command must be an .ORIG directive" );
	}
	else if ( fst.size() != 2 )
	{
		errorMessage( tokens[0], ".ORIG requires one argument" );
	}
	const int addr = convertNumber( tokens[0], fst[1] );
	if ( addr < 0x3000 || addr > 0xFDFF )
	{
		errorMessage( tokens[0], "Address out of bounds: " + std::to_string( addr ) );
	}
	start = addr;
}

void
Assembler::checkEnd()
{
	const std::vector<std::string> end = tokens[tokens.size() - 1].tokens;
	if ( end[0] != ".END" )
	{
		errorMessage( tokens[tokens.size() - 1], "Last command must be an .END directive" );
	}
	else if ( end.size() != 1 )
	{
		errorMessage( tokens[tokens.size() - 1], ".END requires no arguments" );
	}
}

void
Assembler::buildTable()
{
	int j = start;
	for ( std::vector<Command>::size_type i = 1; i < tokens.size(); i++ )
	{
		const Command cmd = tokens[i];
		const std::vector<std::string> v = cmd.tokens;
		if ( cmd.isLabel )
		{
			const std::string label = v[0].substr( 0, v[0].length() - 1 );
			symbolTable.insert( std::pair<std::string, int>( label, j ) );
		}
		if ( cmd.isDirective )
		{
			const std::string dir = cmd.isLabel ? v[1] : v[0];
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
				errorMessage( cmd, "Invalid directive: " + dir );
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
	unsigned short origin = convertNumber( tokens[0], tokens[0].tokens[1] );
	toLittleEndian( origin );
	f.write( reinterpret_cast<const char*>( &origin ),  sizeof origin );
	for ( std::vector<Command>::size_type i = 1; i < tokens.size(); i++ )
	{
		handleTokens( tokens[i], f );
	}
	f.close();
}

void
Assembler::handleTokens( const Command& cmd, std::ofstream& stream )
{
	if ( cmd.isDirective )
	{
		handleDirectives( cmd, stream );
	} 
	else
	{
		const int i = cmd.isLabel ? 1 : 0;
		const std::string fst = cmd.tokens[i];
		const int opcode = getOpcode( cmd, fst );
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
		else if ( opbit & 0x1011 || fst == "TRAP" )
		{
			args = 2;
		}
		argumentsCheck( cmd, fst, cmd.tokens.size() - 1 - i, args - 1 );
		short r0, r1, r2, target;
		unsigned short val = opcode << 12;
		if ( opbit & 0x5EEE || fst == "JSRR" )
		{
			registerCheck( cmd, cmd.tokens[i + 1] );
			r0 = std::stoi( std::string( 1, cmd.tokens[i + 1][1] ) );
			const int shift = fst == "JSRR" || fst == "JMP" ? 6 : 9;
			val |= r0 << shift;
		}
		if ( opbit & 0x2E2 )
		{
			registerCheck( cmd, cmd.tokens[i + 2] );
			r1 = std::stoi( std::string( 1, cmd.tokens[i + 2][1] ) );
			val |= r1 << 6;
		} 
		if ( opbit & 0x4C0D || fst == "JSR" )
		{
			const int x = fst == "JSR" || opcode == BR ? 1 : 2;
			target = checkSymbol( cmd.tokens[i + x] ) ? symbolTable.at( cmd.tokens[i + x] ) - start - cmd.n : convertNumber( cmd, cmd.tokens[i + x] );
			const int len = fst == "JSR" ? 11 : 9;
			checkOperand( cmd, target, len, true );			
			target &= ( 1 << len ) - 1; 
			target = fst == "JSR" ? target | 1 << 11 : target;
			val |= target;
		}
		if ( fst == "AND" || fst == "ADD" )
		{
			const bool imm = !checkRegister( cmd.tokens[i + 3] );
			r2 = imm ? convertNumber( cmd, cmd.tokens[i + 3] ) : std::stoi( std::string( 1, cmd.tokens[i + 3][1] ) );	
			if ( imm )
			{
				checkOperand( cmd, r2, 5, true );
				r2 &= 0x1F;
			}
			val |= r2;
			val = imm ? val | 1 << 5 : val;
		} 
		else if ( fst == "LDR" || fst == "STR" )
		{
			int offset = convertNumber( cmd, cmd.tokens[i + 3] );
			checkOperand( cmd, offset, 6, true );
			offset &= 0x3F;
			val |= offset;
		} 
		else if ( fst == "NOT" )
		{
			val |= 0x3F;
		}
		else if ( fst == "RET" )
		{
			val |= 7 << 6;
		}
		else if ( fst == "TRAP" )
		{
			const unsigned short tv = convertNumber( cmd, cmd.tokens[i + 1] );
			checkOperand( cmd, tv, 8, false );
			val |= tv;
		}
		else if ( opcode == BR )
		{
			val |= getMask( fst );	
		}
		else if ( opcode == TRAP )
		{
			val |= getVector( fst ); 
		}

		toLittleEndian( val );
		stream.write( reinterpret_cast<const char*>( &val ),  sizeof val );	
	}
}

int
Assembler::getVector( const std::string& fst )
{
	if ( fst == "GETC" )
	{
		return 0x20;
	} 
	else if ( fst == "OUT" )
	{
		return 0x21;
	}
	else if ( fst == "PUTS" )
	{
		return 0x22;
	}
	else if ( fst == "IN" )
	{
		return 0x23;
	}
	else if ( fst == "PUTSP" )
	{
		return 0x24;
	}
	else
	{
		return 0x25;
	}
}

int
Assembler::getMask( const std::string& fst )
{
	if ( fst == "BR" || fst == "BRnzp" )
	{
		return BRnzp;
	}
	else if ( fst == "BRp" )
	{
		return BRp;
	}
	else if ( fst == "BRz" )
	{
		return BRz;
	} 
	else if ( fst == "BRn" )
	{
		return BRn;
	}
	else if ( fst == "BRzp" )
	{
		return BRzp;
	}
	else if ( fst == "BRnp" )
	{
		return BRnp;
	} 
	else
	{
		return BRnz;
	}
}

int
Assembler::getOpcode( const Command& cmd, const std::string& fst )
{
	if ( fst == "BR" || fst == "BRp" || fst == "BRz" || fst == "BRn"
		|| fst == "BRzp" || fst == "BRnp" || fst == "BRnz" || fst == "BRnzp" )
	{
		return BR;
	}
	else if ( fst == "ADD" )
	{
		return ADD;
	}
	else if ( fst == "LD" )
	{
		return LD;
	}
	else if ( fst == "ST" )
	{
		return ST;
	}
	else if ( fst == "JSR" || fst == "JSRR" )
	{
		return JSR;
	}
	else if ( fst == "AND" )
	{
		return AND;
	}
	else if ( fst == "LDR" )
	{
		return LDR;
	}
	else if ( fst == "STR" )
	{
		return STR;
	}
	else if ( fst == "RTI" )
	{
		return RTI;
	}
	else if ( fst == "NOT" )
	{
		return NOT;
	}
	else if ( fst == "LDI" )
	{
		return LDI;
	}
	else if ( fst == "STI" )
	{
		return STI;
	}
	else if ( fst == "JMP" )
	{
		return JMP;
	}
	else if ( fst == "RES" )
	{
		return RES;
	}
	else if ( fst == "LEA" )
	{
		return LEA;
	}
	else if ( fst == "TRAP" || fst == "GETC" || fst == "OUT" || fst == "PUTS"
		|| fst == "IN" || fst == "PUTSP" || fst == "HALT" )
	{
		return TRAP;
	}
	errorMessage( cmd, "Unknown Command: " + fst );
	return 0;
}

void
Assembler::handleDirectives( const Command& cmd, std::ofstream& stream )
{
	const std::string dir = cmd.isLabel ? cmd.tokens[1] : cmd.tokens[0];
	if ( dir == ".ORIG" )
	{
		errorMessage( cmd, "More than one .ORIG in program" );
	}
	else if ( dir == ".FILL" )
	{
		unsigned short val = convertNumber( cmd, cmd.tokens[cmd.tokens.size() - 1] );
		toLittleEndian( val );
		stream.write( reinterpret_cast<const char*>( &val ),  sizeof val );
	}
	else if ( dir == ".BLKW" )
	{
		const unsigned short zero = 0;
		for ( int i = 0; i < std::stoi( cmd.tokens[cmd.tokens.size() - 1] ); i++ )
		{
			stream.write( reinterpret_cast<const char*>( &zero ), sizeof zero );
		}
	}
	else if ( dir == ".STRINGZ" )
	{
		const std::string str = cmd.tokens[cmd.tokens.size() - 1];
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
		errorMessage( cmd, "Unknown directive: " + dir );
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
Assembler::checkOperand( const Command& cmd, const int& operand, const int& bits, const bool& isSigned )
{
	bool predicate = isSigned ? operand < -( 2 << ( bits - 1 ) ) || operand > ( 2 << ( bits - 1 ) ) - 1 : operand < 0 || operand > ( 2 << ( bits ) ) - 1;
	if ( predicate )
	{
		errorMessage( cmd, "Operand: " + std::to_string( operand ) + " cannot be represented in " + std::to_string( bits ) + " bits" );
	}
}

bool
Assembler::checkRegister( const std::string& reg )
{
	return !( reg.length() != 2 || reg[0] != 'R' || std::stoi( std::string( 1, reg[1] ) ) > 7 );
}

void
Assembler::registerCheck( const Command& cmd, const std::string& reg )
{
	if ( !checkRegister( reg ) )
	{
		errorMessage( cmd, "Invalid register: " + reg );
	}
}

void
Assembler::argumentsCheck( const Command& cmd, const std::string& fst, const int& length, const int& n )
{
	if ( length != n )
	{
		errorMessage( cmd, std::to_string( length ) + " arguments provided to " + fst + ", " + std::to_string( n ) + " arguments required" );
	}
}

void
Assembler::errorMessage( const Command& cmd, const std::string& err )
{
	throw std::runtime_error( "Error on line " + std::to_string( cmd.line ) + " : " + cmd.cmd + '\n' + err );
}
