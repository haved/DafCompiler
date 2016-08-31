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
    while(true) {
        if(isWhitespace(currentChar));
        else if(isEOF(currentChar)) {
            lookaheadToken.type = END;
            break;
        }
        else {
            bool specialChars = true;
            if(isStartOfText(currentChar)) {
                specialChars = false;
            }
            else if(!isLegalSpecialChar(currentChar)) {
                logDafC(fileForParsing, line, col, ERROR) << "Character not legal: " << currentChar << std::endl;
                advanceChar();
                continue;
            }
            std::string word;
            word.push_back(currentChar);
            while(true) {
                advanceChar();
                if((specialChars && !isLegalSpecialChar(currentChar)) || (!specialChars && !isPartOfText(currentChar)))
                    break;
                word.push_back(currentChar);
            }
            std::cout << "Found word: " << word << std::endl;
            break;
        }
        advanceChar();
    }
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

