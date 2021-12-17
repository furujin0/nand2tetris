#include <fstream>
#include <iostream>
#include "VmTranslator.hpp"

ArithmeticType toArithmeticType(const std::string& op) {
	return ArithmeticCmdDict.at(op);
}

std::string SegTypeToAsmSymbol(SegType seg) {
	std::string symbol;
	switch (seg)
	{
	case SegType::ARG:
		symbol = "ARG";
		break;
	case SegType::LCL:
		symbol = "LCL";
		break;
	case SegType::THIS:
		symbol = "THIS";
		break;
	case SegType::THAT:
		symbol = "THAT";
		break;
	case SegType::POINTER:
		symbol = "3";
		break;
	case SegType::TEMP:
		symbol = "5";
		break;
	default:
		break;
	}
	return symbol;
}

Parser::Parser(const std::string& path){
	_ifs.open(path);
}

bool Parser::hasMoreCommands() {
	return !_ifs.eof();
}

void Parser::advance() {
	bool is_command = true;
	std::getline(_ifs, _line);
	auto pos = _line.find("//");
	if (pos != decltype(_line)::npos) {
		_line.erase(_line.begin() + pos, _line.end());
	}
	for (auto itr_beg = _line.begin(); itr_beg != _line.end(); itr_beg++) {
		if (*itr_beg == ' ') {
			auto itr_end = itr_beg;
			while (itr_end != _line.end() && *itr_end == ' ') {
				itr_end++;
			}
			_line.replace(itr_beg, itr_end, " ");
		}
	}
	if (!_line.empty() && _line.find(' ') == 0) {
		_line.erase(_line.begin(), _line.begin() + 1);
	}
	if (!_line.empty() && _line.rfind(' ') == _line.size() - 1) {
		_line.erase(_line.end() - 1, _line.end());
	}
	if (_line.empty() && hasMoreCommands()) {
		advance();
	}
}

std::string Parser::currentCommand() {
	return _line;
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
	return CmdType::C_INVALID;
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

CodeWriter::CodeWriter(const std::string& targetName){
	_ofs = std::ofstream(targetName + ".asm");
	_filename = targetName;
}

void CodeWriter::setFileName(const std::string& filename) {
	_input_name = filename;
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
		writeArithmeticOr(); break;
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
		op = "-"; break;
	case ArithmeticType::NOT:
		op = "!"; break;
	default:
		break;
	}
	_ofs
		<< "@SP" << std::endl
		<< "M=M-1" << std::endl //decrement SP
		<< "A=M" << std::endl //get address of the topmost element
		<< "M=" << op << "M" << std::endl //execute operation
		<< "@SP" << std::endl
		<< "M=M+1" << std::endl; //increment SP

	_asm_next_line += 6;
}

void CodeWriter::writeArithmeticTwoOp(const std::string& cmd) {
	std::string op;
	switch (ArithmeticCmdDict.at(cmd))
	{
	case ArithmeticType::ADD:
		op = "+"; break;
	case ArithmeticType::SUB:
		op = "-"; break;
	case ArithmeticType::AND:
		op = "&"; break;
	case ArithmeticType::OR:
		op = "|"; break;
	default:
		break;
	}

	_ofs
		<< "@SP" << std::endl
		<< "M=M-1" << std::endl
		<< "A=M" << std::endl
		<< "D=M" << std::endl
		<< "@SP" << std::endl
		<< "M=M-1" << std::endl
		<< "A=M" << std::endl
		<< "M=M" << op << "D" << std::endl
		<< "@SP" << std::endl
		<< "M=M+1" << std::endl;

	_asm_next_line += 10;
}

