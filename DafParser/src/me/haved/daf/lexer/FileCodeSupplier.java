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
		this(fileText, macros, true);
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
	public String getSourceName() {
		return fileText.getSourceName();
	}

	@Override
	public boolean advance() throws IOException {
		currentChar++; //After advance() hasChar is either true, or advance returned false
		
		if(currentChar < length) //We know the char is already loaded
			return true; //Successful advance!
		
		return appendNextChar(true); //We need to append a new char
	}
	
	private boolean appendNextChar(boolean advance) throws IOException {
		if(advance && !fileText.advance()) //We NEED a char! Why did you fail us??
			return false;
		
		if(!fileText.hasChar())
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
			
			return appendNextChar(true); //In theory a bit ugly and stuff because of the recursion, but two consecutive macro definitions will
										//both be parsed, and a letter will be added after it. Nice code??
		}
		
		Macro macro = macros.getMacro(identifier);
		if(macro != null) { //When here, currentChar is the char after the macro Name
			
			if(!evaluateMacroFromHere(macro))
				return false;
			
			if(fileText.hasChar())
				log(SUPER_DEBUG, "Just finished evaluateMacroFromHere()!", fileText.getCurrentChar());
			else
				log(SUPER_DEBUG, "Evaluate macro advanced past the fileText supplier!");
			//After evaluateMacro, we know that current char is to be added
			return appendNextChar(false);
			
		} else {
			if(!allowUnresolvedMacros) {
				log(fileText.getFile().fileName, poundLine, poundCol, ERROR, "Unresolved macro found: '#%s'", identifier);
				return false;
			}
			//Time to add all the stuff back!
			
			assureExtraSpace(identifier.length()+2); //Both the pound symbol and the char after the identifier ended (current)
			
			appendChar(firstChar, poundLine, poundCol);
			for(int i = 0; i < identifier.length(); i++)
				appendChar(identifier.charAt(i), poundLine, poundCol+1+i); //We know that only one-wide chars are in identifier
			
			appendChar(fileText.getCurrentChar(), fileText.getCurrentLine(), fileText.getCurrentCol());
			return true; //All good
		}
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

	/**
	 * fileText current char must be the char after the macro name on call
	 * After the call, the fileText current char is the first char that isn't a space after the name (spaces have been appended), 
	 * 								OR the first char after the parameter list that isn't '>' (might not be a char, so check!)
	 * 
	 * @param macro the macro instance that is to be evaluated and then added onto the buffer
	 * @return true if everything is well. Note: Stuff isn't necessarily added to the buffer. Be sure! 
	 * @throws IOException
	 */
	private boolean evaluateMacroFromHere(Macro macro) throws IOException {
		int whitespacesSkipped = 0;
		boolean fileIsOver = false;
		
		int whitespacesStartCol = fileText.getCurrentCol();
		
		while(true) { //Skip whitespace
			if(fileText.getCurrentChar() == ' ') {
				whitespacesSkipped++;
				if(!fileText.advance()) {
					fileIsOver = true;
					break;
				}	
			}
			else
				break;
		}
		
		String[] params = null;
		if(macro.hasMacroParameters()) {
			params = new String[macro.getMacroParameters().length];
			if(params.length == 0)
				log(ASSERTION_FAILED, "The macro parameter list existed, but had a length of 0. Should be null!");
		}
		int foundParameters = -1; //Means to parameter list
		if(!fileIsOver) { //Then we look for parameters
			char afterName = fileText.getCurrentChar();
			if(TextParserUtil.isStartOfMacroParameters(afterName)) { //Parameter list!
				foundParameters = 0;
				log(SUPER_DEBUG, "We have a parameter list!");
				while(true) {
					if(!fileText.advance()) {
						log(ERROR, "File ended in middle of macro parameter list for macro: %s", macro.getMacroName());
						return false;
					}
					if(TextParserUtil.isEndOfMacroParameters(fileText.getCurrentChar())) {
						break; //We are done with the macro parameters!
					}
				}
				fileText.advance(); //We don't care what it returns. we just need to be sure we're past the '>'
				foundParameters = params.length; //For the sake of debugging, we pretend to have parameters
			}
		} else if(params != null) {
			log(ERROR, "Macro evaluation text was over before the needed %d parameters were found!", params.length);
			return false;
		}
		
		if(params != null && foundParameters < params.length) { //Not enough parameters 
			if(foundParameters == -1)
				log(ERROR, "Expected %d macro parameters, but there was no parameter list!", params.length);
			else
				log(ERROR, "Expected %d macro parameters, but only %d were given!", params.length, foundParameters);
			return false;
		}
		else if(foundParameters > (params == null ? 0:params.length)) {
			log(ERROR, "Too many macro parameters were given to '%s'! Expected %d but got %d", 
					macro.getMacroName(), params==null?0:params.length, foundParameters);
			return false;
		}
		
		MacroMap map = null;
		if(params != null) {
			map = macro.makeMacroMapFromParameters(params);
			if(map == null)
				log(fileText.getFile().fileName, fileText.getCurrentLine(), fileText.getCurrentCol(), ERROR, "Wrong amount of macro parameters passed");
		}
			
		String macroEvaluation = macro.getMacroValue();
		
		if(macroEvaluation != null && macroEvaluation.trim().length()!=0) {
		
			if(map != null) { //We need to pass it through a FileCodeSupplier
				StringTextSupplier supplier = 
						new StringTextSupplier(macroEvaluation, fileText.getFile().fileName, fileText.getCurrentLine(), fileText.getCurrentCol());
			
				
			}
			
			assureExtraSpace(macroEvaluation.length());
			for(int i = 0; i < macroEvaluation.length(); i++) {
				appendChar(macroEvaluation.charAt(i), fileText.getCurrentLine(), fileText.getCurrentLine());
			}
		}
		
		if(foundParameters == -1) //We didn't find a macro list, but may still have skipped spaces. Add them back after the macro
			for(int i = 0; i < whitespacesSkipped; i++) {
				appendChar(' ', fileText.getCurrentLine(), whitespacesStartCol+i);
			}
		
		return true;
	}
}
