#include <iostream>
#include "compiler.hpp"

void Tokenizer::eraseComments() {
	std::ofstream ofs(_inputName + ".tmp.txt");
	char c0, c1;
	c0 = _ifs.get();
	c1 = _ifs.get();
	while (!_ifs.eof()) {
		if (c0 == '/' && c1 == '/') {
			while (c1 != '\n' && !_ifs.eof()) {
				c0 = c1;
				c1 = _ifs.get();
			}
		}
		else if (c0 == '/' && c1 == '*') {
			while (!(c0 == '*' && c1 == '/') && !_ifs.eof()) {
				c0 = c1;
				c1 = _ifs.get();
			}
			c1 = _ifs.get();
		}
		else {
			ofs << c0;
		}
		c0 = c1;
		c1 = _ifs.get();
		if (_ifs.eof()) {
			if (std::char_traits<char>::not_eof(c0)) {
				ofs << c0;
			}
			if (std::char_traits<char>::not_eof(c1)) {
				std::cout << std::char_traits<char>::not_eof(c1);
				ofs << c1;
			}
		}
	}
	// normal end: c1 = last character c0 : already printed
	// end with */ : c1 = traits::eof c0 = traits::eof
	// end with // comment : c1 = traits::eof c0: last letter of the comment

	ofs.close();
	_ifs.clear();
	_ifs.seekg(0, std::ios::beg);
	
}


Tokenizer::Tokenizer(const std::string& inputName)
:_inputName(inputName) {
	_ifs.open(inputName);

}