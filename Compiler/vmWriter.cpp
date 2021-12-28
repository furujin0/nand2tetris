#include "vmWriter.hpp"

VmWriter::VmWriter(
	const std::string& outputName
) {
	_ofs.open(outputName);
}

void VmWriter::writePush(SEG segment, int idx) {
	_ofs << "push ";
	switch (segment) {
	case SEG::CONST:
		_ofs << "constant "; break;
	case SEG::ARG:
		_ofs << "argument "; break;
	case SEG::LOCAL:
		_ofs << "local "; break;
	case SEG::STATIC:
		_ofs << "static "; break;
	case SEG::THIS:
		_ofs << "this "; break;
	case SEG::THAT:
		_ofs << "that "; break;
	case SEG::POINTER:
		_ofs << "pointer "; break;
	case SEG::TEMP:
		_ofs << "temp "; break;
	default:
		break;
	}
	_ofs << idx << std::endl;
}

void VmWriter::writePop(SEG segment, int idx) {
	_ofs << "pop ";
	switch (segment)
	{
	case SEG::CONST:
		_ofs << "constant ";  break;
	case SEG::ARG:
		_ofs << "argument ";  break;
	case SEG::LOCAL:
		_ofs << "local "; break;
	case SEG::STATIC:
		_ofs << "static "; break;
	case SEG::THIS:
		_ofs << "this "; break;
	case SEG::THAT:
		_ofs << "that "; break;
	case SEG::POINTER:
		_ofs << "pointer "; break;
	case SEG::TEMP:
		_ofs << "temp "; break;
	default:
		break;
	}
	_ofs << idx << std::endl;
}

void VmWriter::writeArithmetic(CMD cmd) {
	switch (cmd) {
	case CMD::ADD:
		_ofs << "add" << std::endl; break;
	case CMD::SUB:
		_ofs << "sub" << std::endl; break;
	case CMD::NEG:
		_ofs << "neg" << std::endl; break;
	case CMD::EQ:
		_ofs << "eq" << std::endl; break;
	case CMD::GT:
		_ofs << "gt" << std::endl; break;
	case CMD::LT:
		_ofs << "lt" << std::endl; break;
	case CMD::AND:
		_ofs << "and" << std::endl; break;
	case CMD::OR:
		_ofs << "or" << std::endl; break;
	case CMD::NOT:
		_ofs << "not" << std::endl; break;
	default:
		break;
	}
}

void VmWriter::writeLabel(const std::string& label) {
	_ofs << "label " << label << std::endl;
}

void VmWriter::writeGoto(const std::string& label) {
	_ofs << "goto " << label << std::endl;
}

void VmWriter::writeIf(const std::string& label) {
	_ofs << "if-goto " << label << std::endl;
}

void VmWriter::writeCall(const std::string& name, int numArgs) {
	_ofs << "call " << name << " " << numArgs << std::endl;
}

void VmWriter::writeFunction(const std::string& name, int numLocals) {
	_ofs << "function " << name << " " << numLocals << std::endl;;
}

void VmWriter::writeReturn() {
	_ofs << "return" << std::endl;
}

void VmWriter::close() {
	_ofs.close();
}