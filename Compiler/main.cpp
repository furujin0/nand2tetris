#include "compiler.hpp"

int main(int argc, char** argv) {
	Tokenizer tokenizer("test.vm");

	tokenizer.eraseComments("testProcessed.txt");
	return 0;
}