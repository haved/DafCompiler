package me.haved.daf.lexer;

import java.io.File;
import java.io.FileReader;

import static me.haved.daf.LogHelper.*;

public class IndexedFile {
	public String infileName;
	public char[] letters;
	public int[] lineNums;
	public int[] colNums; 
	
	private IndexedFile(String infileName) {
		this.infileName = infileName;
		letters  = new char[0];
		lineNums = new int [0];
		colNums  = new int [0];
	}
	
	private IndexedFile(String infileName, char[] letters, int[] lineNums, int[] colNums) {
		this.infileName = infileName;
		this.letters    = letters;
		this.lineNums   = lineNums;
		this.colNums    = colNums;
	}
	
	public String getErrorLocation(int index) {
		return String.format("%s:%d:%d", infileName, lineNums[index], colNums[index]);
	}
	
	public static IndexedFile indexFile(File inputFile, String infileName) {
		try {
			StringBuilder text = new StringBuilder("\n"); //Start of with a newline
			
			try(FileReader reader = new FileReader(inputFile)) {
				int c;
				while((c=reader.read()) != -1) {
					text.append((char) c);
				}
				reader.close();
			}
			
			text.append('\n');
			
			char[] letters = new char[text.length()];
			text.getChars(0, letters.length, letters, 0);
			
			int[] lineNums = new int[letters.length];
			int[] colNums  = new int[letters.length];
			
			int line = 0;
			int col = 0;
			
			for(int i = 0; i < letters.length; i++) {
				lineNums[i] = line;
				colNums [i] = col;
				
				if(letters[i]=='\n') {
					line++;
					col = 1;
				} else
					col++;
			}
			
			return new IndexedFile(infileName, letters, lineNums, colNums);
			
		} catch (Exception e) {
			e.printStackTrace();
			log(ERROR, "indexFile(%s) Failed to read the file", infileName);
		}
		return new IndexedFile(infileName);
	}
}
