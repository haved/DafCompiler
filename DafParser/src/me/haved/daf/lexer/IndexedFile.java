package me.haved.daf.lexer;

import java.io.File;

import static me.haved.daf.LogHelper.*;

public class IndexedFile {
	private String infileName;
	private char[] letters;
	private int[][] letterIndex;
	
	private IndexedFile(String infileName) {
		this.infileName = infileName;
	}
	
	private IndexedFile(String infileName, char[] letters, int[][] letterIndex) {
		this.infileName = infileName;
		this.letters = letters;
		this.letterIndex = letterIndex;
	}
	
	public static IndexedFile indexFile(File inputFile, String infileName) {
		try {
			StringBuilder text = new StringBuilder("\n"); //Start of with a newline
			
			
			
			char[] letters = new char[0];
			int[][] letterIndex = new int[0][2];
			IndexedFile file = new IndexedFile(infileName, letters, letterIndex);
			
			return file;
		} catch (Exception e) {
			e.printStackTrace();
			log(ERROR, "Failed to read the file '%s'");
		}
		return new IndexedFile(infileName);
	}
}
