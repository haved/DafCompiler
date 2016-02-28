package me.haved.dafParser.semantic;

public class AlreadyParsingException extends RuntimeException {
	private static final long serialVersionUID = 8658820499688065383L;
	
	private String infileName;
	
	public AlreadyParsingException(String infileName) {
		this.infileName = infileName;
	}
	
	public String getInfileName() {
		return infileName;
	}
	
	public String toString() {
		return String.format("Recursive parsing was detected in the file '%s'", infileName);
	}
}
