#include "Assembler.h"
#include <iostream>
#include <vector>
#include <sstream>

using namespace std;


string Assembler::formatOutput(string s)
{
	while (s.length() < 4)
	{
		s = "0" + s;
	}

	return s;
}

string Assembler::createAssembledLine(const Instruction& instruction)
{
	int output = (instruction.opcode << 12) + (instruction.format << 11) + (instruction.rd << 8);
	switch (instruction.type)
	{
	case A:
		output += instruction.ra << 5;
		output += instruction.rb;
		break;

	case B:
		output += instruction.ra << 5;
		output += instruction.imm5 & 0x1f;
		break;

	case C:
		output += instruction.imm8 & 0xff;
		break;
	}

	char outputs[256];
#ifdef _WIN32
	sprintf_s(outputs, "%x", output);
#else
	sprintf(outputs, "%x", output);
#endif
	return string(outputs);
}

Instruction Assembler::parseLine(const string& line, int currentLine, string& errorText)
{
	Instruction parsedInstruction;
	parsedInstruction.invalid = false;
	
	//get opcode
	stringstream lineStream(line);

	string instructionName;
	lineStream >> instructionName;

	const int hasComment = instructionName.find("//");
	if (hasComment >= 0)
	{
		parsedInstruction.invalid = true;
		return parsedInstruction;
	}

	const int hasPeriod = instructionName.find(".");
	if (hasPeriod >= 0)
	{
		parsedInstruction.invalid = true;

		//check if data
		if (instructionName == ".data")
		{
			this->isParsingData = true;
		}
		else if (instructionName == ".code")
		{
			this->isParsingData = false;
		}
		else if (instructionName == ".skip")
		{
			lineStream >> parsedInstruction.skipAmount;
		}

		return parsedInstruction;
	}

	if (this->isParsingData)
	{
		const int isHex = line.find("0x");
		if (isHex < 0)
		{
			stringstream nStream(line);
			nStream >> parsedInstruction.data;
		}
		else
		{
			stringstream nStream(line.substr(2));
			int res = 0;
			nStream >> std::hex >> res;
			parsedInstruction.data = static_cast<short>(res);
			/*if (line[2] < 57) 
			{

			}
			else //hex character
			{
				string fString = line.substr(2);
				fString[0] = 0;
				stringstream nStream(fString);
				nStream >> std::hex >> parsedInstruction.data + ((line[0] - a) + 10;
			}*/
			
		}
		return parsedInstruction;
	}

	const auto opcode = this->opcodes.find(instructionName);
	if (opcode == this->opcodes.end())
	{
		parsedInstruction.invalid = true;
		const int ind = instructionName.find(':');
		if (ind < 0)
		{ 
			//is not a label
			errorText += "ERROR: Invalid opcode " + instructionName + " at line " + to_string(currentLine) + "\n";
		}

		return parsedInstruction;
	}
	parsedInstruction.opcode = opcode->second;

	string rd;
	lineStream >> rd;
	parsedInstruction.rd = registers[rd];

	string registerOrLabelOrConstant;

	lineStream >> registerOrLabelOrConstant;

	//check if register
	const auto registerA = registers.find(registerOrLabelOrConstant);
	if (registerA != registers.end())
	{
		parsedInstruction.ra = registerA->second;
	}
	else
	{
		parsedInstruction.type = C;
		parsedInstruction.format = 1;

		//check if label
		const auto label = labels.find(registerOrLabelOrConstant);
		if (label != labels.end())
		{
			parsedInstruction.imm8 = label->second - currentLine - 1;
		}
		else
		{
			//re-read the token as a number
			stringstream nStream(registerOrLabelOrConstant);
			nStream >> parsedInstruction.imm8;
		}

		if (parsedInstruction.imm8 > 128 - 1 || parsedInstruction.imm8 < -128)
		{
			errorText += "Error: value " + to_string(parsedInstruction.imm8) + " for imm8 at line " + to_string(currentLine) + " does not fit in 8 bits";
			parsedInstruction.invalid = true;
		}

		return parsedInstruction;
	}

	lineStream >> registerOrLabelOrConstant;

	//check if register b
	const auto registerB = registers.find(registerOrLabelOrConstant);

	if (registerB != registers.end())
	{
		parsedInstruction.rb = registerB->second;
		parsedInstruction.type = A;
		parsedInstruction.format = 1;
	}
	else
	{
		parsedInstruction.type = B;
		parsedInstruction.format = 0;

		//check if label
		const auto label = labels.find(registerOrLabelOrConstant);
		if (label != labels.end())
		{
			parsedInstruction.imm5 = label->second - currentLine - 1;
		}
		else
		{
			//re-read the token as a number
			stringstream nStream(registerOrLabelOrConstant);
			nStream >> parsedInstruction.imm5;
		}

		if (parsedInstruction.imm5 > 16 - 1 || parsedInstruction.imm5 < -16)
		{
			errorText += "Error: value " + to_string(parsedInstruction.imm5) + " for imm5 at line " + to_string(currentLine) + " does not fit in 5 bits";
			parsedInstruction.invalid = true;
		}
	}

	return parsedInstruction;
}

