#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include "VmTranslator.hpp"

std::vector<std::string> find_vm_files(const std::string& target_name) {
	//search direcotry
	std::string dir_name = ".\\\\" + target_name;
	std::vector<std::string> paths;
	for (const auto& entry : std::filesystem::directory_iterator(dir_name)) {
		if (entry.path().string().find_last_of(".vm") == entry.path().string().size() - 1) {
			paths.push_back(entry.path().string());
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
	std::cout << "Enter the name of a single .vm file or directory that contains .vm files.: ";
	std::cin >> target_name;
	//find .vm file
	auto paths = find_vm_files(target_name);
	for (auto& path : paths) {
		std::ifstream ifs(path);
		std::string line;
		std::getline(ifs, line);
		std::cout << line << std::endl;
		ifs.close();
	}

	return 0;
}