void CodeWriter::writeArithmeticCompare(const std::string& cmd) {
	int set_true_line = _asm_next_line + 12;
	int set_false_line = _asm_next_line + 16;
	int return_value_line = _asm_next_line + 20;

	std::string opJump;
	if (cmd == "eq") 
		opJump = "JEQ";
	else if (cmd == "lt")
		opJump = "JGT";
	else if (cmd == "gt")
		opJump = "JLT";

	_ofs
		<< "@SP" << std::endl
		<< "M=M-1" << std::endl // decrement SP, now it has the address of y
		<< "A=M" << std::endl // get address of y
		<< "D=M" << std::endl // get y
		<< "@SP" << std::endl
		<< "M=M-1" << std::endl // decrement SP, now it has the address of x
		<< "A=M" << std::endl // get address of y 
		<< "D=D-M" << std::endl //put y-x to D
		<< "@" << set_true_line << std::endl
		<< "D;" << opJump << std::endl
		<< "@" << set_false_line << std::endl
		<< "0;JMP" << std::endl
		<< "@0" << std::endl
		<< "D=!A" << std::endl //put 'true' to D
		<< "@" << return_value_line << std::endl
		<< "0;JMP" << std::endl
		<< "@0" << std::endl
		<< "D=A" << std::endl // put 'false' to D
		<< "@" << return_value_line << std::endl
		<< "0;JMP" << std::endl
		<< "@SP" << std::endl
		<< "A=M" << std::endl // get the address of return value
		<< "M=D" << std::endl
		<< "@SP" << std::endl
		<< "M=M+1" << std::endl; //increment SP
	_asm_next_line += 25;
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
		writePush(seg, idx);
		break;
	case CmdType::C_POP:
		writePop(seg, idx);
		break;
	default:
		break;
	}
}

void CodeWriter::writePush(const std::string& seg, int idx) {
	auto seg_type = SegDict.at(seg);
	switch (seg_type) {
	case SegType::CONST:
		writePushConst(idx); break;
	case SegType::STATIC:
		writePushStatic(idx); break;
	case SegType::POINTER:
		writePushPointer(idx); break;
	case SegType::TEMP:
		writePushTemp(idx); break;
	default:
		writePushNonStatic(seg_type, idx); break;
	}
}

void CodeWriter::writePop(const std::string& seg, int idx) {
	auto seg_type = SegDict.at(seg);
	switch (seg_type) {
	case SegType::CONST:
		writePopConst(); break;
	case SegType::STATIC:
		writePopStatic(idx); break;
	case SegType::POINTER:
		writePopPointer(idx); break;
	case SegType::TEMP:
		writePopTemp(idx); break;
	default:
		writePopNonStatic(seg_type, idx); break;
	}
}

void CodeWriter::writePushNonStatic(SegType seg_type, int idx) {
	std::string segment = SegTypeToAsmSymbol(seg_type);
	_ofs << "@" << segment << std::endl
		<< "D=M" << std::endl // get the address of segment[0]
		<< "@" << idx << std::endl
		<< "A=D+A" << std::endl // get the address of segment[idx]
		<< "D=M" << std::endl // get the value of segment[idx]
		<< "@SP" << std::endl
		<< "A=M" << std::endl // get the address to put the value
		<< "M=D" << std::endl
		<< "@SP" << std::endl
		<< "M=M+1" << std::endl;
	;
	_asm_next_line += 10;
}

void CodeWriter::writePushTemp(int idx) {
	_ofs 
		<< "@" << idx + 5 << std::endl
		<< "D=M" << std::endl // get the value of segment[idx]
		<< "@SP" << std::endl
		<< "A=M" << std::endl // get the address to put the value
		<< "M=D" << std::endl
		<< "@SP" << std::endl
		<< "M=M+1" << std::endl;
	_asm_next_line += 7;
}

void CodeWriter::writePushPointer(int idx) {
	_ofs
		<< "@" << idx + 3 << std::endl
		<< "D=M" << std::endl // get the value of segment[idx]
		<< "@SP" << std::endl
		<< "A=M" << std::endl // get the address to put the value
		<< "M=D" << std::endl
		<< "@SP" << std::endl
		<< "M=M+1" << std::endl;
	;
	_asm_next_line += 7;

}

void CodeWriter::writePushStatic(int idx) {
	_ofs
		<< "@" << _input_name << "." << idx << std::endl
		<< "D=M" << std::endl // get static object
		<< "@SP" << std::endl //
		<< "A=M" << std::endl // get the address to put the value
		<< "M=D" << std::endl
		<< "@SP" << std::endl
		<< "M=M+1" << std::endl;

	_asm_next_line += 7;
}

