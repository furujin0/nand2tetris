#include <iostream>
#include "compiler.hpp"

int main(int argc, char** argv) {
	std::string inputName("Square.jack");
	std::string formattedName("testProcessed.txt");
	Formatter formatter;
	formatter.eraseComments(inputName, formattedName);

	Tokenizer tokenizer(formattedName);
	
	while (tokenizer.hasMoreTokens()) {
		tokenizer.advance();
		std::cout << tokenizer.token() << std::endl;
	}

	return 0;
}