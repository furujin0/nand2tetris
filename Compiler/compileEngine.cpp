#include <cctype>
#include <iostream>
#include <string>
#include "compileEngine.hpp"

CompileEngine::CompileEngine(
	const std::string& inputName,
	const std::string& outputName
) :_tokenizer(inputName), _writer(outputName)
{}

void CompileEngine::createSubroutineTable() {
	while (_tokenizer.hasMoreTokens()) {
		_tokenizer.advance();
		if (_tokenizer.tokenType() == TOKEN_TYPE::KEYWORD()) {
			auto keyword = _tokenizer.keyWord();
			if (keyword == KEYWORD::CONSTRUCTOR
				|| keyword == KEYWORD::FUNCTION
				|| keyword == KEYWORD::METHOD)
			{
				SUBROUTINE_TYPE func_type;
				switch (keyword) {
				case KEYWORD::CONSTRUCTOR:
					func_type = SUBROUTINE_TYPE::CONSTRUCTOR; break;
				case KEYWORD::FUNCTION:
					func_type = SUBROUTINE_TYPE::FUNCTION; break;
				case KEYWORD::METHOD:
					func_type = SUBROUTINE_TYPE::METHOD; break;
				default:
					break;
				}
				_tokenizer.advance();
				auto return_type = _tokenizer.identifier();
				_tokenizer.advance();
				auto name = _tokenizer.identifier();
				_subroutines.insert(std::make_pair(name, func_type));
			}
		}
	}
	_tokenizer.reset();
}

void CompileEngine::compileClass() {

	_tokenizer.advance(); //to class keyword
	_tokenizer.advance(); // move to class name
	
	_className = _tokenizer.identifier(); //get class name
	_tokenizer.advance();

	_tokenizer.advance(); // move from '{' to classVarDec

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

	_tokenizer.advance();
	return;
}

void CompileEngine::compileClassVarDec() {

	// determine static || field
	KIND kind = (_tokenizer.keyWord() == KEYWORD::STATIC ? KIND::STATIC : KIND::FIELD);
	
	_tokenizer.advance();
	auto type = _tokenizer.identifier();
	do {
		_tokenizer.advance(); // move to varName
		auto name = _tokenizer.identifier();
		_classSymbols.define(name, type, kind);
		_tokenizer.advance(); // move to seperator (';' or ',')
	} while (_tokenizer.symbol() != ';');

	_tokenizer.advance();
}

void CompileEngine::compileSubroutine() {
	std::cout << std::string(indent, '-') << "<subroutineDec>" << std::endl;
	_subroutineName.clear();
	indent += 2;
	_subroutineSymbols.startSubroutine();
	_ifCount = 0;
	_whileCount = 0;

	bool isMethod = (_tokenizer.keyWord() == KEYWORD::METHOD);
	auto func_type = _tokenizer.keyWord();
	_tokenizer.advance(); // move to  return type ( In Jack, every function returns value of 16bit  )
	_tokenizer.advance(); // move to subroutineName
	_subroutineName = _className + "." + _tokenizer.identifier();
	_tokenizer.advance(); // move to symbol '('
	_tokenizer.advance(); // move to parameterList
	compileParameterList(); // .
	_tokenizer.advance();// move from ')' to subroutine body

	//subroutine body
	_tokenizer.advance(); // move from '{' into statements
	while (_tokenizer.tokenType() == TOKEN_TYPE::KEYWORD && _tokenizer.keyWord() == KEYWORD::VAR) {
		compileVarDec();
	}
	////print variables definitions
	//for (auto&& s : _subroutineSymbols._table) {
	//	std::cout << s.second.name << " : " << s.second.type << std::endl;
	//}
	//for (auto&& s : _classSymbols._table) {
	//	std::cout << s.second.name << " : " << s.second.type << std::endl;
	//}

	_writer.writeFunction(_subroutineName, _subroutineSymbols.varCount(KIND::VAR));
	if (isMethod) { // set this pointer to the object
		_writer.writePush(SEG::ARG, 0);
		_writer.writePop(SEG::POINTER, 0);
	}
	else if (func_type == KEYWORD::CONSTRUCTOR) {
		_writer.writePush(SEG::CONST, _classSymbols.varCount(KIND::FIELD));
		_writer.writeCall("Memory.alloc", 1);
		_writer.writePop(SEG::POINTER, 0);
	}
	compileStatements();
	_tokenizer.advance();
	indent -= 2;
	std::cout << std::string(indent, '-') << "</subroutineDec>" << std::endl;
}

