#include <cctype>
#include <iostream>
#include <string>
#include "compileEngine.hpp"

CompileEngine::CompileEngine(
	const std::string& inputName,
	const std::string& outputName
) :_tokenizer(inputName)
{
	_ofs.open(outputName);
}

void CompileEngine::compileClass() {
	_ofs << "<class>" << std::endl;
	indent += 2;
	_tokenizer.advance();
	compileKeyword();
	compileIdentifier();
	compileSymbol();

	while (_tokenizer.tokenType() == TOKEN_TYPE::KEYWORD
		&& (_tokenizer.keyWord() == KEYWORD::STATIC || _tokenizer.keyWord() == KEYWORD::FIELD))
	{
		compileClassVarDec();
	}
	
	while (_tokenizer.tokenType() == TOKEN_TYPE::KEYWORD
		&& (_tokenizer.keyWord() == KEYWORD::CONSTRUCTOR
			|| _tokenizer.keyWord() == KEYWORD::FUNCTION
			|| _tokenizer.keyWord() == KEYWORD::METHOD))
	{
		compileSubroutine();
	}

	compileSymbol();
	indent -= 2;
	_ofs << "</class>";
	return;
}

void CompileEngine::compileClassVarDec() {
	_ofs << std::string(indent, ' ') << "<classVarDec>" << std::endl;
	indent += 2;
	compileKeyword();
	writeType();

	do {
		_tokenizer.advance();
		compileIdentifier();
		writeSymbol(_tokenizer.symbol()); // comma or semicolon
		if (_tokenizer.symbol() == ';') {
			break;
		}
	} while (true);

	_tokenizer.advance();
	indent -= 2;
	_ofs << std::string(indent, ' ') << "</classVarDec>" << std::endl;
}

void CompileEngine::compileSubroutine() {
	_ofs << std::string(indent, ' ') << "<subroutineDec>" << std::endl;
	indent += 2;

	compileKeyword();
	//void or type
	writeType();
	_tokenizer.advance();
	//subroutine name
	compileIdentifier();
	//(
	compileSymbol();
	compileParameterList();
	//)
	compileSymbol();

	//subroutine body
	_ofs << std::string(indent, ' ') << "<subroutineBody>" << std::endl;
	indent += 2;
	compileSymbol();
	while (_tokenizer.tokenType() == TOKEN_TYPE::KEYWORD && _tokenizer.keyWord() == KEYWORD::VAR) {
		compileVarDec();
	}
	compileStatements();
	compileSymbol();
	indent -= 2;
	_ofs << std::string(indent, ' ') << "</subroutineBody>" << std::endl;
	indent -= 2;
	_ofs << std::string(indent, ' ') << "</subroutineDec>" << std::endl;
}

void CompileEngine::compileVarDec() {
	_ofs << std::string(indent, ' ') << "<varDec>" << std::endl;
	indent += 2;

	compileKeyword();
	writeType();
	do {
		_tokenizer.advance();
		compileIdentifier();
		writeSymbol(_tokenizer.symbol());
		if (_tokenizer.symbol() == ';') {
			break;
		}
	} while (true);
	_tokenizer.advance();
	indent -= 2;

	_ofs << std::string(indent, ' ') << "</varDec>" << std::endl;
}

void CompileEngine::compileIdentifier() {
	writeIdentifier(_tokenizer.identifier());
	_tokenizer.advance();
	while (_tokenizer.tokenType() == TOKEN_TYPE::SYMBOL && _tokenizer.symbol() == '.') {
		compileSymbol();
		writeIdentifier(_tokenizer.identifier());
		_tokenizer.advance();
	}
}

void CompileEngine::compileParameterList() {
	_ofs << std::string(indent, ' ') << "<parameterList>" << std::endl;
	if (_tokenizer.tokenType() != TOKEN_TYPE::SYMBOL && _tokenizer.symbol() != ')')
	{
		do {
			writeType();
			_tokenizer.advance();
			compileIdentifier();
			if (_tokenizer.symbol() == ',') {
				compileSymbol();
			}
			else {
				break;
			}
		} while (true);
	}
	_ofs << std::string(indent, ' ') << "</parameterList>" << std::endl;
}

void CompileEngine::writeSymbol(const char c) {
	std::string sym;
	switch (c) {
	case '<':
		sym = "&lt;"; break;
	case '>':
		sym = "&gt;"; break;
	case '"':
		sym = "&quot;"; break;
	case '&':
		sym = "&amp;"; break;
	default:
		sym = std::string(1, c);
	}
	_ofs << std::string(indent, ' ') << "<symbol> " << sym << " </symbol>" << std::endl;
}

