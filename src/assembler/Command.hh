#include <string>
#include <vector>

class Command
{
	public:
		Command( const std::string line, int x, int y );
		int line;
		int n;
		std::string cmd;
		std::vector<std::string> tokens;
		bool isLabel;
		bool isDirective;
	private:
		std::vector<std::string> toTokens( const std::string& str );
		bool checkLabel( const std::string& fst );
		bool checkDirective( const std::string& fst );
};
