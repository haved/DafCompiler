package me.haved.dafParser.lexical;

public class TokenFileLocation implements TokenLocation {
	String fileName;
	int lineNumber;
	int linePosition;
	
	public TokenFileLocation(String fileName, int lineNumber, int linePosition) {
		this.fileName = fileName;
		this.lineNumber = lineNumber;
		this.linePosition = linePosition;
	}
	
	public String getErrorString() {
		return String.format("%s:%d:%d", fileName, lineNumber, linePosition);
	}
}
