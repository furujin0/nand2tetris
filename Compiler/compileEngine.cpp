#include <cctype>
#include <iostream>
#include <string>
#include "compileEngine.hpp"

CompileEngine::CompileEngine(
	const std::string& inputName,
	const std::string& outputName
) :_tokenizer(inputName), _writer(outputName)
{
	_ofs.open(outputName);
}
/**/
void CompileEngine::compileClass() {
	_tokenizer.advance(); //to class keyword
	_tokenizer.advance(); // move to class name
	
	_className = _tokenizer.identifier(); //get class name
	_tokenizer.advance();

	compileSymbol(); // {

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
	_ofs << "</class>";
	return;
}

void CompileEngine::compileClassVarDec() {

	// determine static || field
	KIND kind = KIND::NONE;
	if (_tokenizer.keyWord() == KEYWORD::STATIC) {
		kind = KIND::STATIC;
	}
	else if (_tokenizer.keyWord() == KEYWORD::FIELD) {
		kind = KIND::FIELD;
	}
	_tokenizer.advance();
	auto type = _tokenizer.identifier();
	do {
		_tokenizer.advance();
		auto name = _tokenizer.identifier();

		_classSymbols.define(name, type, kind);
		_tokenizer.advance();
		if (_tokenizer.symbol() == ';') {
			break;
		}
	} while (true);

	_tokenizer.advance();
}

void CompileEngine::compileSubroutine() {
	_subroutineSymbols.startSubroutine();
	bool isMethod = false;
	if (_tokenizer.keyWord() == KEYWORD::FUNCTION||_tokenizer.keyWord() == KEYWORD::CONSTRUCTOR) {
		isMethod = false;
	}
	else if (_tokenizer.keyWord() == KEYWORD::METHOD) {
		isMethod = true;
	}
	_tokenizer.advance(); // move to  return type ( In Jack, every function returns value of 16bit  )
	_tokenizer.advance(); // move to subroutineName
	auto subroutineName = _tokenizer.identifier();
	_tokenizer.advance(); // move to symbol '('
	_tokenizer.advance(); // move to parameterList
	compileParameterList(); // number of args are determined in call, so this code only moves the position to ')'.
	_tokenizer.advance();// move to subroutine body

	//subroutine body
	compileSymbol();
	_tokenizer.advance(); // move into statements
	while (_tokenizer.tokenType() == TOKEN_TYPE::KEYWORD && _tokenizer.keyWord() == KEYWORD::VAR) {
		compileVarDec();
	}
	_writer.writeFunction(subroutineName, _subroutineSymbols.varCount(KIND::VAR));
	compileStatements();
	compileSymbol();
	_ofs << std::string(indent, ' ') << "</subroutineBody>" << std::endl;
	_ofs << std::string(indent, ' ') << "</subroutineDec>" << std::endl;
}

void CompileEngine::compileVarDec() {
	_tokenizer.advance(); //move from var to type
	//now current token is type
	auto type = _tokenizer.identifier();
	do {
		_tokenizer.advance();
		_subroutineSymbols.define(_tokenizer.identifier(), type, KIND::VAR);
		_tokenizer.advance(); // move to ',' or ';'
		if (_tokenizer.symbol() == ';') {
			break;
		}
	} while (true);
	_tokenizer.advance();
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
	while (_tokenizer.tokenType() != TOKEN_TYPE::SYMBOL && _tokenizer.symbol() != ')') {
		_tokenizer.advance();
	}
	_tokenizer.advance();
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
	compileKeyword();
	_tokenizer.advance(); // move to varName;
	auto varName = _tokenizer.identifier();
	_tokenizer.advance();
	int idx = 0;
	if (_tokenizer.symbol() == '[') {
		_tokenizer.advance();
		idx = _tokenizer.intVal();
		_tokenizer.advance();
		_tokenizer.advance();
	}
	_tokenizer.advance(); // move from '=' to expression
	compileExpression();
	_writer.writePop(SEG::LOCAL, idx);
	_tokenizer.advance(); //move from ';'
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

	switch (_tokenizer.tokenType()) {
	case TOKEN_TYPE::INT_CONST:
		_writer.writePush(SEG::CONST, _tokenizer.intVal());
		_tokenizer.advance();
		break;
	case TOKEN_TYPE::STRING_CONST:
		_writer.writeCall("String.new", _tokenizer.stringVal().length());
		for (auto&& c : _tokenizer.stringVal()) {
			_writer.writePush(SEG::CONST, c);
			_writer.writeCall("String.appendChar", 2);
		}
		_tokenizer.advance();
		break;
	case TOKEN_TYPE::KEYWORD: //
		switch (_tokenizer.keyWord()) {
		case KEYWORD::TRUE: // push -1
			_writer.writePush(SEG::CONST, 0);
			_writer.writeArithmetic(CMD::NEG);
			break;
		case KEYWORD::FALSE:
			_writer.writePush(SEG::CONST, 0);
			break;
		case KEYWORD::NULL_WORD:
			_writer.writePush(SEG::CONST, 0);
			break;
		case KEYWORD::THIS:
			_writer.writePush(SEG::THIS, 0);
		default:
			break;
		}
		_tokenizer.advance();
		break;
	case TOKEN_TYPE::SYMBOL:
		if (_tokenizer.symbol() == '(') {
			_tokenizer.advance(); // move to expression
			compileExpression();
			_tokenizer.advance(); // move from ')'
		}
		else {
			compileSymbol();
			auto unaryOp = _tokenizer.symbol();
			compileTerm();
			if (unaryOp == '-')
				_writer.writeArithmetic(CMD::NEG);
			else if (unaryOp == '~')
				_writer.writeArithmetic(CMD::NOT);
		}
		break;
	case TOKEN_TYPE::IDENTIFIER:
		//compileIdentifier();

		if (_subroutineSymbols.isDefined(_tokenizer.identifier())) {
			_subroutineSymbols.kindOf(_tokenizer.identifier());
			_subroutineSymbols.indexOf(_tokenizer.identifier());
			 _subroutineSymbols.typeOf(_tokenizer.identifier());
		}
		else if (_classSymbols.isDefined(_tokenizer.identifier())) {
			_classSymbols.kindOf(_tokenizer.identifier());
			_classSymbols.indexOf(_tokenizer.identifier());
			_classSymbols.typeOf(_tokenizer.identifier());
		}
		else { //subroutine
			_tokenizer.advance(); //move from name to '(';
			compileExpressionList();
			_tokenizer.advance(); //move from ')'

		}
		_tokenizer.advance();
		if (_tokenizer.tokenType() == TOKEN_TYPE::SYMBOL && _tokenizer.symbol() == '[') { //array type
			_tokenizer.advance(); // move to expression
			compileExpression();
			//now result is pushed to the stack

			compileSymbol(); //move from ']'
		}
		else if (_tokenizer.tokenType() == TOKEN_TYPE::SYMBOL && _tokenizer.symbol() == '(') { // expression
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
	_tokenizer.advance();
}


void CompileEngine::compileSymbol() {
	_tokenizer.advance();
}