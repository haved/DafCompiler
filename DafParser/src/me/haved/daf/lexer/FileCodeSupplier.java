package me.haved.daf.lexer;

import java.io.IOException;

import static me.haved.daf.LogHelper.*;

public class FileCodeSupplier implements Supplier {
	
	private static int DEFAULT_CHAR_BUFFER_LENGTH = 30; // A macro will quickly destroy this
	private static float ARRAY_EXTRA_EXPAND_FACTOR = 0.5f;
	
	private FileTextSupplier fileText;
	private MacroMap macros;
	private char[] chars;
	private int[] lineNums;
	private int[] colNums;
	
	private int currentBase = 0;
	private int currentChar = 0;
	private int length = 0; //The length of the used space of the buffer
	
	public FileCodeSupplier(FileTextSupplier fileText, MacroMap macros) throws Exception {
		this.fileText = fileText;
		this.macros = macros;
		this.chars = new char[DEFAULT_CHAR_BUFFER_LENGTH];
		this.lineNums = new int[DEFAULT_CHAR_BUFFER_LENGTH];
		this.colNums = new int[DEFAULT_CHAR_BUFFER_LENGTH];
	
		appendChar(fileText.getCurrentChar(), fileText.getCurrentLine(), fileText.getCurrentCol());
	}

	private boolean appendChar(char c, int line, int col) {
		assureExtraSpace(1); //We need at least one more char
		if(length >= chars.length) //Our size making measures failed
			return false;
		chars   [length] = c;
		lineNums[length] = line;
		colNums [length] = col;
		length++;
		return true;
	}
	
	/** Makes sure the buffer is big enough that length+space <= chars.length
	 * 
	 * @param space the size that must be available after the current used space
	 */
	private void assureExtraSpace(int space) {
		if(length+space<=chars.length) { //We can fit it already. No problem
			return;
		}
		
		int moreSpace = (length+space)-chars.length; //How much more space we need
		
		if(currentBase>=moreSpace) { //We can just move everything back by currentBase to make enough space!
			length -=      currentBase;
			currentChar -= currentBase;
			log(SUPER_DEBUG, "Moving chars in FileCodeSupplier back to make more space. OldLength: %d, NewLength: %d", length+currentBase, length);
			
			System.arraycopy(chars,    currentBase, chars,    0, length); //Move from current base to 0
			System.arraycopy(lineNums, currentBase, lineNums, 0, length);
			System.arraycopy(colNums,  currentBase, colNums,  0, length);
			
			currentBase = 0;
		} else { //We must make a bigger array!
			int newSize = length + space - currentBase; //How much we need
			if(space < 4)
				newSize += length * ARRAY_EXTRA_EXPAND_FACTOR; //Add a bit more
			
			char[] oldChars = chars;
			int[] oldLines = lineNums;
			int[] oldCols = colNums;
			chars = new char[newSize];
			lineNums = new int[newSize];
			colNums = new int[newSize];
			
			length -= currentBase;
			currentChar -= currentBase;
			System.arraycopy(oldChars, currentBase, chars, 0, length);
			System.arraycopy(oldLines, currentBase, lineNums, 0, length);
			System.arraycopy(oldCols , currentBase, colNums, 0, length);	
			
			currentBase = 0;
		}
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
		if(!hasChar())
			log(ASSERTION_FAILED, "Asked for current char even though there is no current char!");
		return chars[currentChar];
	}

	@Override
	public int getCurrentLine() {
		if(!hasChar())
			log(ASSERTION_FAILED, "Asked for current char line even though there is no current char!");
		return lineNums[currentChar];
	}
	
	@Override
	public int getCurrentCol() {
		if(!hasChar())
			log(ASSERTION_FAILED, "Asked for current char col even though there is no current char!");
		return colNums[currentChar];
	}

	@Override
	public boolean advance() throws IOException {
		currentChar++; //After advance() hasChar is either true, or advance returned false
		
		if(currentChar < length) //We know the char is already loaded
			return true; //Successful advance!
		
		if(!fileText.advance()) //We NEED a char! Why did you fail us??
			return false;
		
		char firstChar = fileText.getCurrentChar();
		if(firstChar == '/') { //A comment perhaps? 
			int firstLine = fileText.getCurrentLine();
			int firstCol = fileText.getCurrentCol();
			
			if(!fileText.advance()) {
				appendChar(firstChar, firstLine, firstCol);
				return true; //Next advance call will return false
			}
			
			char nextC = fileText.getCurrentChar();
			if(nextC == '/') { //One line comment, baby!
				while(true) {
					if(!fileText.advance()) //The file is done in a comment (Never happens because of forced newline end
						return false;
					if(fileText.getCurrentChar() == '\n')
						break;
				}
				if(!fileText.advance()) //There was nothing after the comment. The file is over
					return false;
				appendChar(fileText.getCurrentChar(), fileText.getCurrentLine(), fileText.getCurrentCol());
				return true;
			} else if(nextC == '*') { //Multiple line comment!
				boolean starFound = false;
				while(true) {
					if(!fileText.advance()) //The file is done in a comment (Never happens because of forced newline end
						return false;
					char c = fileText.getCurrentChar();
					if(c == '*')
						starFound = true;
					else if(c == '/' && starFound)
						break; //We are done
					else
						starFound = false;
				}
				if(!fileText.advance()) //There was nothing after the comment. The file is over
					return false;
				appendChar(fileText.getCurrentChar(), fileText.getCurrentLine(), fileText.getCurrentCol());
				return true;
			} else { //If next char wasn't interesting, we add the first char as well as the next char
				appendChar(firstChar, firstLine, firstCol);
				appendChar(nextC, fileText.getCurrentLine(), fileText.getCurrentCol());
				return true;
			}
		} else {
			appendChar(firstChar, fileText.getCurrentLine(), fileText.getCurrentCol());
			
			return true;
		}
	}
}
