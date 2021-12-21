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

bool Tokenizer::isSymbol(char c) const{
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

bool Tokenizer::isIdentifier(const std::string& str) const{
	if (std::isdigit(str[0]))
		return false;

	for (auto&& c : str) {
		if (!std::isalpha(c) && c !='_' && !std::isdigit(c))
			return false;
	}
	return true;
}

KEYWORD Tokenizer::keyWord() const{
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