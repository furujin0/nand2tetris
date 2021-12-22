#pragma once

#include <fstream>
#include <set>
#include <unordered_set>
#include <string>
#include "privdef.hpp"

class Formatter {
public:
	void eraseComments(
		const std::string& inputName,
		const std::string& outputName
	) const;
};

class Tokenizer {

	std::unordered_map<std::string, KEYWORD> dictKeyword{
		std::make_pair(std::string("class"), KEYWORD::CLASS),
		std::make_pair(std::string("method"), KEYWORD::METHOD),
		std::make_pair(std::string("constructor"), KEYWORD::CONSTRUCTOR),
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
		std::make_pair(std::string("void"), KEYWORD::VOID)
	};

	std::set<char> symbolSet{
		'{', '}', '(', ')', '[', ']', '.', ',', ';', '+', '-', '*', '/', '&', '|', '<', '>', '=', '-'
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

class CompileEngine {
	Tokenizer _tokenizer;
	Formatter _formatter;
	std::ofstream _ofs;
	std::unordered_set<std::string> classSet;

public:
	CompileEngine(
		const std::string& inputName,
		const std::string& outputName
	);

	void compileClass();

	void compileClassVarDec();

	void compileSubroutine();

	void compileParameterList();

	void compileVarDec();

	void compileStatements();

	void compileDo();

	void compileLet();

	void compileWhile();

	void compileReturn();

	void compileIf();

	void compileExpression();

	void compileTerm();

	void compileExpressionList();

	void writeKeyword(KEYWORD keyword);

	void writeSymbol(char c);

	void writeIdentifier(const std::string& identifier);

	void writeType();

	void writeIntConst(int value);

	void writeStringConst(const std::string& str);

	bool isClassName(const std::string& name);

	bool isBuiltInType(const std::string& type);

	bool isOp(char c);

	bool isUnaryOp(char c);

	void compileSubroutineCall();
};