void CompileEngine::compileVarDec() {
	std::cout << std::string(indent, '-') << "<varDec>" << std::endl;
	indent += 2;
	_tokenizer.advance(); //move from var to type
	//now the current token is type
	auto type = _tokenizer.identifier();
	do {
		_tokenizer.advance(); // move to varName
		_subroutineSymbols.define(_tokenizer.identifier(), type, KIND::VAR);
		_tokenizer.advance(); // move to a separator ',' or ';'
	} while (_tokenizer.symbol() != ';');
	_tokenizer.advance();
	indent -= 2;
	std::cout << std::string(indent, '-') << "</varDec>" << std::endl;
}


void CompileEngine::compileParameterList() {
	std::cout << std::string(indent, '-')<< "<parameter list>" << std::endl;
	indent += 2;

	while (_tokenizer.tokenType() != TOKEN_TYPE::SYMBOL || _tokenizer.symbol() != ')')
	{
		auto type = _tokenizer.identifier();
		_tokenizer.advance(); // move from type to name
		_subroutineSymbols.define(_tokenizer.identifier(), type, KIND::ARG);
		_tokenizer.advance();//move from name to ','
		if (_tokenizer.tokenType() == TOKEN_TYPE::SYMBOL && _tokenizer.symbol() == ')') {
			break;
		}
		else {
			_tokenizer.advance();
		}
	}
	indent -= 2;
	std::cout << std::string(indent, '-') << "</parameter list>" << std::endl;
}


bool CompileEngine::isBuiltInType(const std::string& type) {
	return type == "int" || type == "char" || type == "boolean";
}

void CompileEngine::compileStatements() {
	std::cout << std::string(indent, '-') << "<satements>" << std::endl;
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
				std::cout << "No statements remain. The current token is:" << _tokenizer.token() << std::endl;
				hasMoreStatements = false; break;
			}
		}
		else {
			std::cout << "No statements remain. The current token is:" << _tokenizer.token() << std::endl;

			hasMoreStatements = false;
		}
	}
	indent -= 2;
	std::cout << std::string(indent, '-') << "</statements>" << std::endl;
}

void CompileEngine::compileLet() {
	std::cout << std::string(indent, '-') << "<letStatement>" << std::endl;
	indent += 2;
	//determine varName and its kind
	_tokenizer.advance(); // move from "let" to varName;
	auto varName = _tokenizer.identifier();
	auto kind = (_subroutineSymbols.isDefined(varName) ? _subroutineSymbols.kindOf(varName) : _classSymbols.kindOf(varName));
	auto type = (_subroutineSymbols.isDefined(varName) ? _subroutineSymbols.typeOf(varName) : _classSymbols.typeOf(varName));
	auto index = (_subroutineSymbols.isDefined(varName) ? _subroutineSymbols.indexOf(varName) : _classSymbols.indexOf(varName));
	SEG seg;
	switch (kind) {
	case KIND::STATIC:
		seg = SEG::STATIC; break;
	case KIND::FIELD:
		seg = SEG::THIS; break;
	case KIND::ARG:
		seg = SEG::ARG; break;
	case KIND::VAR:
		seg = SEG::LOCAL; break;
	}

	_tokenizer.advance();// move from varName to a symbol
	if (_tokenizer.tokenType() == TOKEN_TYPE::SYMBOL && _tokenizer.symbol() == '[') {
		_tokenizer.advance();//move from '[' to expression
		compileExpression();
		_writer.writePush(seg, index);
		_writer.writeArithmetic(CMD::ADD);
		_tokenizer.advance(); // move from ']' to '='
		_tokenizer.advance(); // move from '=' to expression
		compileExpression();
		_writer.writePop(SEG::TEMP, 0);
		_writer.writePop(SEG::POINTER, 1);
		_writer.writePush(SEG::TEMP, 0);
		_writer.writePop(SEG::THAT, 0);
	}
	else {
		_tokenizer.advance(); //move from '=' to expresssion
		compileExpression();
		_writer.writePop(seg, index);
	}
	_tokenizer.advance();// move from ';'
	indent -= 2;
	std::cout << std::string(indent, '-') << "</letStatement>" << std::endl;

}