bool Assembler::findLabels(string input, string& errorText)
{
	stringstream inp(input);

	int labelCount = 0;
	int lineCount = 0;
	while (!inp.eof())
	{
		string nextLine;
		getline(inp, nextLine);

		if (nextLine.length() == 0)
		{
			continue;
		}

		const int ind = nextLine.find(':');
		if (ind >= 0)
		{
			const string newLabel = nextLine.substr(0, nextLine.size() - 1);
			const int hasH = newLabel.find('h');
			if (this->useHProtection && hasH > -1)
			{
				errorText += "ERROR: label '" + newLabel + "' contains the letter 'h'.\n";
				return false; 
			}
			this->labels[newLabel] = lineCount;
		}
		else
		{
			const int skipInd = nextLine.find(".skip");

			if (skipInd >= 0)
			{
				stringstream inpl(nextLine);
				string s;
				inpl >> s;

				int skipAmount = 0;
				inpl >> skipAmount;

				lineCount += skipAmount;
			}
			else
			{
				++lineCount;
			}
		}
	}

	return true;
}

string Assembler::GenerateStackCode()
{
	string stackCode;
	stackCode += "lea r6 stackBegin //init stack ptr\n";
	stackCode += "\n";
	stackCode += "//start program code here\n";
	stackCode += "\n";
	stackCode += "\n";
	stackCode += "\n";
	stackCode += ".skip 64 //stack grows down\n";
	stackCode += "stackBegin:\n";


	return stackCode;
}

map<string, int> Assembler::opcodes = { { "add", 0x0 }, { "slt", 0x2 }, { "lea", 0xc }, { "call", 0xd }, { "brz", 0xf }, { "ld", 0xa }, { "st", 0xb } };
map<string, int> Assembler::registers = { { "r0", 0x0 }, { "r1", 0x1 }, { "r2", 0x2 }, { "r3", 0x3 }, { "r4", 0x4 }, { "r5", 0x5 }, { "r6", 0x6 }, { "r7", 0x7 } };

string Assembler::Assemble(const string& input, string& errorText)
{	
	findLabels(input, errorText);
	if (errorText != "")
	{
		return "";
	}

	vector<string> outputLines;

	stringstream inp(input);

	while (!inp.eof())
	{
		string nextLine;
		getline(inp, nextLine);

		if (nextLine.length() == 0 || nextLine[0] == 0)
		{
			continue;
		}

		const Instruction nextInstruction = parseLine(nextLine, outputLines.size(), errorText);

		if (!nextInstruction.invalid && !this->isParsingData)
			outputLines.push_back(createAssembledLine(nextInstruction));
		else if (nextInstruction.invalid && nextInstruction.skipAmount != 0)
		{
			for (int a = 0; a < nextInstruction.skipAmount; ++a)
			{
				outputLines.push_back("0000");
			}
		}
		if (this->isParsingData && !nextInstruction.invalid)
		{
			char outputs[256];
#ifdef _WIN32
			sprintf_s(outputs, "%x", static_cast<short>(nextInstruction.data));
#else
			sprintf(outputs, "%x", static_cast<short>(nextInstruction.data));
#endif
			string output(outputs);
			if (output.length() == 8)
			{
				output = output.substr(4);
			}

			outputLines.push_back(output);
		}
	}

	stringstream outputFile;

	outputFile << "WIDTH=16;" << endl;
	outputFile << "DEPTH=" << outputLines.size() << ";" << endl << endl;

	outputFile << "ADDRESS_RADIX=DEC;" << endl;
	outputFile << "DATA_RADIX=HEX;" << endl << endl;
	outputFile << "CONTENT BEGIN" << endl;

	int lineNumber = 0;

	for (int a = 0; a < outputLines.size(); ++a)
	{
		outputFile << lineNumber++ << ": " << formatOutput(outputLines[a]) << ";" << endl;
	}

	outputFile << "END;" << endl;

	return outputFile.str();
}