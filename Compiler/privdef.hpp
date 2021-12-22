#pragma once

#include <set>
#include <string>
#include <unordered_map>

enum class TOKEN_TYPE{
	KEYWORD,
	SYMBOL,
	IDENTIFIER,
	INT_CONST,
	STRING_CONST,
	NON_TOKEN
};

enum class KEYWORD {
	CLASS,
	METHOD,
	FUNCTION,
	CONSTRUCTOR,
	INT,
	VAR,
	STATIC,
	FIELD,
	LET,
	DO,
	IF,
	ELSE,
	WHILE,
	RETURN,
	TRUE,
	FALSE,
	NULL_WORD,
	THIS,
	VOID
};