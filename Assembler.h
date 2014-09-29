#pragma once
#include <string>
#include <map>


using namespace std;

enum InstructionType
{
	A,
	B,
	C
};

struct Instruction
{
	int opcode;
	InstructionType type;
	int format;
	int rd;
	int ra;
	int rb;
	int imm5;
	int imm8;
	bool invalid;
	short data;
	int skipAmount;
};

class Assembler
{
public:
	static map<string, int> opcodes;
	static map<string, int> specialInstructions;
	static map<string, int> registers;
	map<string, int> labels;
	bool useHProtection;
	bool isParsingData;
	const bool createStack;

	Assembler(bool useHProtection = false, bool createStack = false) : useHProtection(useHProtection), isParsingData(false), createStack(createStack) {}
	std::string Assemble(const std::string& input, std::string& errorText);
	static std::string GenerateStackCode();

private:
	Instruction parseLine(const string& line, int currentLine, string& errorText);
	bool findLabels(string input, string& errorText);

	static string formatOutput(string s);
	static string createAssembledLine(const Instruction& instruction);
};