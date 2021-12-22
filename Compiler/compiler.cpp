#include <cctype>
#include <iostream>
#include "compiler.hpp"

void Formatter::eraseComments(
	const std::string& inputName, 
	const std::string& outputName
) const
{
	std::ifstream ifs(inputName);
	std::ofstream ofs(outputName);
	char c0, c1;
	c0 = ifs.get();
	c1 = ifs.get();
	while (!ifs.eof()) {
		if (c0 == '/' && c1 == '/') {
			while (c1 != '\n' && !ifs.eof()) {
				c0 = c1;
				c1 = ifs.get();
			}
		}
		else if (c0 == '/' && c1 == '*') {
			while (!(c0 == '*' && c1 == '/') && !ifs.eof()) {
				c0 = c1;
				c1 = ifs.get();
			}
			c1 = ifs.get();
		}
		else {
			ofs << c0;
		}
		c0 = c1;
		c1 = ifs.get();
		if (ifs.eof()) {
			if (std::char_traits<char>::not_eof(c0)) {
				ofs << c0;
			}
			if (std::char_traits<char>::not_eof(c1)) {
				std::cout << std::char_traits<char>::not_eof(c1);
				ofs << c1;
			}
		}
	}
	ofs.close();
}

bool Tokenizer::hasMoreTokens() const {
	return !_ifs.eof();
}

void Tokenizer::advance() {
	_token.clear();
	char c;
	do{
		c = _ifs.get();
	} while (c == ' ' || c == '\n' || c == '\r');
	if (c == std::char_traits<char>::eof()) {
		return;
	}
	if (symbolSet.find(c) != symbolSet.end()) {
		_token.append(1, c);
	}
	else {
		do {
			_token.append(1, c);
			auto n = _ifs.peek();

			if (!isNonTokenChar(n)) {
				c = _ifs.get();
			}
			else {
				break;
			}
		} while (true);
	}
}

Tokenizer::Tokenizer(const std::string& inputName)
:_inputName(inputName) {
	_ifs.open(inputName);

}

std::string Tokenizer::token() {
	return _token;
}

TOKEN_TYPE Tokenizer::tokenType() const {
	if (isKeyword(_token))
		return TOKEN_TYPE::KEYWORD;
	else if (isSymbol(_token))
		return TOKEN_TYPE::SYMBOL;
	else if (isInteger(_token))
		return TOKEN_TYPE::INT_CONST;
	else if (isString(_token))
		return TOKEN_TYPE::STRING_CONST;
	else if (isIdentifier(_token))
		return TOKEN_TYPE::IDENTIFIER;
	return TOKEN_TYPE::NON_TOKEN;
}

bool Tokenizer::isSymbol(char c) const {
	return symbolSet.find(c) != symbolSet.end();
}

bool Tokenizer::isSymbol(const std::string& str) const {
	return str.size() == 1 && isSymbol(str[0]);
}

bool Tokenizer::isKeyword(const std::string& str) const {
	return dictKeyword.find(str) != dictKeyword.end();
}

bool Tokenizer::isInteger(const std::string& str) const {
	int x = -1;
	auto res = true;
	try {
		x = std::stoi(str);
		if (x < 0 || x > 32767) {
			res = false;
		}
	}
	catch (std::invalid_argument) {
		res = false;
	}
	return res;
}

bool Tokenizer::isString(const std::string& str) const {
	return *str.begin() == '"' && *(--str.end()) == '"';
}

bool Tokenizer::isIdentifier(const std::string& str) const {
	if (std::isdigit(str[0]))
		return false;

	for (auto&& c : str) {
		if (!std::isalpha(c) && c != '_' && !std::isdigit(c))
			return false;
	}
	return true;
}

KEYWORD Tokenizer::keyWord() const {
	return dictKeyword.at(_token);
}

char Tokenizer::symbol() const {
	return _token[0];
}

std::string Tokenizer::identifier() const {
	return _token;
}

int Tokenizer::intVal() const {
	return std::stoi(_token);
}

