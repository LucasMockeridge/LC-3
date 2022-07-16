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
		std::vector<std::vector<std::string>> tokens;
		std::map<std::string, int> symbolTable;
		std::istream& getCommand( std::istream& stream, std::string& str );
		bool isNotWhiteSpace( const std::string& str );
		std::vector<std::string> makeTokens( const std::string& str ); 
		void checkLiteral( const std::string& s );
		void checkOrig();
		void checkEnd();
		void buildTable();
		int convertNumber( const std::string& str );
		void handleTokens( const int& i, const std::vector<std::string>& tokens, std::ofstream& stream );
		void handleDirectives( const std::vector<std::string>& tokens, std::ofstream& stream );
		void toLittleEndian( unsigned short& val );
		bool checkSymbol( const std::string& symbol );
		int getOpcode( const std::string& cmd );
		int getVector( const std::string& cmd );
		int getMask( const std::string& cmd );
		void checkOperand( const int& operand, const int& bits, const bool& isSigned );
		bool checkRegister( const std::string& reg );
		void registerCheck( const std::string& reg );
		void argumentsCheck( const std::string& cmd, const int& length, const int& n );
};
