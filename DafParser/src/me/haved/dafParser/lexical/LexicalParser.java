package me.haved.dafParser.lexical;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.util.ArrayList;

import static me.haved.dafParser.LogHelper.*;

public class LexicalParser {
	
	private File inputFile;
	private ArrayList<Token> tokens;
	
	public LexicalParser(File file) {
		this.inputFile = file;
		tokens = new ArrayList<>();
	}
	
	public void parse() {
		logAssert(inputFile.isFile(), "File given to LexicalParser doesn't exist");
		
		try {
			try(BufferedReader reader = new BufferedReader(new FileReader(inputFile))) {
				parseFromReader(reader);
			}
		}
		catch(Exception e) {
			e.printStackTrace(out);
			log(FATAL_ERROR, "LexicalParser.parse(): exception thrown");
		}
	}
	
	private void parseFromReader(BufferedReader reader) {
		while(true) {
			
		}
	}
}
