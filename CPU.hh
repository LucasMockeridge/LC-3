enum
{
	MEM_SIZE = 65536,
	GPR_COUNT = 8
};

class Memory
{
	public:
		unsigned short& operator[]( const unsigned short addr );
	private:
		unsigned short mem[MEM_SIZE];
		bool checkSTDIN();
};

class CPU
{
	public:
		CPU();
		unsigned short fetchInstr(); 
		void handleInstr( const unsigned short instr );
		void halt();
		bool isHalted();
		void loadPrograms( int argc, char* argv[] );
	private:
		bool halted;
		Memory mem;
		unsigned short GPR[GPR_COUNT];
		unsigned short PC, PSR;
		void handleTrap( const unsigned short instr );
		unsigned short sext( unsigned short val, const int len );
		void setcc( const unsigned short val );
		void loadProgram( const char* filePath ); 
		unsigned short toLittleEndian( unsigned short val );
};
