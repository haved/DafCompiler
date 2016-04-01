package me.haved.daf.lexer;

import java.io.IOException;

import static me.haved.daf.LogHelper.*;

public class FileCodeSupplier implements Supplier {
	
	private static final String MACRO_KEYWORD = "macro";
	
	private static int DEFAULT_CHAR_BUFFER_LENGTH = 30; // A macro will quickly destroy this
	private static float ARRAY_EXTRA_EXPAND_FACTOR = 0.5f;
	
	private FileTextSupplier fileText;
	private MacroMap macros;
	private boolean allowUnresolvedMacros;
	
	private char[] chars;
	private int[] lineNums;
	private int[] colNums;
	
	private int currentBase = 0;
	private int currentChar = 0;
	private int length = 0; //The length of the used space of the buffer
	
	public FileCodeSupplier(FileTextSupplier fileText, MacroMap macros) throws Exception {
		this(fileText, macros, false);
	}
	
	public FileCodeSupplier(FileTextSupplier fileText, MacroMap macros, boolean allowUnresolvedMacros) throws Exception {
		this.fileText = fileText;
		this.macros = macros;
		this.allowUnresolvedMacros = allowUnresolvedMacros;
		
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
		
		return appendNextChar(); //We need to append a new char
	}
	
	private boolean appendNextChar() throws IOException {
		if(!fileText.advance()) //We NEED a char! Why did you fail us??
			return false;
		
		char firstChar = fileText.getCurrentChar();
		if(firstChar == '/') //A comment perhaps? 
			return doCommentChecks(firstChar);
		if(firstChar == '#')
			return doMacroAndFlowChecks(firstChar);
		
		appendChar(firstChar, fileText.getCurrentLine(), fileText.getCurrentCol());
		return true;
	}
	
	/**
	 * 
	 * @param firstChar the char that made you branch over here
	 * @return true if there is a new char added
	 * @throws IOException
	 */
	private boolean doCommentChecks(char firstChar) throws IOException {
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
	}
	
	/**
	 * 
	 * @param firstChar the char that made you branch here
	 * @return true if a new char was added to the text
	 * @throws IOException
	 */
	private boolean doMacroAndFlowChecks(char firstChar) throws IOException {
		int poundLine = fileText.getCurrentLine();
		int poundCol = fileText.getCurrentCol();
		
		StringBuilder identifierB = new StringBuilder();
		
		while(true) {
			if(!fileText.advance()) {
				log(fileText.getFile().fileName, poundLine, poundCol, FATAL_ERROR, "A properly finished keyword or macro identifier wasn't found after a pound symbol");
				return false; //Just to be sure
			}
			
			char c = fileText.getCurrentChar();
			if(identifierB.length()==0?TextParserUtil.isStartOfIdentifier(c):TextParserUtil.isIdentifierChar(c)) {
				identifierB.append(c);
			} else
				break;
		}
		//We are now done with the identifier, and fileText.currentChar is not a part of it!
		
		String identifier = identifierB.toString();
		if(identifier.equals(MACRO_KEYWORD)) { //Adding a new macro, guys!
			if(!TextParserUtil.isNormalWhitespace(fileText.getCurrentChar())) {
				log(fileText.getFile().fileName, poundLine, poundCol + identifier.length(), ERROR, "#%s must be followed by a whitespace before the definition", MACRO_KEYWORD);
				return false;
			}
			if(!resolveMacroDefinition()) { //This ends with fileText.currentChar at the last char of macro definition
				log(fileText.getFile().fileName, poundLine, poundCol, ERROR, "Macro definition failed due to previous error(s)");
				return false; //No error recoverability what so ever
			}
			
			return appendNextChar(); //In theory a bit ugly and stuff because of the recursion, but two consecutive macro definitions will
										//both be parsed, and a letter will be added after it. Nice code??
		}
		
		Macro macro = macros.getMacro(identifier);
		if(macro != null) {
			
		} else {
			if(!allowUnresolvedMacros) {
				log(fileText.getFile().fileName, poundLine, poundCol, ERROR, "Unresolved macro found: '#%s'", identifier);
				return false;
			}
			//Time to add all the stuff back!
		}
		
		return false;
	}
	
	/** Starts at the next char in the fileTextParser and keeps going until the macro definition is over
	 * NB: Does not append a char, and leaves the fileText currentChar at either the newline or $
	 * 
	 * @return true if it worked
	 */
	private boolean resolveMacroDefinition() throws IOException {
		StringBuilder macroLine = new StringBuilder(); //All parse errors are handled in the macroFromString
		
		boolean foundDefStart = false;
		
		while(true) {
			if(!fileText.advance()) {
				log(ERROR, "The file ended before the macro definition was over! Did you forget a closing '%c' ?", TextParserUtil.ENCLOSE_MACRO);
				return false;
			}
			
			char c = fileText.getCurrentChar();
			
			macroLine.append(c);
			
			if(c == TextParserUtil.ENCLOSE_MACRO) {
				if(foundDefStart) { //We are done!
					break;
				}
				foundDefStart = true;
			}
			if(c == TextParserUtil.END_OF_LINE && !foundDefStart) {
				break;
			}
		}
		
		Macro macro = Macro.getMacroFromString(macroLine.toString());
		if(macro == null) {
			return false; //Error location is handled outside here
		}
		
		macros.tryAddMacro(macro);
		
		return true;
	}
}
