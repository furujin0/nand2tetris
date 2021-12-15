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
	for (auto&& cmd : arithmetic_cmds) {
		if (_line.find(cmd) == 0) {
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
	_ofs << "@SP" << std::endl; //set stack pointer to the topmost value
	if (cmd == "neg") {
		_ofs << "M=-M" << std::endl;
		return;
	}
	else if (cmd == "not") {
		_ofs << "M=!M" << std::endl;
		return;
	}
	_ofs << "D=M" << std::endl; // copy the topmost value

	_ofs << "A=A-1" << std::endl;
	_ofs << "R0=A" << std::endl; // update sp (now sp points to the result)

	//output arithmetic or logical operation with two args
	if (cmd == "add")
		_ofs << "M=D+M";
	else if (cmd == "sub")
		_ofs << "M=D-M";
	else if (cmd == "eq") {
		_ofs << "D=D-M" << std::endl;
		_ofs << "@" << std::endl;
		_ofs << "D=D&A" << std::endl;
		_ofs << "";

	}
	else if (cmd == "gt")
		_ofs << "";
	else if (cmd == "lt")
		_ofs << "";
	else if (cmd == "and")
		_ofs << "D&M";
	else if (cmd == "or")
		_ofs << "D|M";

}

void CodeWriter::writeArithmeticEq() {
	int set_true_line = _asm_next_line + 10;
	int set_false_line = _asm_next_line + 14;
	int return_value_line = _asm_next_line + 19;
	_ofs
		<< "@SP" << std::endl
		<< "D=A" << std::endl
		<< "A=A-1" << std::endl
		<< "@R0" << std::endl
		<< "M=A" << std::endl
		<< "D=D-M" << std::endl
		<< "@" << set_true_line << std::endl
		<< "D;JEQ" << std::endl
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

void CodeWriter::writePushPop(const std::string& cmd, const std::string& seg, int idx) {

}