#include <fstream>
#include <string>
#include "privdef.hpp"

class Tokenizer {
	std::ifstream _ifs;
	std::string _inputName;

public:
	Tokenizer(const std::string& inputName);

	bool hasMoreTokens();

	void eraseComments(const std::string& outputName);

	void advance();

	TOKEN_TYPE tokenType();

	KEYWORD keyWord();

	char symbol();

	std::string identifier();

	int intVal();

	std::string StringVal();
};