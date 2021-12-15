#include <bitset>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include "assembler.hpp"


int main(int argc, char** argv) {
	std::string source_name;
	std::cout << "Enter file pass:";
	std::cin >> source_name;

	std::string dest_name = std::string(source_name.begin(), source_name.end()-4) + ".hack";
	std::ofstream ofs(dest_name);
	Parser parser(source_name);
	SymbolTable symbol_table;

	//gather label symbols
	std::cout << "gathering label symbols...:" << std::endl;
	while (parser.hasMoreCommands()) {
		parser.advance();
		std::cout << parser.currentAddress() << " : " << parser.currentCommand();
		if (parser.commandType() == CmdType::L_CMD) {
			auto sym = parser.symbol();
			if (!symbol_table.contains(sym)) {
				symbol_table.addEntry(sym, parser.currentAddress() + 1); // add address of the next line
			}
			std::cout << ": label symbol: " << sym;
		}
		std::cout << std::endl;
	}

	parser.goInit();

	std::cout << "gathering variable symbols ..." << std::endl;


	//gather variable symbols
	int var_address = 16;
	while (parser.hasMoreCommands()) {
		parser.advance();
		std::cout << parser.currentAddress() << " : " << parser.currentCommand();
		auto type = parser.commandType();
		if (type == CmdType::A_CMD) {
			std::string sym = parser.symbol();
			if (!std::isdigit(sym[0]) && !symbol_table.contains(sym)) {
				symbol_table.addEntry(sym, var_address);
				var_address++;
				std::cout << ": var symbol: " << sym;
			}
		}
		std::cout << std::endl;
	}

	parser.goInit();
	symbol_table.show();
	//convert assembly to binary code
	while (parser.hasMoreCommands()) {
		parser.advance();
		auto type = parser.commandType();
		//auto cmd = parser.current_command();
		//ofs << cmd << "->";
		if (type == CmdType::A_CMD) {
			std::string sym = parser.symbol();
			auto value = (std::isdigit(sym[0]) ? std::atoi(sym.c_str()) : symbol_table.getAddress(sym));
			ofs << std::bitset<16>(value);
		}
		else if(type == CmdType::C_CMD) {
			ofs << "111";
			//ofs << " ";
			for (auto b : Code::comp(parser.comp())) {
				//ofs << "(" << parser.comp() << ")";
				ofs << b;
			}
			//ofs << " ";

			for (auto b : Code::dest(parser.dest())) {
				ofs << b;
			}
			//ofs << " ";

			for (auto b : Code::jump(parser.jump())) {
				ofs << b;
			}
			//ofs << " ";

		}
		else if (type == CmdType::L_CMD) {
			continue;
		}
		else {
			continue;
		}
		ofs << std::endl;
	}
	ofs.close();
	return 0;
}