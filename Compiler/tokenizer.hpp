#pragma once

#include <fstream>
#include <set>
#include <string>
#include <unordered_map>
#include "privdef.hpp"

class Tokenizer {

	std::unordered_map<std::string, KEYWORD> dictKeyword{
		std::make_pair(std::string("class"), KEYWORD::CLASS),
		std::make_pair(std::string("method"), KEYWORD::METHOD),
		std::make_pair(std::string("constructor"), KEYWORD::CONSTRUCTOR),
		std::make_pair(std::string("function"), KEYWORD::FUNCTION),
		std::make_pair(std::string("int"), KEYWORD::INT),
		std::make_pair(std::string("var"), KEYWORD::VAR),
		std::make_pair(std::string("static"), KEYWORD::STATIC),
		std::make_pair(std::string("field"), KEYWORD::FIELD),
		std::make_pair(std::string("let"), KEYWORD::LET),
		std::make_pair(std::string("do"), KEYWORD::DO),
		std::make_pair(std::string("if"), KEYWORD::IF),
		std::make_pair(std::string("else"), KEYWORD::ELSE),
		std::make_pair(std::string("while"), KEYWORD::WHILE),
		std::make_pair(std::string("return"), KEYWORD::RETURN),
		std::make_pair(std::string("true"), KEYWORD::TRUE),
		std::make_pair(std::string("false"), KEYWORD::FALSE),
		std::make_pair(std::string("null"), KEYWORD::NULL_WORD),
		std::make_pair(std::string("this"), KEYWORD::THIS),
		std::make_pair(std::string("void"), KEYWORD::VOID),
		std::make_pair(std::string("boolean"), KEYWORD::BOOLEAN),
		std::make_pair(std::string("char"), KEYWORD::CHAR)
	};

	std::set<char> symbolSet{
		'{', '}', '(', ')', '[', ']', '.', ',', ';', '+', '-', '*', '/', '&', '|', '<', '>', '=', '-', '~'
	};

	std::ifstream _ifs;
	std::string _inputName;
	std::string _token;

public:
	Tokenizer(const std::string& inputName);

	bool hasMoreTokens() const;

	void advance();

	std::string token();

	TOKEN_TYPE tokenType() const;

	KEYWORD keyWord() const;

	char symbol() const;

	std::string identifier() const;

	int intVal() const;

	std::string stringVal() const;

	bool isNonTokenChar(char c) const;

	bool isSymbol(char c) const;

	bool isSymbol(const std::string& str) const;

	bool isKeyword(const std::string& str) const;

	bool isInteger(const std::string& str) const;

	bool isString(const std::string& str) const;

	bool isIdentifier(const std::string& str) const;
};
