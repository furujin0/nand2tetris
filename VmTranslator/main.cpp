#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include "VmTranslator.hpp"

std::vector<std::string> find_vm_files(const std::string& target_name) {
	//search direcotry
	std::string dir_name = ".\\\\" + target_name;
	std::vector<std::string> paths;
	if (std::filesystem::exists(dir_name)) {
		for (const auto& entry : std::filesystem::directory_iterator(dir_name)) {
			if (entry.path().string().find_last_of(".vm") == entry.path().string().size() - 1) {
				paths.push_back(entry.path().string());
			}
		}
	}
	//search .vm file
	std::string vm_file_path = ".\\\\" + target_name + ".vm";
	for (const auto& entry : std::filesystem::directory_iterator(".")) {
		if (entry.path() == vm_file_path) {
			paths.push_back(entry.path().string());
		}
	}
	return paths;
}

int main(int argc, char** argv) {
	std::string target_name;
	std::cout << "Enter the name of the target file/directory:";
	std::cin >> target_name;
	//find .vm file
	auto paths = find_vm_files(target_name);

	if (paths.empty()) {
		std::cerr << "No target file was detected." << std::endl;
		return -1;
	}
	std::cout << "Target file(s) detected." << std::endl;
	for (auto&& path : paths) {
		std::cout << path << std::endl;
	}
	std::cout << std::endl;
	CodeWriter writer(target_name);
	writer.writeInit();
	for (auto&& path : paths) {
		std::cout << path << std::endl;
		Parser parser(path);
		auto pos_beg = path.rfind("\\");
		std::string filename(path.begin() + pos_beg + 1, path.end() - 3);
		std::cout << "Parser opened a file: " << filename << std::endl;

		writer.setFileName(filename);

		while (parser.hasMoreCommands()) {
			parser.advance();
			std::cout << parser.currentCommand() << std::endl;
			switch (parser.commandType()) {
			case CmdType::C_ARITHMETIC:
				writer.writeArithmetic(parser.arg1()); break;
			case CmdType::C_POP:
				writer.writePop(parser.arg1(), parser.arg2()); break;
			case CmdType::C_PUSH:
				std::cout << parser.arg1() << ", " << parser.arg2() << std::endl;
				writer.writePush(parser.arg1(), parser.arg2()); break;
			case CmdType::C_LABEL:
				writer.writeLabel(parser.arg1()); break;
			case CmdType::C_FUNCTION:
				writer.writeFunction(parser.arg1(), parser.arg2()); break;
			case CmdType::C_IF:
				writer.writeIf(parser.arg1()); break;
			case CmdType::C_CALL:
				writer.writeCall(parser.arg1(), parser.arg2()); break;
			case CmdType::C_RETURN:
				writer.writeReturn(); break;
			case CmdType::C_GOTO:
				writer.writeGoto(parser.arg1()); break;
			default:
				break;
			}
		}
	}
	
	writer.close();
	return 0;
}