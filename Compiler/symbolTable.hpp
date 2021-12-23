#pragma once

#include <string>
#include <unordered_map>
#include "privdef.hpp"

struct IdInfo {
	std::string name;
	std::string type;
	int index;
	ID_KIND kind;
};

class SymbolTable {

	std::unordered_map<std::string, IdInfo> _table;
public:
	SymbolTable();

	void startSubroutine();

	int define(
		const std::string& name,
		const std::string& type,
		ID_KIND kind
	);

	int varCount(ID_KIND kind);

	ID_KIND kindOf(const std::string& name);

	std::string typeOf(const std::string& name);

	int indexOf(const std::string& name);
};