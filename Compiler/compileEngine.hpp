#pragma once

#include <fstream>
#include <string>
#include <unordered_set>
#include "formatter.hpp"
#include "privdef.hpp"
#include "symbolTable.hpp"
#include "tokenizer.hpp"
#include "vmWriter.hpp"


class CompileEngine {
	Tokenizer _tokenizer;
	SymbolTable _classSymbols;
	SymbolTable _subroutineSymbols;
	VmWriter _writer;
	std::ofstream _ofs;
	std::unordered_set<std::string> classSet;
	std::string _className;
	std::string _subroutineName;
	int _numArgs;
	int indent = 0;

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

	void compileIdentifier();

	void compileSymbol();

	void compileKeyword();
};