void CompileEngine::compileIf() {
	
	auto false_label = _subroutineName + "_false_" + std::to_string(_ifCount);
	auto return_label = _subroutineName + "_return_if_" + std::to_string(_ifCount);
	_ifCount++;
	_tokenizer.advance();
	_tokenizer.advance();
	compileExpression();
	_writer.writeArithmetic(CMD::NOT);
	_writer.writeIf(false_label);
	_tokenizer.advance();
	_tokenizer.advance();
	compileStatements();
	_writer.writeGoto(return_label);
	_tokenizer.advance();
	_writer.writeLabel(false_label);
	if (_tokenizer.tokenType() == TOKEN_TYPE::KEYWORD && _tokenizer.keyWord() == KEYWORD::ELSE) {
		_tokenizer.advance();
		_tokenizer.advance();
		compileStatements();
		_writer.writeGoto(return_label);
		_tokenizer.advance();
	}
	_writer.writeLabel(return_label);
}

void CompileEngine::compileWhile() {
	std::cout << std::string(indent, '-') << "<whileStatement>" << std::endl;
	indent += 2;
	auto whileLabel = _subroutineName + "_while_" + std::to_string(_whileCount);
	auto returnLabel = _subroutineName + "_return_while_" + std::to_string(_whileCount);
	_whileCount++;
	_tokenizer.advance();// move from while to '('
	_tokenizer.advance(); //move from '(' to expression
	_writer.writeLabel(whileLabel);
	compileExpression();
	_writer.writeArithmetic(CMD::NOT);
	_tokenizer.advance();//move from ')' to '{'
	_tokenizer.advance();//move from '{' to statements
	_writer.writeIf(returnLabel);
	compileStatements(); // move from statements to '}'
	_writer.writeGoto(whileLabel);
	_writer.writeLabel(returnLabel);
	_tokenizer.advance(); //move from '}'
	indent -= 2;

	std::cout << std::string(indent, '-') << "</whileStatement>" << std::endl;
}

void CompileEngine::compileDo() {
	std::cout << std::string(indent, '-') << "<doStatement>" << std::endl;
	indent += 2;

	_tokenizer.advance();
	compileSubroutineCall();
	_tokenizer.advance(); // move from ';'
	indent -= 2;
	std::cout << std::string(indent, '-') << "</doStatement>" << std::endl;
}

