#pragma once

#include <fstream>
#include <string>
#include <vector>

enum class CmdType {
	C_ARITHMETIC,
	C_PUSH,
	C_POP,
	C_LABEL,
	C_GOTO,
	C_IF,
	C_FUNCTION,
	C_RETURN,
	C_CALL
};

const std::vector<std::string> arithmetic_cmds{
	"add",
	"sub",
	"neg",
	"eq",
	"gt",
	"lt",
	"and",
	"or",
	"not"
};

class Parser {
private:
	std::string _line;
	std::ifstream _ifs;
	
public:
	Parser(const std::string& filename);

	bool hasMoreCommands();

	void advance();

	CmdType commandType();

	std::string arg1();

	int arg2();

};

class CodeWriter {
private:
	std::ofstream _ofs;
	int _asm_next_line = 0;

public:
	CodeWriter();

	void setFileName(const std::string& filename);

	void writeArithmetic(const std::string& cmd);

	void writePushPop(const std::string& cmd, const std::string& seg, int idx);

	void close();

	void writeArithmeticNeg();

	void writeArithmeticNot();

	void writeArithmeticAdd();

	void writeArithmeticSub();

	void writeArithmeticAnd();

	void writeArithmeticOr();

	void writeArithmeticEq();

	void writeArithmeticGt();

	void writeArithmeticLt();
};