#pragma once
#include <string>
#include <map>

enum InstructionType
{
	A,
	B,
	C
};

enum SpecialOpcodes
{
	callFunc,
	ret,
	push,
	pop
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
private:
	static std::map<std::string, int> opcodes;
	static std::map<std::string, SpecialOpcodes> specialInstructions;
	static std::map<std::string, int> registers;
	std::map<std::string, int> labels;
	bool useHProtection;
	bool isParsingData;
	const bool createStack;
	std::string errorText;
	std::string assemblyCode;
	bool hasExpandedCode;

public:
	Assembler(const std::string& assemblyCode, bool useHProtection = false, bool createStack = false) :
		useHProtection(useHProtection), isParsingData(false), createStack(createStack), assemblyCode(assemblyCode), hasExpandedCode(false) {}

	std::string Assemble();
	static std::string GenerateStackCode();

	std::string GetErrorText() const { return this->errorText; }
	std::string GetAssemblyCode() const { return this->assemblyCode; }
	bool HasExpandedCode() const { return this->hasExpandedCode; }

private:
	void prepareInput();
	bool findLabels(std::string input);
	void expandSpecialInstructions();
	std::string expandSpecialInstruction(const std::string& instruction);
	int readImmediate(const std::string& token, int lineNumber) const;
	Instruction parseLine(const std::string& line, int currentLine);

	static std::string formatOutput(std::string s);
	static std::string createAssembledLine(const Instruction& instruction);
	static bool isCommented(const std::string& s);
	//returns -1 if the token is not a register
	static int readRegister(const std::string& token);
};