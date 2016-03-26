package me.haved.daf.lexer;

import java.io.IOException;

import me.haved.daf.args.MacroOption.AddMacroCall;

import static me.haved.daf.LogHelper.*;

public class FileCodeSupplier implements Supplier {
	
	private static int DEFAULT_CHAR_BUFFER_LENGTH = 30; // A macro will quickly destroy this
	private static float ARRAY_EXPAND_FACTOR = 1.5f;
	
	private FileTextSupplier fileText;
	private MacroMap macros;
	private char[] chars;
	private int[] lineNums;
	private int[] colNums;
	
	private int currentBase = 0;
	private int currentChar = 0;
	private int length = 0;
	
	public FileCodeSupplier(FileTextSupplier fileText, MacroMap macros) throws Exception {
		this.fileText = fileText;
		this.macros = macros;
		this.chars = new char[DEFAULT_CHAR_BUFFER_LENGTH];
		this.lineNums = new int[DEFAULT_CHAR_BUFFER_LENGTH];
		this.colNums = new int[DEFAULT_CHAR_BUFFER_LENGTH];
	
		if(!fileText.hasChar())
			if(!fileText.advance())
				log(FATAL_ERROR, "FileCodeSupplier was given a completly empty: %s", fileText.toString());
		
		if(!fileText.hasChar()) //Now it definitely *should* have one
			log(FATAL_ERROR, "FileCodeSupplier was given broken %s", fileText.toString());
		if(!appendChar(fileText.getCurrentChar(), fileText.getCurrentLine(), fileText.getCurrentCol()))
			log(FATAL_ERROR, "FileCodeSupplier was given a FileTextSupplier that refued to give it's promised char away!");
		if(!fileText.advance())
			log(FATAL_ERROR, "FileCodeSupplier was given a one char long unadvanceable %s", fileText.toString());
	}

	private boolean appendChar(char c, int line, int col) {
		if(length >= chars.length) {
			if(currentBase > 0) {
				deleteUnusedBuffer();
			} else {
				makeBufferBigger();
			}
		}
		if(length >= chars.length) //Our size making measures failed
			return false;
		chars   [length] = c;
		lineNums[length] = line;
		colNums [length] = col;
		length++;
		return true;
	}
	
	private void deleteUnusedBuffer() {
		int newLength = length - currentBase;
		currentChar -= currentBase;
		log(SUPER_DEBUG, "Moving chars in FileCodeSupplier back to make more space. OldLength: %d, NewLength: %d", length, newLength);
		length = newLength;
		
		System.arraycopy(chars,    currentBase, chars,    0, length);
		System.arraycopy(lineNums, currentBase, lineNums, 0, length);
		System.arraycopy(colNums,  currentBase, colNums,  0, length);
		
		currentBase = 0;
	}
	
	private void makeBufferBigger() {
		int newSize = (int)(chars.length*ARRAY_EXPAND_FACTOR);
		
		char[] oldChars = chars;
		int[] oldLines = lineNums;
		int[] oldCols = colNums;
		chars = new char[newSize];
		lineNums = new int[newSize];
		colNums = new int[newSize];
		System.arraycopy(oldChars, 0, chars, 0, length);
		System.arraycopy(oldLines, 0, lineNums, 0, length);
		System.arraycopy(oldCols , 0, colNums, 0, length);
	}
	
	@Override
	public void close() throws IOException {
		fileText.close();
	}

	@Override
	public boolean hasChar() {
		return currentChar < length;
	}

	@Override
	public char getCurrentChar() {
		if(currentChar >= length)
			log(ASSERTION_FAILED, "Asked for current char even though there is no current char!");
		return chars[currentChar];
	}

	@Override
	public int getCurrentLine() {
		if(currentChar >= length)
			log(ASSERTION_FAILED, "Asked for current char line even though there is no current char!");
		return lineNums[currentChar];
	}
	
	@Override
	public int getCurrentCol() {
		if(currentChar >= length)
			log(ASSERTION_FAILED, "Asked for current char col even though there is no current char!");
		return colNums[currentChar];
	}

	@Override
	public boolean advance() throws IOException {
		if(currentChar < length-1) { //We know the next char is already loaded
			currentChar++;
			return true;
		}
		
		if(!fileText.hasChar())
			log(FATAL_ERROR, "FileTextSupplier out of chars before advance() returned false");
		char c = fileText.getCurrentChar();
		if(c == '/') { //Let's check if we are looking at a comment
			int line = fileText.getCurrentLine();
			int col = fileText.getCurrentCol();
			if(fileText.advance()) { //The file might be over after the / (not really because of the '\n' but let's just go with it)
				if(!fileText.hasChar())
					log(FATAL_ERROR, "FileTextSupplier out of chars before advance() returned false");
				char nextC = fileText.getCurrentChar();
				if(nextC == '/') { //One line comment!
					//Advance and check until newline. Don't add anything
					log(SUPER_DEBUG, "Found one-line comment");
				} else if(nextC == '*') { //Multiple line comment
					
				} else {
					appendChar(c, line, col);
					appendChar(nextC, fileText.getCurrentLine(), fileText.getCurrentCol());
				}
			} else {
				appendChar(c, line, col);
			}
		} else if(c == '#') { //Check for macros and control statements and stuff
			
		} else {
			appendChar(c, fileText.getCurrentLine(), fileText.getCurrentCol()); //The usual
		}
		
		if(currentChar < length-1) {
			currentChar++; //We know that we have appended a char
		}
		
		return fileText.advance(); //We do one final advance if we read a char. Uuugly code
	}
}
