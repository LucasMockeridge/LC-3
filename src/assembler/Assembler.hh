#include "Command.hh"
#include <string>
#include <fstream>
#include <map>
#include <vector>

class Assembler
{
	public:
		Assembler( const char* name );
		void toTokens();
		void firstPass();
		void secondPass();
	private:
		const char* filename;
		int start;
		std::vector<Command> tokens;
		std::map<std::string, int> symbolTable;
		std::istream& getCommand( std::istream& stream, std::string& str );
		bool isNotWhiteSpace( const std::string& str );
		void checkLiteral( const Command& cmd, const std::string& s );
		void checkOrig();
		void checkEnd();
		void buildTable();
		int convertNumber( const Command& cmd, const std::string& str );
		void handleTokens( const Command& cmd, std::ofstream& stream );
		void handleDirectives( const Command& cmd, std::ofstream& stream );
		void toLittleEndian( unsigned short& val );
		bool checkSymbol( const std::string& symbol );
		int getOpcode( const Command& cmd, const std::string& fst );
		int getVector( const std::string& fst );
		int getMask( const std::string& fst );
		void checkOperand( const Command& cmd, const int& operand, const int& bits, const bool& isSigned );
		bool checkRegister( const std::string& reg );
		void registerCheck( const Command& cmd, const std::string& reg );
		void argumentsCheck( const Command& cmd, const std::string& fst, const int& length, const int& n );
		void errorMessage( const Command& cmd, const std::string& err );
};
