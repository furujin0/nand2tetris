#include <cctype>
#include <iostream>
#include <string>
#include "compileEngine.hpp"

CompileEngine::CompileEngine(
	const std::string& inputName,
	const std::string& outputName
) :_tokenizer(inputName), _writer(outputName)
{}
/**/
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
	std::cout << "<subroutineDec>" << std::endl;
	_subroutineName.clear();
	indent += 2;
	_subroutineSymbols.startSubroutine();
	_ifCount = 0;
	_whileCount = 0;
	bool isMethod = (_tokenizer.keyWord() == KEYWORD::METHOD);
	_tokenizer.advance(); // move to  return type ( In Jack, every function returns value of 16bit  )
	_tokenizer.advance(); // move to subroutineName
	_subroutineName = _className + "." + _tokenizer.identifier();
	std::cout << "_subroutineName:" << _subroutineName << std::endl;
	_tokenizer.advance(); // move to symbol '('
	_tokenizer.advance(); // move to parameterList
	compileParameterList(); // .
	_tokenizer.advance();// move from ')' to subroutine body

	//subroutine body
	_tokenizer.advance(); // move from '{' into statements
	while (_tokenizer.tokenType() == TOKEN_TYPE::KEYWORD && _tokenizer.keyWord() == KEYWORD::VAR) {
		compileVarDec();
	}
	//print variables definitions
	for (auto&& s : _subroutineSymbols._table) {
		std::cout << s.second.name << " : " << s.second.type << std::endl;
	}
	for (auto&& s : _classSymbols._table) {
		std::cout << s.second.name << " : " << s.second.type << std::endl;
	}

	_writer.writeFunction(_subroutineName, _subroutineSymbols.varCount(KIND::VAR));
	_subroutines.insert(std::make_pair(
		_subroutineName,
		FuncInfo(
			_subroutineName,
			_subroutineSymbols.varCount(KIND::ARG), 
			_subroutineSymbols.varCount(KIND::VAR), 
			(isMethod ? SUBROUTINE_TYPE::METHOD : SUBROUTINE_TYPE::FUNCTION)))
	);
	compileStatements();
	_tokenizer.advance();
	indent -= 2;
	std::cout << std::string(indent, '-') << "</subroutineDec>" << std::endl;
}

void CompileEngine::compileVarDec() {
	_tokenizer.advance(); //move from var to type
	//now the current token is type
	auto type = _tokenizer.identifier();
	do {
		_tokenizer.advance(); // move to varName
		_subroutineSymbols.define(_tokenizer.identifier(), type, KIND::VAR);
		_tokenizer.advance(); // move to a separator ',' or ';'
	} while (_tokenizer.symbol() != ';');
	_tokenizer.advance();
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
	//determine varName and its kind
	_tokenizer.advance(); // move to varName;
	auto varName = _tokenizer.identifier();
	_tokenizer.advance();
	_tokenizer.advance(); // move from '=' to expression
	compileExpression();

	auto kind = (_subroutineSymbols.isDefined(varName) ? _subroutineSymbols.kindOf(varName) : _classSymbols.kindOf(varName));
	auto type = (_subroutineSymbols.isDefined(varName) ? _subroutineSymbols.typeOf(varName) : _classSymbols.typeOf(varName));
	auto index = (_subroutineSymbols.isDefined(varName) ? _subroutineSymbols.indexOf(varName) : _classSymbols.indexOf(varName) );

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

	if (_tokenizer.symbol() == '[') { // variable is array-type
		_writer.writePush(seg, index);
		_tokenizer.advance();//move from '[' to expression
		compileExpression();
		_writer.writeArithmetic(CMD::ADD);
		_writer.writePop(SEG::POINTER, 1);
		_writer.writePop(SEG::THAT, 0);
		_tokenizer.advance(); // move expression to ']'
		_tokenizer.advance(); //move from ']' to ';'
		_writer.writePop(SEG::CONST, 0);
	}
	else { // variable is non-array type
		_writer.writePop(seg, index);
	}
	_tokenizer.advance(); //move from ';'
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
	_tokenizer.advance();
	_tokenizer.advance();
	_writer.writeIf(returnLabel);
	compileStatements();
	_writer.writeGoto(whileLabel);
	_writer.writeLabel(returnLabel);
	_tokenizer.advance();
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
	_numArgs = 0;
	auto firstName = _tokenizer.identifier();
	std::string fullName;
	_tokenizer.advance();
	if (_tokenizer.symbol() == '.') {// the subroutine of another class is called
		_tokenizer.advance();
		auto subroutineName = _tokenizer.identifier();
		_tokenizer.advance(); //move from move to (
		fullName = firstName + "." + subroutineName;
		if (_subroutineSymbols.isDefined(firstName) || _classSymbols.isDefined(firstName)) { // the subroutine is method
			_numArgs++;
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
		//else the subroutine is function
	}
	else { // a subroutine of the own class is called tokenizer.token should be '('
		fullName = _className + "." + firstName;
		if (_subroutines.find(firstName)->second.type == SUBROUTINE_TYPE::METHOD) { // the subroutine is method
			_numArgs++;
			_writer.writePush(SEG::POINTER, 0);
		}
	}

	_tokenizer.advance(); //move from ( to expressionList
	compileExpressionList();
	_tokenizer.advance(); // move from ')' 
	_writer.writeCall(fullName, _numArgs);
}

void CompileEngine::compileReturn() {
	_tokenizer.advance();
	if (_tokenizer.tokenType() != TOKEN_TYPE::SYMBOL || _tokenizer.symbol() != ';') { 
		compileExpression();
	}
	else {
		_writer.writePush(SEG::CONST, 0);
	}
	_writer.writePop(SEG::ARG, 0);
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
			_writer.writeArithmetic(CMD::NEG);
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
		else // varName or varName[expression]
		{
			auto name = _tokenizer.identifier();
			auto index = _subroutineSymbols.isDefined(name) ? _subroutineSymbols.indexOf(name) : _classSymbols.indexOf(name);
			auto kind = _subroutineSymbols.isDefined(name) ? _subroutineSymbols.kindOf(name) : _classSymbols.kindOf(name);
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
				_tokenizer.advance();
				compileExpression();
				_writer.writeArithmetic(CMD::ADD);
				_writer.writePop(SEG::POINTER, 1);
				_writer.writePush(SEG::THAT, 0);
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

void CompileEngine::compileExpressionList() {
	std::cout << std::string(indent, '-') << "<expressionList>" << std::endl;
	indent += 2;

	if (_tokenizer.tokenType() != TOKEN_TYPE::SYMBOL || _tokenizer.symbol() != ')') {
		_numArgs++;
		compileExpression();
		while (_tokenizer.tokenType() == TOKEN_TYPE::SYMBOL && _tokenizer.symbol() == ',') {
			_numArgs++;
			_tokenizer.advance();
			compileExpression();
		}
	}
	indent += 2;
	std::cout << std::string(indent, '-') << "</expressionList>" << std::endl;
}