void CodeWriter::writePushConst(int value) {
	_ofs
		<< "@" << value << std::endl
		<< "D=A" << std::endl
		<< "@SP" << std::endl
		<< "A=M" << std::endl
		<< "M=D" << std::endl
		<< "@SP" << std::endl
		<< "M=M+1" << std::endl;

	_asm_next_line += 7;
}

void CodeWriter::writePopNonStatic(SegType seg_type, int idx) {
	std::string segment = SegTypeToAsmSymbol(seg_type);
	_ofs << "@" << segment << std::endl
		<< "D=M" << std::endl //get the address of segment[0]
		<< "@" << idx << std::endl
		<< "D=D+A" << std::endl //get the address of segment[idx]
		<< "@R13" << std::endl // store the address of segment[idx] ot R13
		<< "M=D" << std::endl
		<< "@SP" << std::endl
		<< "M=M-1" << std::endl
		<< "A=M" << std::endl
		<< "D=M" << std::endl //get the topmost value of the stack
		<< "@R13" << std::endl
		<< "A=M" << std::endl
		<< "M=D" << std::endl; //set the popped value
	_asm_next_line += 13;
}

void CodeWriter::writePopTemp(int idx) {
	_ofs
		<< "@SP" << std::endl
		<< "M=M-1" << std::endl
		<< "A=M" << std::endl
		<< "D=M" << std::endl //get the topmost value of the stack
		<< "@" << idx + 5 << std::endl
		<< "M=D" << std::endl; //set the popped value
	_asm_next_line += 6;
}

void CodeWriter::writePopPointer(int idx) {
	_ofs 
		<< "@SP" << std::endl
		<< "M=M-1" << std::endl
		<< "A=M" << std::endl
		<< "D=M" << std::endl //get the topmost value of the stack
		<< "@" << idx + 3 << std::endl
		<< "M=D" << std::endl; //set the popped value
	_asm_next_line += 6;
}

void CodeWriter::writePopStatic(int idx)
{
	_ofs
		<< "@SP" << std::endl
		<< "M=M-1" << std::endl
		<< "A=M" << std::endl
		<< "D=M" << std::endl // get the topmost value of the stack
		<< "@" << _input_name << "." << idx << std::endl
		<< "M=D" << std::endl; //set the value to the static object

	_asm_next_line += 6;
}

void CodeWriter::writePopConst() {
	_ofs << "@SP" << std::endl
		<< "M=M-1" << std::endl;
	_asm_next_line += 2;
}

void CodeWriter::writeInit() {
	_ofs 
		<< "@256" << std::endl
		<< "D=A" << std::endl
		<< "@SP" << std::endl
		<< "M=D" << std::endl;
	_asm_next_line += 4;
	writeCall("Sys.init", 0);
}

void CodeWriter::writeLabel(const std::string& label) {
	_ofs << "(" << _current_func << "$" << label << ")" << std::endl;
}

void CodeWriter::writeGoto(const std::string& label) {
	_ofs << "@" << _current_func << "$" << label << std::endl
		<< "0;JMP" << std::endl;
	_asm_next_line += 2;
}

void CodeWriter::writeIf(const std::string& label) {
	_ofs << "@SP" << std::endl
		<< "M=M-1" << std::endl
		<< "A=M" << std::endl
		<< "D=M" << std::endl //store value to evaluate into D
		<< "@" << _current_func << "$" << label << std::endl
		<< "D;JNE" << std::endl; //if non-zero then jump
	_asm_next_line += 6;
}

