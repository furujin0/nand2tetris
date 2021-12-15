#include "assembler.hpp"

Parser::Parser(const std::string& filename) {
	ifs.open(filename);
	if (!ifs.is_open())
	{
		std::cerr << "Falied to open the specified file" << filename << std::endl;
	}
}

Parser::~Parser() {
	ifs.close();	
}

void Parser::advance() {
	std::getline(ifs, command);
	const char space = ' ';
	//erase comment
	auto pos = command.find("//");
	if (pos != decltype(command)::npos) {
		command.erase(command.begin() + pos, command.end());
	}
	//erase space
	while (true) {
		auto pos = command.find(space);
		if (pos == decltype(command)::npos) {
			break;
		}
		command.erase(command.begin() + pos, command.begin() + pos + 1);
	}
	if (command.empty() && hasMoreCommands()) {
		advance();
	}
	else if (commandType()!=CmdType::L_CMD) {
		++address;
	}
}

bool Parser::hasMoreCommands() {
	return !ifs.eof();
}

CmdType Parser::commandType() {
	if (command.empty()) {
		return CmdType::INVALID;
	}
	auto type = CmdType::C_CMD;
	switch (command[0]) {
	case '@':
		type = CmdType::A_CMD;
		break;
	case '(':
		type = CmdType::L_CMD;
		break;
	default:
		break;
	}
	return type;
}

std::string Parser::symbol() {
	std::string s;
	switch (commandType()) {
	case CmdType::A_CMD:
		s = std::string(command.begin() + 1, command.end());
		break;
	case CmdType::L_CMD:
		s = std::string(command.begin() + 1, command.end() - 1);
	}
	return s;
}

std::string Parser::dest() {
	auto pos = command.find('=');
	if (pos == decltype(command)::npos) {
		pos = 0;
	}
	return std::string(command.begin(), command.begin() + pos);
}

std::string Parser::comp() {
	auto pos_eq = command.find('=');
	if (pos_eq == decltype(command)::npos) {
		pos_eq = -1;
	}
	auto pos_semicolon = command.find(';');
	if (pos_semicolon == decltype(command)::npos) {
		pos_semicolon = command.size();
	}
	return std::string(command.begin() + pos_eq + 1, command.begin() + pos_semicolon);
}

std::string Parser::jump() {
	auto pos = command.find(';');
	return std::string(command.begin() + pos + 1, command.end());
}

void Parser::goInit() {
	ifs.clear();
	ifs.seekg(0, decltype(ifs)::beg);
	address = -1;
}

std::string Parser::currentCommand() {
	return command;
}

std::vector<bool> Code::comp(const std::string& str) {
	std::vector<bool> c;
	std::vector<bool> res{ !(str.find('M') == std::string::npos) };
	if (str == "0") {
		c = { 1, 0, 1, 0, 1, 0};
	}
	else if (str == "1") {
		c = { 1, 1, 1, 1, 1, 1};
	}
	else if(str == "-1"){
		c = { 1, 1, 1, 0, 1, 0};
	}
	else if (str == "D") {
		c = { 0, 0, 1, 1, 0, 0};
	}
	else if (str == "A" || str == "M") {
		c = { 1, 1, 0, 0, 0, 0};
	}
	else if (str == "!D") {
		c = { 0, 0, 1, 1, 0, 1};
	}
	else if (str == "!A" || str=="!M") {
		c = { 1, 1, 0, 0, 0, 1 };
	}
	else if (str == "-D") {
		c = { 0, 0, 1, 1, 1, 1 };
	}
	else if (str == "-A" || str == "-M") {
		c = { 1, 1, 0, 0, 1, 1 };
	}
	else if (str == "D+1") {
		c = { 0, 1, 1, 1, 1, 1 };
	}
	else if (str == "A+1" || str == "M+1") {
		c = { 1, 1, 0, 1, 1, 1 };
	}
	else if (str == "D-1") {
		c = { 0, 0, 1, 1, 1, 0 };
	}
	else if (str == "A-1" || str == "M-1") {
		c = { 1, 1, 0, 0, 1, 0 };
	}
	else if (str == "D+A" || str == "D+M") {
		c = { 0, 0, 0, 0, 1, 0 };
	}
	else if (str == "D-A" || str == "D-M") {
		c = { 0, 1, 0, 0, 1, 1 };
	}
	else if(str=="A-D" || str=="M-D") {
		c = { 0, 0, 0, 1, 1, 1 };
	}
	else if (str == "D&A" || str == "D&M") {
		c = { 0, 0, 0, 0, 0, 0 };
	}
	else if (str == "D|A" || str == "D|M") {
		c = { 0, 1, 0, 1, 0, 1 };
	}

	res.insert(res.end(), c.begin(), c.end());
	return res;
}

std::vector<bool> Code::dest(const std::string& str) {
	std::vector<bool> res(3);
	res[0] = !(str.find('A') == std::string::npos);
	res[1] = !(str.find('D') == std::string::npos);
	res[2] = !(str.find('M') == std::string::npos);
	return res;
}

std::vector<bool> Code::jump(const std::string& str) {
	std::vector<bool> res;
	if (str == "JGT") {
		res = { 0, 0, 1 };
	}
	else if (str == "JEQ") {
		res = { 0, 1, 0 };
	}
	else if (str == "JGE") {
		res = { 0, 1, 1 };
	}
	else if (str == "JLT") {
		res = { 1, 0, 0 };
	}
	else if(str == "JNE"){
		res = { 1, 0, 1 };
	}
	else if (str == "JLE") {
		res = { 1, 1, 0 };
	}
	else if (str == "JMP") {
		res = { 1, 1, 1 };
	}
	else{
		res = {0, 0, 0};
	}
	return res;
}

SymbolTable::SymbolTable() {
	//Add predefined labels to the table
	table.insert(std::make_pair("SP", 0));
	table.insert(std::make_pair("LCL", 1));
	table.insert(std::make_pair("ARG", 2));
	table.insert(std::make_pair("THIS", 3));
	table.insert(std::make_pair("THAT", 4));
	for (int i = 0; i <= 15; i++) {
		table.insert(std::make_pair(std::string("R") + std::to_string(i), i));
	}
	table.insert(std::make_pair("SCREEN", 0x4000));
	table.insert(std::make_pair("KBD", 0x6000));
}

void SymbolTable::addEntry(const std::string& symbol, int address) {
	table.insert(std::make_pair(symbol, address));
}

bool SymbolTable::contains(const std::string& symbol) {
	return table.contains(symbol);
}

int SymbolTable::getAddress(const std::string& symbol) {
	return table.at(symbol);
}

void SymbolTable::show() {
	for (auto&& e : table) {
		std::cout << e.first << " : " << e.second << std::endl;
	}
}