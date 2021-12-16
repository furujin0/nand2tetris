#pragma once

#include <fstream>
#include <string>
#include <unordered_map>
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
	C_CALL,
	C_INVALID
};

enum class SegType {
	ARG,
	LCL,
	STATIC,
	CONST,
	THIS,
	THAT,
	POINTER,
	TEMP
};

enum class ArithmeticType {
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

const std::unordered_map<std::string, ArithmeticType> ArithmeticCmdDict{
	std::make_pair("add", ArithmeticType::ADD),
	std::make_pair("sub", ArithmeticType::SUB),
	std::make_pair("neg", ArithmeticType::NEG),
	std::make_pair("eq", ArithmeticType::EQ),
	std::make_pair("gt", ArithmeticType::GT),
	std::make_pair("lt", ArithmeticType::LT),
	std::make_pair("and", ArithmeticType::AND),
	std::make_pair("or", ArithmeticType::OR),
	std::make_pair("not", ArithmeticType::NOT)
};


const std::unordered_map<std::string, SegType> SegDict{
	std::make_pair("argument", SegType::ARG),
	std::make_pair("local", SegType::LCL),
	std::make_pair("static", SegType::STATIC),
	std::make_pair("this", SegType::THIS),
	std::make_pair("that", SegType::THAT),
	std::make_pair("pointer", SegType::POINTER),
	std::make_pair("temp", SegType::TEMP),
	std::make_pair("constant", SegType::CONST)
};

ArithmeticType toArithmeticType(const std::string& op);

std::string SegTypeToAsmSymbol(SegType seg);


class Parser {
private:
	std::string _line;
	std::ifstream _ifs;
	
public:
	Parser(const std::string& path);

	bool hasMoreCommands();

	void advance();

	CmdType commandType();

	std::string arg1();

	int arg2();

	std::string currentCommand();
};

class CodeWriter {
private:
	std::ofstream _ofs;
	int _asm_next_line = 0;
	std::string _filename;

public:
	CodeWriter();

	void setFileName(const std::string& filename);

	void writeArithmetic(const std::string& cmd);

	void writePushPop(CmdType cmd, const std::string& seg, int idx);

	void writePush(const std::string& seg, int idx);

	void writePop(const std::string& seg, int idx);

	void close();

	void writeArithmeticCompare(const std::string& cmd);

	void writeArithmeticOneOp(const std::string& cmd);

	void writeArithmeticTwoOp(const std::string& cmd);

	void writeArithmeticNeg();

	void writeArithmeticNot();

	void writeArithmeticAdd();

	void writeArithmeticSub();

	void writeArithmeticAnd();

	void writeArithmeticOr();

	void writeArithmeticEq();

	void writeArithmeticGt();

	void writeArithmeticLt();

	void writePushNonStatic(SegType seg_type, int idx);
	
	void writePushTemp(int idx);

	void writePushPointer(int idx);

	void writePushStatic(int idx);

	void writePushConst(int value);

	void writePopNonStatic(SegType seg_type, int idx);

	void writePopTemp(int idx);

	void writePopPointer(int idx);

	void writePopStatic(int idx);

	void writePopConst();

};