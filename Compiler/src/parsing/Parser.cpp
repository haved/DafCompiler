#include "parsing/Parser.hpp"
#include "parsing/lexing/Lexer.hpp"
#include "parsing/lexing/ArgHandler.hpp"
#include "DafLogger.hpp"
#include "parsing/NameScopeParser.hpp"

#include <iostream>

void parseFileSyntax(FileForParsing& ffp) {
	Lexer lexer(ffp);
	parseFileAsNameScope(lexer, &ffp.m_nameScope);
	assert(ffp.m_nameScope);
	ffp.m_nameScope->printSignature();
	std::cout << std::endl;
	terminateIfErrors();
}
