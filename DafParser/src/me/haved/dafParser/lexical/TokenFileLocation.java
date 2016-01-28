package me.haved.dafParser.lexical;

import me.haved.dafParser.LogHelper;

public class TokenFileLocation implements TokenLocation {
	String fileName;
	int lineNumber;
	int columnNumber;
	
	public TokenFileLocation(String fileName, int line, int column) {
		this.fileName = fileName;
		this.lineNumber = line;
		this.columnNumber = column;
	}
	
	public String getErrorString() {
		return LogHelper.fileLocation(fileName, lineNumber, columnNumber);
	}
}