void CompileEngine::writeKeyword(KEYWORD keyword) {
	std::string keywordStr;
	switch (keyword)
	{
	case KEYWORD::CLASS:
		keywordStr = "class"; break;
	case KEYWORD::METHOD:
		keywordStr = "method"; break;
	case KEYWORD::FUNCTION:
		keywordStr = "function"; break;
	case KEYWORD::CONSTRUCTOR:
		keywordStr = "constructor"; break;
	case KEYWORD::INT:
		keywordStr = "int"; break;
	case KEYWORD::VAR:
		keywordStr = "var"; break;
	case KEYWORD::STATIC:
		keywordStr = "static"; break;
	case KEYWORD::FIELD:
		keywordStr = "field"; break;
	case KEYWORD::LET:
		keywordStr = "let"; break;
	case KEYWORD::DO:
		keywordStr = "do"; break;
	case KEYWORD::IF:
		keywordStr = "if"; break;
	case KEYWORD::ELSE:
		keywordStr = "else"; break;
	case KEYWORD::WHILE:
		keywordStr = "while"; break;
	case KEYWORD::RETURN:
		keywordStr = "return"; break;
	case KEYWORD::TRUE:
		keywordStr = "true"; break;
	case KEYWORD::FALSE:
		keywordStr = "false"; break;
	case KEYWORD::NULL_WORD:
		keywordStr = "null"; break;
	case KEYWORD::THIS:
		keywordStr = "this"; break;
	case KEYWORD::VOID:
		keywordStr = "void"; break;
	case KEYWORD::BOOLEAN:
		keywordStr = "boolean"; break;
	case KEYWORD::CHAR:
		keywordStr = "char"; break;
	default:
		break;
	}
	_ofs << std::string(indent, ' ') << "<keyword> " << keywordStr << " </keyword>" << std::endl;
}

void CompileEngine::writeIdentifier(const std::string& identifier) {
	_ofs << std::string(indent, ' ') << "<identifier> " << identifier << " </identifier>" << std::endl;
}

void CompileEngine::writeType() {
	if (_tokenizer.tokenType() == TOKEN_TYPE::KEYWORD)
		writeKeyword(_tokenizer.keyWord());
	else
		writeIdentifier(_tokenizer.identifier());
}

bool CompileEngine::isClassName(const std::string& name) {
	return classSet.find(name) != classSet.end();
}

bool CompileEngine::isBuiltInType(const std::string& type) {
	return type == "int" || type == "char" || type == "boolean";
}

void CompileEngine::compileStatements() {
	_ofs << std::string(indent, ' ') << "<statements>" << std::endl;
	indent += 2;

	bool hasMoreStatements = true;
	while (hasMoreStatements) {
		if (_tokenizer.tokenType() == TOKEN_TYPE::KEYWORD) {
			switch (_tokenizer.keyWord()) {
			case KEYWORD::LET:
				compileLet(); break;
			case KEYWORD::IF:
				compileIf(); break;
			case KEYWORD::WHILE:
				compileWhile(); break;
			case KEYWORD::DO:
				compileDo(); break;
			case KEYWORD::RETURN:
				compileReturn(); break;
			default:
				hasMoreStatements = false; break;
			}
		}
		else {
			hasMoreStatements = false;
		}
	}
	indent -= 2;

	_ofs << std::string(indent, ' ') << "</statements>" << std::endl;
}

void CompileEngine::compileLet() {
	_ofs << std::string(indent, ' ') << "<letStatement>" << std::endl;
	indent += 2;
	compileKeyword();
	compileIdentifier();
	if (_tokenizer.symbol() == '[') {
		compileSymbol();
		compileExpression();
		compileSymbol();
	}
	compileSymbol();
	compileExpression();
	compileSymbol();
	indent -= 2;
	_ofs << std::string(indent, ' ') << "</letStatement>" << std::endl;
}

void CompileEngine::compileIf() {
	_ofs << std::string(indent, ' ') << "<ifStatement>" << std::endl;
	indent += 2;

	compileKeyword();
	compileSymbol();
	compileExpression();
	compileSymbol();
	compileSymbol();
	compileStatements();
	compileSymbol();
	if (_tokenizer.tokenType() == TOKEN_TYPE::KEYWORD && _tokenizer.keyWord() == KEYWORD::ELSE) {
		compileKeyword();
		compileSymbol();
		compileStatements();
		compileSymbol();
	}
	indent -= 2;

	_ofs << std::string(indent, ' ') << "</ifStatement>" << std::endl;
}

