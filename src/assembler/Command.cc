#include "Command.hh"

Command::Command( const std::string str, int x, int y )
{
	line = x;
	n = y;
	cmd = str;
	tokens = toTokens( cmd );
	isLabel = checkLabel( tokens[0] );
	const int i = isLabel ? 1 : 0; 
	isDirective = checkDirective( tokens[0 + i] );
}

std::vector<std::string>
Command::toTokens( const std::string& str )
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

bool
Command::checkLabel( const std::string& fst )
{
	return fst[fst.length() - 1] == ':';
}

bool
Command::checkDirective( const std::string& fst )
{
	return fst[0] == '.';
}
