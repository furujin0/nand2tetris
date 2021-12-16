#include <fstream>
#include "VmTranslator.hpp"

Parser::Parser(const std::string& filename):_ifs(filename + ".vm") {};

bool Parser::hasMoreCommands() {
	return !_ifs.eof();
}

void Parser::advance() {
	std::getline(_ifs, _line);
	bool is_command = true;

	//skip comment
	auto pos = _line.find("//");
	if (pos != decltype(_line)::npos) {
		is_command = false;
	}

	//reduce spaces
	for (auto itr_beg = _line.begin(); itr_beg != _line.end(); itr_beg++) {
		if (*itr_beg = ' ') {
			auto itr_end = itr_beg;
			while (itr_end != _line.end() || *itr_end != ' ') {
				itr_end++;
			}
			_line.replace(itr_beg, itr_end, " ");
		}
	}
	//erase spaces before command
	if (_line.find(' ') == 0) {
		_line.erase(_line.begin());
	}
	if (_line.find(' ') == _line.size() - 1);
	{
		_line.erase(_line.end() - 1);
	}

	if (_line.empty()) {
		is_command = false;
	}

	if (!is_command) {
		advance();
	}
}

CmdType Parser::commandType()
{
	for (auto&& cmd : ArithmeticCmdDict) {
		if (_line.find(cmd.first) == 0) {
			return CmdType::C_ARITHMETIC;
		}
	}
	if (_line.find("pop") == 0) {
		return CmdType::C_POP;
	}
	else if (_line.find("push") == 0) {
		return CmdType::C_PUSH;
	}
	else if (_line.find("label") == 0) {
		return CmdType::C_LABEL;
	}
	else if (_line.find("goto") == 0) {
		return CmdType::C_GOTO;
	}
	else if (_line.find("if-goto") == 0) {
		return CmdType::C_IF;
	}
	else if (_line.find("function") == 0) {
		return CmdType::C_FUNCTION;
	}
	else if (_line.find("call") == 0) {
		return CmdType::C_CALL;
	}
	else if (_line.find("return") == 0) {
		return CmdType::C_RETURN;
	}
}

std::string Parser::arg1() {
	if (commandType() == CmdType::C_ARITHMETIC) {
		return std::string(_line.begin(), _line.end());
	}
	auto pos_beg = _line.find(' ');
	auto pos_end = _line.find(' ', pos_beg + 1);
	if (pos_end == decltype(_line)::npos) {
		pos_end = _line.size();
	}
	return std::string(_line.begin() + pos_beg + 1, _line.begin() + pos_end);
}

int Parser::arg2() {
	auto pos_beg = _line.rfind(' ');
	return std::stoi(std::string(_line.begin() + pos_beg, _line.end()));
}

CodeWriter::CodeWriter(){}

void CodeWriter::setFileName(const std::string& filename) {
	_ofs = std::ofstream(filename + ".asm");
}


void CodeWriter::writeArithmetic(const std::string& cmd) {
	switch (toArithmeticType(cmd)) {
	case ArithmeticType::ADD:
		writeArithmeticAdd(); break;
	case ArithmeticType::SUB:
		writeArithmeticSub(); break;
	case ArithmeticType::NEG:
		writeArithmeticNeg(); break;
	case ArithmeticType::EQ:
		writeArithmeticEq(); break;
	case ArithmeticType::LT:
		writeArithmeticLt(); break;
	case ArithmeticType::GT:
		writeArithmeticGt(); break;
	case ArithmeticType::AND:
		writeArithmeticAnd(); break;
	case ArithmeticType::OR:
		writeArithmeticOr();
	case ArithmeticType::NOT:
		writeArithmeticNot(); break;
	default:
		break;
	}

}

void CodeWriter::writeArithmeticOneOp(const std::string& cmd) {
	std::string op;
	switch (ArithmeticCmdDict.at(cmd))
	{
	case ArithmeticType::NEG:
		op = "-";
	case ArithmeticType::NOT:
		op = "!";
	default:
		break;
	}
	_ofs
		<< "@SP" << std::endl
		<< "M=" << op << "M" << std::endl;
	_asm_next_line += 2;
}

void CodeWriter::writeArithmeticTwoOp(const std::string& cmd) {
	std::string op;
	switch (ArithmeticCmdDict.at(cmd))
	{
	case ArithmeticType::ADD:
		op = "+";
	case ArithmeticType::SUB:
		op = "-";
	case ArithmeticType::AND:
		op = "&";
	case ArithmeticType::OR:
		op = "|";
	default:
		break;
	}

	_ofs
		<< "@SP" << std::endl
		<< "D=M" << std::endl
		<< "A=A-1" << std::endl
		<< "M=D" << op << "M" << std::endl
		<< "@0" << std::endl
		<< "M=A" << std::endl; //update stack pointer

	_asm_next_line += 6;
}

void CodeWriter::writeArithmeticCompare(const std::string& cmd) {
	int set_true_line = _asm_next_line + 10;
	int set_false_line = _asm_next_line + 14;
	int return_value_line = _asm_next_line + 19;

	std::string opJump;
	if (cmd == "eq") 
		opJump = "JEQ";
	else if (cmd == "lt")
		opJump = "JGT";
	else if (cmd == "gt")
		opJump = "JLT";

	_ofs
		<< "@SP" << std::endl
		<< "D=A" << std::endl
		<< "A=A-1" << std::endl
		<< "@0" << std::endl
		<< "M=A" << std::endl // update stack pointer
		<< "D=D-M" << std::endl
		<< "@" << set_true_line << std::endl
		<< "D;" << opJump << std::endl
		<< "@" << set_false_line << std::endl
		<< "0;JMP" << std::endl
		<< "@65535" << std::endl
		<< "D=A" << std::endl
		<< "@" << return_value_line << std::endl
		<< "0;JMP" << std::endl
		<< "@0" << std::endl
		<< "D=A" << std::endl
		<< "@" << return_value_line << std::endl
		<< "0;JMP" << std::endl
		<< "@SP" << std::endl
		<< "M=D" << std::endl;
	_asm_next_line += 20;
}

void CodeWriter::writeArithmeticAdd() {
	writeArithmeticTwoOp("add");
}

void CodeWriter::writeArithmeticSub() {
	writeArithmeticTwoOp("sub");
};

void CodeWriter::writeArithmeticNeg() {
	writeArithmeticOneOp("neg");
}

void CodeWriter::writeArithmeticEq() {
	writeArithmeticCompare("eq");
}

void CodeWriter::writeArithmeticGt() {
	writeArithmeticCompare("gt");
}

void CodeWriter::writeArithmeticLt() {
	writeArithmeticCompare("lt");
}

void CodeWriter::writeArithmeticAnd() {
	writeArithmeticTwoOp("and");
}

void CodeWriter::writeArithmeticOr() {
	writeArithmeticTwoOp("or");
}

void CodeWriter::writeArithmeticNot() {
	writeArithmeticOneOp("not");
}

void CodeWriter::writePushPop(CmdType cmd, const std::string& seg, int idx) {
	switch (cmd) {
	case CmdType::C_PUSH:

	}
}