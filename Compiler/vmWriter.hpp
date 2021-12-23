#pragma once

#include <fstream>
#include <string>

enum class SEG {
	CONST,
	ARG,
	LOCAL,
	STATIC,
	THIS,
	THAT,
	POINTER,
	TEMP
};

enum class CMD {
	ADD,
	SUB,
	NEG,
	EQ,
	GT,
	LT,
	AND,
	OR,
	NOT
};

class VmWriter {
	std::ofstream _ofs;
public:
	VmWriter(const std::string& outputName);

	void writePush(SEG segment, int idx);

	void writePop(SEG segment, int idx);

	void writeArithmetic(CMD cmd);

	void writeLabel(const std::string& label);

	void writeGoto(const std::string& label);

	void writeIf(const std::string& label);

	void writeCall(const std::string& name, int numArgs);

	void writeFunction(const std::string& name, int numLocals);

	void writeReturn();

	void close();
};