void CompileEngine::compileSubroutineCall() {
	auto firstName = _tokenizer.identifier();
	std::string fullName;
	int numArgs = 0;
	_tokenizer.advance(); // move to '.' (if another's subroutine) or '(' (if own subroutine)
	if (_tokenizer.symbol() == '.') {// the subroutine of another class is called
		_tokenizer.advance();
		auto subroutineName = _tokenizer.identifier();
		_tokenizer.advance(); //move from subroutineName to (
	
		if (_subroutineSymbols.isDefined(firstName) || _classSymbols.isDefined(firstName)) { // the subroutine is method
			auto className = (_subroutineSymbols.isDefined(firstName) ? _subroutineSymbols.typeOf(firstName) : _classSymbols.typeOf(firstName));
			fullName = className + "." + subroutineName;
			numArgs++;
			auto idx = (_subroutineSymbols.isDefined(firstName) ? _subroutineSymbols.indexOf(firstName) : _classSymbols.indexOf(firstName));
			auto kind = (_subroutineSymbols.isDefined(firstName) ? _subroutineSymbols.kindOf(firstName) : _classSymbols.kindOf(firstName));
			auto type  = (_subroutineSymbols.isDefined(firstName) ? _subroutineSymbols.typeOf(firstName) : _classSymbols.typeOf(firstName));
			SEG seg;
			switch (kind) {
			case KIND::STATIC:
				seg = SEG::STATIC; break;
			case KIND::ARG:
				seg = SEG::ARG; break;
			case KIND::FIELD:
				seg = SEG::THIS; break;
			case KIND::VAR:
				seg = SEG::LOCAL; break;
			default:
				break;
			}
			_writer.writePush(seg, idx);

		}
		else {//else the subroutine is function
			fullName = firstName + "." + subroutineName;
		}
	}
	else { // a subroutine of the own class is called tokenizer.token should be '('
		fullName = _className + "." + firstName;
		auto a = _subroutines.find(fullName);
		if (_subroutines.find(fullName) -> second == SUBROUTINE_TYPE::METHOD) { // the subroutine is method
			numArgs++;
			_writer.writePush(SEG::POINTER, 0);
		}
	}

	_tokenizer.advance(); //move from ( to expressionList
	numArgs += compileExpressionList();
	_tokenizer.advance(); // move from ')' 
	_writer.writeCall(fullName, numArgs);
}

void CompileEngine::compileReturn() {
	_tokenizer.advance();
	if (_tokenizer.tokenType() != TOKEN_TYPE::SYMBOL || _tokenizer.symbol() != ';') { 
		compileExpression();
	}
	else {
		_writer.writePush(SEG::CONST, 0);
	}
	_writer.writeReturn();
	_tokenizer.advance();
}

bool CompileEngine::isOp(char c) {
	return c == '+' || c == '-' || c == '*' || c == '/' || c == '&' || c == '|' || c == '<' || c == '>' || c == '=';
}

bool CompileEngine::isUnaryOp(char c) {
	return c == '+' || c == '-';
}

