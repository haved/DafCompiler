#include "parsing/Lexer.h"
#include "DafLogger.h"
#include <iostream>

Lexer::Lexer(const FileForParsing& file) : fileForParsing(file), currentToken(token1), lookaheadToken(token2) {
    infile.open(file.inputFile.string()); //For the time being, there is no text processor
    currentChar = '\n'; //Not too nice, but oh well
    lookaheadChar = '\n';
    advanceChar(); //To set look-ahead char
    advanceChar(); //To set current char
    line = 1;
    col = 1;
    advance(); //To make look-ahead an actual token
    advance(); //To make current an actual token
}

bool isWhitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

inline bool isEOF(char c) {
    return c == EOF;
}

bool isStartOfText(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

bool isPartOfText(char c) {
    return isStartOfText(c) || isDigit(c);
}

bool isLegalSpecialChar(char c) {
    return (c >= '!' && c <= '/') || (c >= ':' && c <= '@') || (c >= '[' && c <= '_') || (c >= '{' && c <= '}');
}

bool Lexer::advance() {
    std::swap(currentToken, lookaheadToken);
    //Now set the look-ahead token
    do {
        while(true) {
            if(isWhitespace(currentChar)) {
                advanceChar();
                continue;
            }
            else if(isEOF(currentChar)) {
                setProperEOFToken(lookaheadToken, line, col);
                break;
            }
            else {
                if(isStartOfText(currentChar)) {
                    std::string word;
                    word.push_back(currentChar);
                    int startCol = col;
                    while(true) {
                        advanceChar();
                        if(!isPartOfText(currentChar))
                            break;
                        word.push_back(currentChar);
                    }
                    if(!setTokenFromWord(lookaheadToken, word, line, startCol, col)) {
                        logDafC(fileForParsing, line, startCol, ERROR) << "Token '" << word << "' not recognized" << std::endl;
                        continue;
                    }
                    break;
                }
                else if(isLegalSpecialChar(currentChar)) {
                    char c = currentChar;
                    advanceChar();
                    if(!setTokenFromSpecialChar(lookaheadToken, c, line, col)) {
                        logDafC(fileForParsing, line, col, ERROR) << "Special char '" << currentChar << "' not a token" << std::endl;
                        continue;
                    }
                    break;
                } else {
                    logDafC(fileForParsing, line, col, ERROR) << "Character not legal: " << currentChar << std::endl;
                    advanceChar();
                    continue;
                }
            }
            assert(false); //Make sure we always end properly
        }
    }
    while(mergeTokens(currentToken, lookaheadToken));
    return currentToken.type != END;
}

void Lexer::advanceChar() {
    if(currentChar == '\n') {
        line++;
        col = 1;
    } else if(currentChar == '\t')
        col += 4;
    else
        col++;

    currentChar = lookaheadChar;
    if(!infile.get(lookaheadChar))
        lookaheadChar = EOF;

    if(currentChar == '/') {
        if(lookaheadChar == '/') {
            while(currentChar != EOF && currentChar != '\n') {
                advanceChar();
            }
            //We stay at the \n or EOF
        }
        else if(lookaheadChar == '*') {
            while(currentChar != '*' || lookaheadChar != '/') {
                advanceChar();
                if(currentChar == EOF) {
                    logDafC(fileForParsing, line, col, ERROR) << "File ended during multi-line comment" << std::endl;
                    break;
                }
            }
            advanceChar(); //Skip the '*'
            advanceChar(); //Skip the '/' too
        }
    }
}

