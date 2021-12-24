#pragma once

#include <string>
#include <unordered_map>
#include "privdef.hpp"

struct IdInfo {
	std::string name;
	std::string type;
	KIND kind;
	int index;
};

struct FuncInfo {
	std::string name;
	int numArgs;
	int numLocals;
};

class SymbolTable {

	int _next_idx = 0;
	std::unordered_map<std::string, IdInfo> _table;
public:

	void startSubroutine();

	void define(
		const std::string& name,
		const std::string& type,
		KIND kind
	);

	int varCount(KIND kind) const;

	KIND kindOf(const std::string& name) const;

	std::string typeOf(const std::string& name) const;

	int indexOf(const std::string& name) const;

	bool isDefined(const std::string& name) const;
};