void CodeWriter::writeCall(const std::string& funcName, int numArgs) {

	int return_address = 43 + _asm_next_line;
	//push return address
	_ofs
		<< "@" << return_address << std::endl
		<< "D=A" << std::endl
		<< "@SP" << std::endl
		<< "A=M" << std::endl
		<< "M=D" << std::endl // push return address
		<< "@SP" << std::endl
		<< "M=M+1" << std::endl;
	_asm_next_line += 7;

	// push lcl/arg/this/that of the caller
	for (auto& symbol : std::vector<std::string>{ "LCL", "ARG", "THIS", "THAT" }) {
		_ofs << "@" << symbol<< std::endl
			<< "D=M" << std::endl
			<< "@SP" << std::endl
			<< "A=M" << std::endl
			<< "M=D" << std::endl // push [symbol] address of caller func
			<< "@SP" << std::endl
			<< "M=M+1" << std::endl;
		_asm_next_line += 7;
	}

	//update ARG address
	_ofs << "@SP" << std::endl
		<< "D=M" << std::endl
		<< "@" << 5 + numArgs << std::endl
		<< "D=D-A" << std::endl
		<< "@ARG" << std::endl
		<< "M=D" << std::endl;
	_asm_next_line += 6;

	_ofs << "@" << funcName << std::endl
		<< "0;JMP" << std::endl;
	_asm_next_line += 2;
}

void CodeWriter::writeReturn() {
	//save the return value
	_ofs << "@SP" << std::endl
		<< "M=M-1" << std::endl
		<< "A=M" << std::endl
		<< "D=M" << std::endl //store return value to D
		<< "@R15" << std::endl
		<< "M=D" << std::endl; // return value to R15
	_asm_next_line += 6;

	//move SP to LCL
	_ofs << "@LCL" << std::endl
		<< "D=M" << std::endl
		<< "@SP" << std::endl
		<< "M=D" << std::endl;
	_asm_next_line += 4;

	writePopPointer(1); //update THIS
	writePopPointer(0); //update THAT
	
	//save new SP location
	_ofs << "@ARG" << std::endl
		<< "D=M" << std::endl
		<< "@R13" << std::endl
		<< "M=D+1" << std::endl;
	_asm_next_line += 4;
	//pop ARG address of caller and store it to ARG register
	_ofs << "@SP" << std::endl
		<< "M=M-1" << std::endl
		<< "A=M" << std::endl
		<< "D=M" << std::endl // store the caller's ARG address to D register
		<< "@ARG" << std::endl
		<< "M=D" << std::endl // update ARG address
		<< "@SP" << std::endl
		<< "M=M-1" << std::endl //decrement SP, now points LCL
		<< "A=M" << std::endl
		<< "D=M" << std::endl // store the callers's LCL address to D register
		<< "@LCL" << std::endl
		<< "M=D" << std::endl; // update LCL address
	_asm_next_line += 12;
	//store return address to R14
	_ofs
		<< "@SP" << std::endl
		<< "A=M-1" << std::endl //now A is the addr of the return addr
		<< "D=M" << std::endl // now D is the return addr
		<< "@R14" << std::endl
		<< "M=D" << std::endl;
	_asm_next_line += 5;


	//update SP location
	_ofs
		<< "@R13" << std::endl
		<< "D=M" << std::endl
		<< "@SP" << std::endl
		<< "M=D" << std::endl;
	_asm_next_line += 4;

	//set return value
	_ofs
		<< "@R15" << std::endl
		<< "D=M" << std::endl
		<< "@SP" << std::endl
		<< "A=M-1" << std::endl
		<< "M=D" << std::endl;
	_asm_next_line += 5;

	//jump
	_ofs
		<< "@R14" << std::endl
		<< "A=M" << std::endl
		<< "0;JMP" << std::endl; // go to return addr
	_asm_next_line += 3;
}

void CodeWriter::writeFunction(const std::string& funcName, int numLocals) {
	_current_func = funcName;
	_num_locals = numLocals;
	_ofs << "(" << funcName << ")" << std::endl;
	//update LCL address
	_ofs << "@SP" << std::endl
		<< "D=M" << std::endl
		<< "@LCL" << std::endl
		<< "M=D" << std::endl;
	_asm_next_line += 4;
	for (int i = 0; i < numLocals; i++) {
		writePushConst(0);
	}
}

void CodeWriter::close() {
	_ofs.close();
}