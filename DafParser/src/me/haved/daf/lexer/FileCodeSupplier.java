package me.haved.daf.lexer;

import java.io.IOException;

import static me.haved.daf.LogHelper.*;

public class FileCodeSupplier implements Supplier {
	
	private static int DEFAULT_CHAR_BUFFER_LENGTH = 30; // A macro will quickly destroy this
	private static float ARRAY_EXPAND_AMOUNT = 1.5f;
	
	private FileTextSupplier fileText;
	private char[] chars;
	private int[] lineNums;
	private int[] colNums;
	
	private int currentBase = 0;
	private int currentChar = 0;
	private int length = 0;
	
	public FileCodeSupplier(FileTextSupplier fileText) throws Exception {
		this.fileText = fileText;
		chars = new char[DEFAULT_CHAR_BUFFER_LENGTH]; //Start of with 20
		lineNums = new int[DEFAULT_CHAR_BUFFER_LENGTH];
		colNums = new int[DEFAULT_CHAR_BUFFER_LENGTH];
		
		if(!fileText.hasChar())
			if(!fileText.advance())
				log(FATAL_ERROR, "The %s given to a FileCodeSupplier refused to give a char or advance!", fileText.toString());
		if(!fileText.hasChar())
			log(FATAL_ERROR, "%s refused to give a char a second time!", fileText.toString());
		
		chars   [length] = fileText.getCurrentChar();
		lineNums[length] = fileText.getCurrentLine();
		colNums [length] = fileText.getCurrentCol ();
		length++;
	}
	
	@Override
	public void close() throws IOException {
		fileText.close();
	}

	@Override
	public boolean hasChar() {
		return currentChar<length;
	}

	@Override
	public char getCurrentChar() {
		if(currentChar<length)
			return chars[currentChar];
		log(ERROR, "FileCodeSupplier asked for chars when hasChar retruned false");
		return '\n';
	}

	@Override
	public int getCurrentLine() {
		if(currentChar<length)
			return lineNums[currentChar];
		log(ERROR, "FileCodeSupplier asked for line number when hasChar retruned false");
		return -1;
	}

	@Override
	public int getCurrentCol() {
		if(currentChar<length)
			return colNums[currentChar];
		log(ERROR, "FileCodeSupplier asked for line col when hasChar retruned false");
		return -1;
	}

	@Override
	public boolean advance() throws IOException {
		if(currentChar<length-1) { //We just go on by one!
			currentChar++; //current char may now be the last char
			return true;
		}
		
		//We need to go get some more chars!
		
		//As long as the char array isn't full, keep going
		
		if(length == chars.length) { //The array is full! Expand!
			char[] oldChars    = chars;
			int[]  oldLineNums = lineNums;
			int[]  oldColNums  = colNums;
			int newSize = (int)(oldChars.length*ARRAY_EXPAND_AMOUNT);
			chars = new char[newSize];
			lineNums = new int[newSize];
			colNums = new int[newSize];
			System.arraycopy(oldChars,    0, chars,    0, oldChars.length);
			System.arraycopy(oldLineNums, 0, lineNums, 0, oldLineNums.length);
			System.arraycopy(oldColNums,  0, colNums,  0, oldColNums.length);
			log(DEBUG, "Had to expand char buffer in FileCodeSupplier from %d to %d", oldChars.length, chars.length);
		}
		
		if(fileText.hasChar()) {
			chars   [length] = fileText.getCurrentChar();
			lineNums[length] = fileText.getCurrentLine();
			colNums [length] = fileText.getCurrentCol ();
			length++;
			if(currentChar < length-1) //In the case we didn't have a char at all, we got one now
				currentChar++;
			if(chars[currentChar]==' ')
				updateBaseToCurrent();
			return fileText.advance(); //Out of text?
		} else {
			log(ERROR, "The FileTextSupplier is out of chars before advance() returned false");
			return false;
		}
	}
	
	public void resetAtBase() {
		currentChar = currentBase;
	}

	public void updateBaseToCurrent() {
		
		length = length - currentChar;
		log(DEBUG, "Just update base of FileCodeSupplier! Length went from %d to %d!", length+currentChar, length);
		
		System.arraycopy(chars,    currentChar, chars,    0, length);
		System.arraycopy(lineNums, currentChar, lineNums, 0, length);
		System.arraycopy(colNums,  currentChar, colNums,  0, length);
		
		currentChar = 0;
		currentBase = 0;
	}
	
}
