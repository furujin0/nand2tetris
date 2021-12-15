#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

enum class CmdType{
	A_CMD,
	C_CMD,
	L_CMD,
	INVALID
};

class Parser {
private:
	std::ifstream ifs;
	std::string command;
	int address = -1;

public:
	//Parser() = delete;
	Parser(const std::string& filename);

	~Parser();

	bool hasMoreCommands();

	CmdType commandType();

	std::string symbol();

	std::string dest();

	std::string comp();

	std::string jump();

	void advance();

	std::string currentCommand();

	size_t currentAddress() {
		return address;
	}

	void goInit();
};

class Code {
public:
	static std::vector<bool> dest(const std::string& str);

	static std::vector<bool> comp(const std::string& str);

	static std::vector<bool> jump(const std::string& str);
};

class SymbolTable {
private:
	std::unordered_map<std::string, int> table;
public:
	SymbolTable();
	void addEntry(const std::string& symbol, int address);
	bool contains(const std::string& symbol);
	int getAddress(const std::string& symbol);
	void show();
};