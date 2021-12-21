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


bool Tokenizer::isNonTokenChar(char c) const {
	return
		c == ' '
		|| c == '\n'
		|| c == '\r'
		|| symbolSet.find(c) != symbolSet.end()
		|| c == std::char_traits<char>::eof();
}