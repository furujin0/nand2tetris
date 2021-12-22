#include <iostream>
#include "compiler.hpp"

int main(int argc, char** argv) {
	
	std::cout << "Enter the name of a source file (without file extension):";
	std::string targetName;
	std::cin >> targetName;
	
	std::string sourceName = targetName + ".jack";
	std::string intermediateName = targetName + "_temp.txt";
	std::string outputName = targetName + ".xml";

	Formatter formatter;
	formatter.eraseComments(sourceName, intermediateName);

	Tokenizer t(intermediateName);
	while (t.hasMoreTokens()) {
		t.advance();
		std::cout << t.token() << std::endl;
	}
	CompileEngine engine(intermediateName, outputName);
	engine.compileClass();

	return 0;
}