void CompileEngine::compileWhile() {
	_ofs << std::string(indent, ' ') << "<whileStatement>" << std::endl;
	indent += 2;
	compileKeyword();
	compileSymbol();
	compileExpression();
	compileSymbol();
	compileSymbol();
	compileStatements();
	compileSymbol();
	indent -= 2;
	_ofs << std::string(indent, ' ') << "</whileStatement>" << std::endl;
}

void CompileEngine::compileDo() {
	_ofs << std::string(indent, ' ') << "<doStatement>" << std::endl;
	indent += 2;
	compileKeyword();
	compileSubroutineCall();
	compileSymbol();
	indent -= 2;
	_ofs << std::string(indent, ' ') << "</doStatement>" << std::endl;
}

void CompileEngine::compileSubroutineCall() {
	compileIdentifier();
	compileSymbol();
	compileExpressionList();
	compileSymbol();
}

void CompileEngine::compileReturn() {
	_ofs << std::string(indent, ' ') << "<returnStatement>" << std::endl;
	indent += 2;

	compileKeyword();
	if (_tokenizer.tokenType() != TOKEN_TYPE::SYMBOL || _tokenizer.symbol() != ';') {
		compileExpression();
	}
	compileSymbol();
	indent -= 2;

	_ofs << std::string(indent, ' ') << "</returnStatement>" << std::endl;
}

bool CompileEngine::isOp(char c) {
	return c == '+' || c == '-' || c == '*' || c == '/' || c == '&' || c == '|' || c == '<' || c == '>' || c == '=';
}

bool CompileEngine::isUnaryOp(char c) {
	return c == '+' || c == '-';
}

void CompileEngine::compileTerm() {
	_ofs << std::string(indent, ' ') << "<term>" << std::endl;
	indent += 2;

	switch (_tokenizer.tokenType()) {
	case TOKEN_TYPE::INT_CONST:
		writeIntConst(_tokenizer.intVal());
		_tokenizer.advance();
		break;
	case TOKEN_TYPE::STRING_CONST:
		writeStringConst(_tokenizer.stringVal());
		_tokenizer.advance();
		break;
	case TOKEN_TYPE::KEYWORD:
		compileKeyword();
		break;
	case TOKEN_TYPE::SYMBOL:
		if (_tokenizer.symbol() == '(') {
			compileSymbol();
			compileExpression();
			compileSymbol();
		}
		else {
			compileSymbol();
			compileTerm();
		}
		break;
	case TOKEN_TYPE::IDENTIFIER:
		compileIdentifier();
		if (_tokenizer.tokenType() == TOKEN_TYPE::SYMBOL && _tokenizer.symbol() == '[') {
			compileSymbol();
			compileExpression();
			compileSymbol();
		}
		else if (_tokenizer.tokenType() == TOKEN_TYPE::SYMBOL && _tokenizer.symbol() == '(') {
			compileSymbol();
			compileExpressionList();
			compileSymbol();
		}
		break;
	case TOKEN_TYPE::NON_TOKEN:
		break;
	default:
		break;
	}
	indent -= 2;

	_ofs << std::string(indent, ' ') << "</term>" << std::endl;
}

void CompileEngine::writeIntConst(int value) {
	_ofs << std::string(indent, ' ') << "<integerConstant> " << value << " </integerConstant>" << std::endl;
}

void CompileEngine::writeStringConst(const std::string& str) {
	_ofs << std::string(indent, ' ') << "<stringConstant> " << str << " </stringConstant>" << std::endl;
}

void CompileEngine::compileExpression() {
	_ofs << std::string(indent, ' ') << "<expression>" << std::endl;
	indent += 2;

	compileTerm();
	while (_tokenizer.tokenType() == TOKEN_TYPE::SYMBOL && isOp(_tokenizer.symbol())) {
		compileSymbol();
		compileTerm();
	}
	indent -= 2;
	_ofs << std::string(indent, ' ') << "</expression>" << std::endl;
}

void CompileEngine::compileExpressionList() {
	_ofs << std::string(indent, ' ') << "<expressionList>" << std::endl;
	if (_tokenizer.tokenType() != TOKEN_TYPE::SYMBOL || _tokenizer.symbol() != ')') {
		compileExpression();
		while (_tokenizer.tokenType() == TOKEN_TYPE::SYMBOL && _tokenizer.symbol() == ',') {
			compileSymbol();
			compileExpression();
		}
	}
	_ofs << std::string(indent, ' ') << "</expressionList>" << std::endl;
}

void CompileEngine::compileKeyword() {
	writeKeyword(_tokenizer.keyWord());
	_tokenizer.advance();
}


void CompileEngine::compileSymbol() {
	writeSymbol(_tokenizer.symbol());
	_tokenizer.advance();
}