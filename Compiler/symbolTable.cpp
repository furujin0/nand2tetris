#include "symbolTable.hpp"

void SymbolTable::define(
	const std::string& name,
	const std::string& type,
	KIND kind
){
	IdInfo info{ name, type, kind, varCount(kind)};
	_table.insert(std::make_pair(name,  info));
}

void SymbolTable::startSubroutine() {
	_table.clear();
}

int SymbolTable::varCount(KIND kind) const {
	int count = 0;
	for (auto&& var : _table) {
		if (var.second.kind == kind) {
			count++;
		}
	}
	return count;
}

KIND SymbolTable::kindOf(const std::string& name) const {
	return _table.at(name).kind;
}

std::string SymbolTable::typeOf(const std::string& name) const {
	return _table.at(name).type;
}

int SymbolTable::indexOf(const std::string& name) const {
	return _table.at(name).index;
}

bool SymbolTable::isDefined(const std::string& name) const {
	return _table.find(name) != _table.end();
}