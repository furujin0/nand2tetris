#include <iostream>
#include "compiler.hpp"

int main(int argc, char** argv) {
	
	std::cout << "Enter the name of a source file (without file extension):";
	std::string targetName;
	std::cin >> targetName;
	
	std::string sourceName = targetName + ".jack";
	std::string intermediateName = targetName + "_temp.txt";
	Formatter formatter;
	formatter.eraseComments(sourceName, intermediateName);
	std::string outputName = targetName + ".vm";

	CompileEngine engine(intermediateName, outputName);
	
	engine.compileClass();

	return 0;
}