void CompileEngine::compileTerm() {
	std::cout << std::string(indent, '-') << "<term>" << std::endl;
	indent += 2;
	switch (_tokenizer.tokenType()) {
	case TOKEN_TYPE::INT_CONST:
		_writer.writePush(SEG::CONST, _tokenizer.intVal());
		_tokenizer.advance();
		break;
	case TOKEN_TYPE::STRING_CONST:
		_writer.writePush(SEG::CONST, _tokenizer.stringVal().size());
		_writer.writeCall("String.new", 1);

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
			_writer.writeArithmetic(CMD::NOT);
			break;
		case KEYWORD::FALSE: //push 0
			_writer.writePush(SEG::CONST, 0);
			break;
		case KEYWORD::NULL_WORD:
			_writer.writePush(SEG::CONST, 0);
			break;
		case KEYWORD::THIS:
			_writer.writePush(SEG::POINTER, 0);
		default:
			break;
		}
		_tokenizer.advance();
		break;
	case TOKEN_TYPE::SYMBOL:
		if (_tokenizer.symbol() == '(') { // expression
			_tokenizer.advance(); // move to expression
			compileExpression();
			_tokenizer.advance(); // move from ')'
		}
		else { // unary operation
			auto unaryOp = _tokenizer.symbol();
			_tokenizer.advance(); //move to term
			compileTerm();
			if (unaryOp == '-')
				_writer.writeArithmetic(CMD::NEG);
			else if (unaryOp == '~')
				_writer.writeArithmetic(CMD::NOT);
		}
		break;
	case TOKEN_TYPE::IDENTIFIER:

		if (!_subroutineSymbols.isDefined(_tokenizer.identifier()) && !_classSymbols.isDefined(_tokenizer.identifier())) // subroutine
		{
			compileSubroutineCall();
		}	
		else // varName or varName[expression] or varName.method
		{
			auto name = _tokenizer.identifier();
			auto index = _subroutineSymbols.isDefined(name) ? _subroutineSymbols.indexOf(name) : _classSymbols.indexOf(name);
			auto kind = _subroutineSymbols.isDefined(name) ? _subroutineSymbols.kindOf(name) : _classSymbols.kindOf(name);
			auto type = _subroutineSymbols.isDefined(name) ? _subroutineSymbols.typeOf(name) : _classSymbols.typeOf(name);
			SEG seg;
			switch (kind) {
			case KIND::STATIC:
				seg = SEG::STATIC; break;
			case KIND::FIELD:
				seg = SEG::THIS; break;
			case KIND::ARG:
				seg = SEG::ARG; break;
			case KIND::VAR:
				seg = SEG::LOCAL; break;
			}
			_writer.writePush(seg, index);
			_tokenizer.advance(); // move from name
			if (_tokenizer.tokenType() == TOKEN_TYPE::SYMBOL && _tokenizer.symbol() == '[') { // array-type variable
				_tokenizer.advance(); // move from '[' to expression
				compileExpression();
				_writer.writeArithmetic(CMD::ADD);
				_writer.writePop(SEG::POINTER, 1);
				_writer.writePush(SEG::THAT, 0);
				_tokenizer.advance(); // move from ']'
			}
			else if (_tokenizer.tokenType() == TOKEN_TYPE::SYMBOL && _tokenizer.symbol() == '.') { // given that field is private, this must be varName.method
				_tokenizer.advance(); // move from . to methodName;
				
				std::string methodName = _tokenizer.identifier();
				_tokenizer.advance(); // move from methodName to '('
				_tokenizer.advance(); //move from ( to expressionList
				int numArgs = compileExpressionList();
				_tokenizer.advance(); // move from ')' 
				_writer.writeCall(type + "." + methodName, numArgs + 1);
			}
		}
		break;
	case TOKEN_TYPE::NON_TOKEN:
		break;
	default:
		break;
	}
	indent -= 2;
	std::cout << std::string(indent, '-') << "</term>" << std::endl;
}

void CompileEngine::compileExpression() {
	std::cout << std::string(indent, '-') << "<expresion>" << std::endl;
	indent += 2;

	compileTerm();
	while (_tokenizer.tokenType() == TOKEN_TYPE::SYMBOL && isOp(_tokenizer.symbol())) {
		auto sym = _tokenizer.symbol();
		_tokenizer.advance();
		compileTerm();
		switch (sym)
		{
		case '+':
			_writer.writeArithmetic(CMD::ADD); break;
		case '-':
			_writer.writeArithmetic(CMD::SUB); break;
		case '*':
			_writer.writeCall("Math.multiply", 2); break;
		case '/':
			_writer.writeCall("Math.divide", 2); break;
		case '&':
			_writer.writeArithmetic(CMD::AND); break;
		case '|':
			_writer.writeArithmetic(CMD::OR); break;
		case '<':
			_writer.writeArithmetic(CMD::LT); break;
		case '>':
			_writer.writeArithmetic(CMD::GT); break;
		case '=':
			_writer.writeArithmetic(CMD::EQ); break;
		default:
			break;
		}
	}
	indent -= 2;
	std::cout << std::string(indent, '-') << "</expression>" << std::endl;
}

int CompileEngine::compileExpressionList() {
	std::cout << std::string(indent, '-') << "<expressionList>" << std::endl;
	indent += 2;

	int numExp = 0;
	if (_tokenizer.tokenType() != TOKEN_TYPE::SYMBOL || _tokenizer.symbol() != ')') {
		numExp++;
		compileExpression();
		while (_tokenizer.tokenType() == TOKEN_TYPE::SYMBOL && _tokenizer.symbol() == ',') {
			numExp++;
			_tokenizer.advance();
			compileExpression();
		}
	}
	indent -= 2;
	std::cout << std::string(indent, '-') << "</expressionList>" << std::endl;
	return numExp;
}