std::string Tokenizer::stringVal() const {
	return std::move(std::string(_token.begin() + 1, _token.end() - 1));
}

bool Tokenizer::isNonTokenChar(char c) const {
	return
		c == ' '
		|| c == '\n'
		|| c == '\r'
		|| isSymbol(c)
		|| c == std::char_traits<char>::eof();
}

CompileEngine::CompileEngine(
	const std::string& inputName,
	const std::string& outputName
):_tokenizer(inputName)
{
	_ofs.open(outputName);	
}

void CompileEngine::compileClass() {
	_ofs << "<class>" << std::endl;
	_tokenizer.advance();
	if (_tokenizer.tokenType() != TOKEN_TYPE::KEYWORD || _tokenizer.keyWord() != KEYWORD::CLASS) {
		std::cerr << "The source file must start with the definition of a class. (class { ..." << std::endl;
		return;
	}
	writeKeyword(KEYWORD::CLASS);

	_tokenizer.advance();
	if (_tokenizer.tokenType() != TOKEN_TYPE::IDENTIFIER) {
		std::cerr << "Class name is missing." << std::endl;
		return;
	}
	writeIdentifier(_tokenizer.identifier());

	_tokenizer.advance();
	if (_tokenizer.tokenType() != TOKEN_TYPE::SYMBOL || _tokenizer.symbol() != '{') {
		std::cerr << "{ is missing." << std::endl;
		return;
	}
	writeSymbol(_tokenizer.symbol());

	_tokenizer.advance();
	while (_tokenizer.tokenType() != TOKEN_TYPE::KEYWORD
		&& (_tokenizer.keyWord() == KEYWORD::STATIC || _tokenizer.keyWord() == KEYWORD::FIELD)) {
		compileClassVarDec();
	}
	while (_tokenizer.tokenType() == TOKEN_TYPE::KEYWORD
		&& (_tokenizer.keyWord() == KEYWORD::CONSTRUCTOR
			|| _tokenizer.keyWord() == KEYWORD::FUNCTION
			|| _tokenizer.keyWord() == KEYWORD::METHOD)
		)
	{
		compileSubroutine();
	}

	if (_tokenizer.tokenType() != TOKEN_TYPE::SYMBOL || _tokenizer.symbol() != '}') {
		std::cerr << "} is missing." << std::endl;
		return;
	}
	writeSymbol(_tokenizer.symbol());
	_tokenizer.advance();
	_ofs << "</class>";
	return;
}

void CompileEngine::compileClassVarDec() {
	writeKeyword(_tokenizer.keyWord()); //output static / field

	_tokenizer.advance();
	writeType();

	do {
		_tokenizer.advance();
		writeIdentifier(_tokenizer.identifier());
		_tokenizer.advance();
		writeSymbol(_tokenizer.symbol()); // comma or semicolon
		if (_tokenizer.symbol() == ';') {
			break;
		}
	} while (true);


	_tokenizer.advance();
	writeIdentifier(_tokenizer.identifier());

	_tokenizer.advance();
}

void CompileEngine::compileSubroutine() {
	writeKeyword(_tokenizer.keyWord()); // constructor or function or method
	_tokenizer.advance();
	//void or type
	if (_tokenizer.tokenType() == TOKEN_TYPE::KEYWORD) {
		writeKeyword(_tokenizer.keyWord());
	}
	else {
		writeIdentifier(_tokenizer.identifier());
	}
	_tokenizer.advance();
	//subroutine name
	writeIdentifier(_tokenizer.identifier());
	_tokenizer.advance();
	//(
	writeSymbol(_tokenizer.symbol());
	_tokenizer.advance();
	if(_tokenizer.tokenType() != TOKEN_TYPE::SYMBOL && _tokenizer.symbol() != ')')
		compileParameterList();
	//)
	writeSymbol(_tokenizer.symbol());
	_tokenizer.advance();

	//subroutine body
	writeSymbol(_tokenizer.symbol());//{
	_tokenizer.advance();
	while (true) {
		if (_tokenizer.tokenType() == TOKEN_TYPE::KEYWORD) {
			compileVarDec();
			_tokenizer.advance();
		}
		else {
			break;
		}
	}
	compileStatements();
	writeSymbol(_tokenizer.symbol());
	_tokenizer.advance();
}

