#include <filesystem>
#include <iostream>
#include "compiler.hpp"

std::vector<std::string> find_source_files(const std::string& target_name) {
	//search direcotry whose name is identical to the target name
	std::string dir_name = ".\\\\" + target_name;
	std::vector<std::string> paths;
	if (std::filesystem::exists(dir_name)) {
		for (const auto& entry : std::filesystem::directory_iterator(dir_name)) {
			if (entry.path().string().find_last_of(".jack") == entry.path().string().size() - 1) {
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
	
	std::cout << "Enter the name of a source file (without file extension):";
	std::string sourceName;
	std::cin >> sourceName;
	
	auto sources = find_source_files(sourceName);
	if (sources.empty()) {
		std::cerr << "No source file found." << std::endl;
		return -1;
	}
	Formatter formatter;
	for (auto&& source : sources) {
		std::string targetName(source.begin(), source.end() - 5);
		std::string intermediateName = targetName + "_temp.txt";
		std::string outputName = targetName + ".vm";
		formatter.eraseComments(source, intermediateName);
		CompileEngine engine(intermediateName, outputName);
		engine.compileClass();
	}

	return 0;
}