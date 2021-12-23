#include <fstream>
#include <iostream>

#include "formatter.hpp"

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