void CompileEngine::compileVarDec() {
	writeKeyword(_tokenizer.keyWord());
	_tokenizer.advance();
	writeType();
	do {
		_tokenizer.advance();
		writeIdentifier(_tokenizer.identifier());
		_tokenizer.advance();
		writeSymbol(_tokenizer.symbol());
		if (_tokenizer.symbol() == ';') {
			break;
		}
	} while (true);
	_tokenizer.advance();
}

void CompileEngine::compileParameterList() {
	do {
		writeType();
		_tokenizer.advance();
		writeIdentifier(_tokenizer.identifier());
		_tokenizer.advance();
		if (_tokenizer.symbol() == ',') {
			writeSymbol(_tokenizer.symbol());
			_tokenizer.advance();
		}
		else{
			break;
		}
	} while (true);
}

void CompileEngine::writeSymbol(const char c) {
	_ofs << "<symbol> " << c << "</symbol>" << std::endl;
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
	default:
		break;
	}
	_ofs << "<keyword> " << keywordStr << " </keyword>" << std::endl;
}

void CompileEngine::writeIdentifier(const std::string& identifier) {
	_ofs << "<identifier> " << identifier << " </identifier>" << std::endl;
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
}

void CompileEngine::compileLet() {
	writeKeyword(_tokenizer.keyWord());
	_tokenizer.advance();
	writeIdentifier(_tokenizer.identifier());
	_tokenizer.advance();
	if (_tokenizer.symbol() == '[') {
		writeSymbol(_tokenizer.symbol());
		_tokenizer.advance();
		compileExpression();
		writeSymbol(_tokenizer.symbol());
		_tokenizer.advance();
	}
	writeSymbol(_tokenizer.symbol());
	_tokenizer.advance();
	compileExpression();
	writeSymbol(_tokenizer.symbol());
	_tokenizer.advance();
}

void CompileEngine::compileIf() {
	writeKeyword(_tokenizer.keyWord());
	_tokenizer.advance();
	writeSymbol(_tokenizer.symbol());
	_tokenizer.advance();
	compileExpression();
	writeSymbol(_tokenizer.symbol());
	_tokenizer.advance();
	writeSymbol(_tokenizer.symbol());
	_tokenizer.advance();
	compileStatements();
	writeSymbol(_tokenizer.symbol());
	_tokenizer.advance();
	if (_tokenizer.tokenType() == TOKEN_TYPE::KEYWORD && _tokenizer.keyWord() == KEYWORD::ELSE) {
		writeKeyword(_tokenizer.keyWord());
		_tokenizer.advance();
		writeSymbol(_tokenizer.symbol());
		_tokenizer.advance();
		compileStatements();
		writeSymbol(_tokenizer.symbol());
		_tokenizer.advance();
	}
}

void CompileEngine::compileWhile() {
	writeKeyword(_tokenizer.keyWord());
	_tokenizer.advance();
	writeSymbol(_tokenizer.symbol());
	_tokenizer.advance();
	compileExpression();
	writeSymbol(_tokenizer.symbol());
	_tokenizer.advance();
	writeSymbol(_tokenizer.symbol());
	_tokenizer.advance();
	compileStatements();
	writeSymbol(_tokenizer.symbol());
}

void CompileEngine::compileDo() {
	writeKeyword(_tokenizer.keyWord());
	_tokenizer.advance();
	compileSubroutineCall();
	writeSymbol(_tokenizer.symbol());
	_tokenizer.advance();
}

