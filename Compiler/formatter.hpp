#pragma once

#include <string>

class Formatter {
public:
	void eraseComments(
		const std::string& inputName,
		const std::string& outputName
	) const;
};