void CompileEngine::compileSubroutineCall() {
	writeIdentifier(_tokenizer.identifier());
	_tokenizer.advance();
	writeSymbol(_tokenizer.symbol());
	_tokenizer.advance();
	compileExpressionList();
	writeSymbol(_tokenizer.symbol());
	_tokenizer.advance();
	writeIdentifier(_tokenizer.identifier());
	_tokenizer.advance();
	writeSymbol(_tokenizer.symbol());
	_tokenizer.advance();
	writeIdentifier(_tokenizer.identifier());
	_tokenizer.advance();
	writeSymbol(_tokenizer.symbol());
	_tokenizer.advance();
	compileExpressionList();
	writeSymbol(_tokenizer.symbol());
	_tokenizer.advance();
}

void CompileEngine::compileReturn() {
	writeKeyword(_tokenizer.keyWord());
	_tokenizer.advance();
	if (_tokenizer.tokenType() != TOKEN_TYPE::SYMBOL || _tokenizer.symbol() != ';') {
		compileExpression();
	}
	writeSymbol(_tokenizer.symbol());
	_tokenizer.advance();
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
		writeIntConst(_tokenizer.intVal());
		_tokenizer.advance();
		break;
	case TOKEN_TYPE::STRING_CONST:
		writeStringConst(_tokenizer.stringVal());
		_tokenizer.advance();
		break;
	case TOKEN_TYPE::KEYWORD:
		writeKeyword(_tokenizer.keyWord());
		_tokenizer.advance();
		break;
	case TOKEN_TYPE::SYMBOL:
		if (_tokenizer.symbol() == '(') {
			writeSymbol(_tokenizer.symbol());
			_tokenizer.advance();
			compileExpression();
			writeSymbol(_tokenizer.symbol());
			_tokenizer.advance();
		}
		else {
			writeSymbol(_tokenizer.symbol());
			_tokenizer.advance();
			compileTerm();
		}
		break;
	case TOKEN_TYPE::IDENTIFIER:
		writeIdentifier(_tokenizer.identifier());
		_tokenizer.advance();
		if (_tokenizer.tokenType() == TOKEN_TYPE::SYMBOL && _tokenizer.symbol() == '[') {
			writeSymbol(_tokenizer.symbol());
			_tokenizer.advance();
			compileExpression();
			writeSymbol(_tokenizer.symbol());
			_tokenizer.advance();
		}
		else if (_tokenizer.tokenType() == TOKEN_TYPE::SYMBOL && _tokenizer.symbol() == '(') {
			writeSymbol(_tokenizer.symbol());
			_tokenizer.advance();
			compileExpressionList();
			writeSymbol(_tokenizer.symbol());
			_tokenizer.advance();
			writeIdentifier(_tokenizer.identifier());
			_tokenizer.advance();
			writeSymbol(_tokenizer.symbol());
			_tokenizer.advance();
			writeIdentifier(_tokenizer.identifier());
			_tokenizer.advance();
			writeSymbol(_tokenizer.symbol());
			_tokenizer.advance();
			compileExpressionList();
			writeSymbol(_tokenizer.symbol());
			_tokenizer.advance();
		}
		break;
	case TOKEN_TYPE::NON_TOKEN:
		break;
	default:
		break;
	}
}

void CompileEngine::writeIntConst(int value) {
	_ofs << "<integerConstant> " << value << " </integerConstant>" << std::endl;
}

void CompileEngine::writeStringConst(const std::string& str) {
	_ofs << "<stringConstant> " << str << " </stringConstant>" << std::endl;
}

void CompileEngine::compileExpression() {
	compileTerm();
	while (_tokenizer.tokenType() == TOKEN_TYPE::SYMBOL && isOp(_tokenizer.symbol())) {
		writeSymbol(_tokenizer.symbol());
		_tokenizer.advance();
		compileTerm();
	}
}

void CompileEngine::compileExpressionList() {
	compileExpression();
	while (_tokenizer.tokenType() == TOKEN_TYPE::SYMBOL && _tokenizer.symbol() == ',') {
		writeSymbol(_tokenizer.symbol());
		_tokenizer.advance();
		compileExpression();
	}
}