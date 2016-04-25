package me.haved.daf.lexer;

import me.haved.daf.RegisteredFile;

import static me.haved.daf.LogHelper.*;

import java.util.Stack;

public class CodeSupplier {
	
	private RegisteredFile file;
	private Stack<MacroMap> macros;
	private int timeToPopMacroMap;
	private Stack<Integer> macroPoppingCounters;
	
	private FileTextSupplier fileText;
	
	private Stack<Character> charBuffer;
	private Stack<Integer> lineNumBuffer;
	private Stack<Integer> colNumBuffer;
	
	private char inputChar;
	private int inputLine;
	private int inputCol;
	
	private char current;
	private int line;
	private int col;
	
	public CodeSupplier(RegisteredFile file, MacroMap macros) {
		this.file = file;
		this.macros = new Stack<>();
		this.macros.push(macros);
		this.timeToPopMacroMap = 0;
		this.macroPoppingCounters = new Stack<>();
		fileText = new FileTextSupplier(file);
		charBuffer = new Stack<>();
		lineNumBuffer = new Stack<>();
		colNumBuffer = new Stack<>();
		
		if(!fileText.hasChar())
			log(ASSERTION_FAILED, "new FileTextSupplier was created, but has NO chars!");
		
		if(!trySetCurrentChar(fileText.getCurrentChar(), fileText.getCurrentCol(), fileText.getCurrentLine())) //This should never be false because '\n'
			log(ASSERTION_FAILED, "new FileTextSupplier gave a char that didn't fill the code supplier");
	}
	
	public boolean advance() {
		while(true) {
			if(!advanceInput())
				return false;
			if(trySetCurrentChar(inputChar, inputLine, inputCol)) //If we didn't add a letter, try again!
				return true;
		}
	}
	
	private boolean advanceInput() {
		if(timeToPopMacroMap>0) {
			timeToPopMacroMap--;
			if(timeToPopMacroMap == 0) {
				if(macros.size() > 1)
					System.out.printf("%n\t\tPoping MacroMap: %s%n%n", macros.pop().toString());
				else
					log(getFileName(), inputLine, inputCol, ASSERTION_FAILED, 
							"The MacroMap stack was supposed to be popped, but needs at least one MacroMap");
				if(!macroPoppingCounters.isEmpty()) //If it is empty, timeToPopMacroMap will stay at 0, making nothing happen anymore
					timeToPopMacroMap = macroPoppingCounters.pop();
				else
					log(getFileName(), inputLine, inputCol, ASSERTION_FAILED, 
							"%s Not replacing even though the MacroMap stack has got %d items",
							"The MacroMap time to pop stack was empty when timeToPopMacroMap ran out.",
							macros.size());
			}
		}
		if(!charBuffer.empty()) {
			inputChar = charBuffer.pop();
			inputLine = lineNumBuffer.pop();
			inputCol  = colNumBuffer.pop();
			return true;
		} else if(fileText.advance()) {
			inputChar = fileText.getCurrentChar();
			inputLine = fileText.getCurrentLine();
			inputCol  = fileText.getCurrentCol();
			return true;
		} else
			return false;
	}
	
	private boolean trySetCurrentChar(char c, int line, int col) {
		if(c=='/')
			return checkComments(c);
		else if(c=='#')
			return doMacroAndFlowChecks(c);
			
		return forceSetCurrentChar(c, line, col);
	}
	
	private boolean forceSetCurrentChar(char c, int line, int col) {
		this.current = c;
		this.line = line;
		this.col = col;
		return true;
	}
	
	private boolean checkComments(char firstChar) {
		int firstLine = inputLine;
		int firstCol  = fileText.getCurrentCol ();
		
		if(!advanceInput())
			return forceSetCurrentChar(firstChar, firstLine, firstCol);
			
		char next = inputChar;
		if(next == '/') { //One line comment!
			while(true) {
				if(!advanceInput())
					return false;
				if(inputChar=='\n')
					return forceSetCurrentChar(inputChar, inputLine, inputCol);
			}
		} else if(next=='*') {
			boolean lastStar=false;
			while(true) {
				if(!advanceInput())
					return false;
				char c = inputChar;
				if(lastStar&&c=='/')
					return false;
				lastStar = c=='*';
			}
		}
		
		pushBufferedInputChar(next, inputLine, inputCol);
		return forceSetCurrentChar(firstChar, firstLine, firstCol);
	}
	
	private boolean doMacroAndFlowChecks(char firstChar) {
		int firstLine = inputLine;
		int firstCol = inputCol;
		
		if(!advanceInput())
			return forceSetCurrentChar(firstChar, firstLine, firstCol);
		
		char nameStartChar = inputChar;
		if(!TextParserUtil.isStartOfIdentifier(nameStartChar)) { //So we need to set the pound symbol, and push the next letter
			pushBufferedInputChar(nameStartChar, inputLine, inputCol);
			return forceSetCurrentChar(firstChar, firstLine, firstCol);
		}
		
		StringBuilder identifierBuilder = new StringBuilder();
		identifierBuilder.append(nameStartChar);
		
		while(true) {
			if(!advanceInput()) { //We need to add all the shit back to the stack, and write the pound symbol to the current char
				for(int i = identifierBuilder.length()-1; i>=0; i--)
					pushBufferedInputChar(identifierBuilder.charAt(i), firstLine, firstCol+i+1);
				return forceSetCurrentChar(firstChar, firstLine, firstCol);
			}
			char c = inputChar;
			if(TextParserUtil.isIdentifierChar(c))
				identifierBuilder.append(c);
			else break;
		}
		
		String identifier = identifierBuilder.toString();
		
		int compilerFlagHandeling = handleCompilerFlag(firstLine, firstCol, identifier);
		if(compilerFlagHandeling == NEXT_CHAR_SET)//if this returns false, add the pound symbol and stuff back.
			return true;
		else if(compilerFlagHandeling == USE_STACK_OR_FILE_FOR_NEXT)
			return false;
		else { //WE push the identifier and force set the pound symbol
			for(int i = identifier.length()-1; i>=0; i--)
				pushBufferedInputChar(identifier.charAt(i), firstLine, firstCol+i+1);
			return forceSetCurrentChar(firstChar, firstLine, firstCol);
		}
	}
	
	/** Handles whatever happens when code is done. The current char at the time of calling is 
	 * 	the first char after the identifier
	 * 
	 * @param identifier the name of the flag
	 * @return the key saying what to do with the current char and stack
	 */
	private static final int PUSH_IDENTIFIER_AGAIN = 0;
	private static final int USE_STACK_OR_FILE_FOR_NEXT = 1;
	private static final int NEXT_CHAR_SET = 2;
	
	private int handleCompilerFlag(int line, int col, String identifier) {
		if(identifier.equalsIgnoreCase("macro")) {
			if(!handleMacroDefinition()) {
				log(getFileName(), line, col, ERROR, "Macro definition failed");
			}
			return USE_STACK_OR_FILE_FOR_NEXT; //Means the fileText is advanced after this.
		}
		Macro macro = null;
		for(int i = macros.size()-1; i>=0; i--) {
			macro = macros.elementAt(i).getMacro(identifier);
			if(macro!=null) {
				return evaluateMacro(line, col, macro);
			}
		}
		//Unrecognized token, mate. Put it back again.
		//The letter after the macro name is our responsibility to push back
		pushBufferedInputChar(inputChar, inputLine, inputCol);
		//This pushes the rest of the macro name in the wrong order, force setting the pound symbol
		return PUSH_IDENTIFIER_AGAIN;
	}
	
	/**When called, currentChar is the newline or $ of the macro definition
	 * 
	 * @return
	 */
	private boolean handleMacroDefinition() {
		if(!TextParserUtil.isNormalWhitespace(inputChar)) {
			log(getFileName(), inputLine, inputCol, ERROR, "'#macro' must be followed by a space or tab, not '%c'", inputChar);
			return false;
		}
		
		StringBuilder macroDef = new StringBuilder();
		
		boolean encloseMacro = false;
		
		while(true) {
			if(!advanceInput()) {
				log(getFileName(), inputLine, inputCol, ERROR, "File ended before macro definition!");
				return false;
			}
			char c = inputChar;
			macroDef.append(c);
			if(c==TextParserUtil.ENCLOSE_MACRO) {
				if(!encloseMacro) {
					encloseMacro = true;
				}
				else {
					break;
				}
			}
			else if(c==TextParserUtil.END_OF_LINE && !encloseMacro) {
				break;
			}
		}
		
		Macro macro = Macro.getMacroFromString(macroDef.toString());
		if(macro == null)
			return false;
		
		macros.firstElement().tryAddMacro(macro); //Add the macro to the bottom. Upper macro maps are only for macro parameters
		return true;
	}
	
	private int evaluateMacro(int line, int col, Macro macro) {
		//The input char is the one right after the macro name
		
		int firstWhitespace = inputCol;
		int whiteSpacesSkipped = 0;
		boolean done = false;
		
		while(inputChar==' ') {
			if(!advanceInput()) {
				done = true;
				break;
			}
			whiteSpacesSkipped++;
		}
		
		String[] parameters = null;
		
		if(!done && TextParserUtil.isStartOfMacroParameters(inputChar)) { //Parameter list
			if(!advanceInput()) { //Get to the char after '<'
				log(getFileName(), inputLine, inputCol, ERROR, "Macro parameter list didn't have an end!");
				return USE_STACK_OR_FILE_FOR_NEXT;
			}
			parameters = new String[macro.getMacroParameterCount()];
			int currentParam;
			for(currentParam = 0; currentParam < parameters.length; currentParam++) {
				char nextChar = currentParam == parameters.length-1 ? TextParserUtil.END_OF_MACRO_PARAMETER : macro.getSeparator(currentParam);
				StringBuilder parameter = new StringBuilder();
				int scopeDepth = 0; //Used for allowing < > inside < >
				while(true) {
					if(inputChar == nextChar && scopeDepth==0)
						break;
					if(inputChar == '<')
						scopeDepth++;
					else if(inputChar == '>')
						scopeDepth--;
					parameter.append(inputChar);
					if(!advanceInput()) {
						log(getFileName(), inputLine, inputCol, ERROR, "Macro parameter list didn't have an end!");
						return USE_STACK_OR_FILE_FOR_NEXT;
					}
				}
				parameters[currentParam] = parameter.toString().trim(); //We should in theory
			}
			if(currentParam != parameters.length) { //CurrentParam is at the index after the last parameter
				log(getFileName(), line, col, ERROR, "Not enough parameters were passed to the macro '%s' (%d/&d)", 
						macro.getMacroName(), currentParam, parameters.length);
				return USE_STACK_OR_FILE_FOR_NEXT;
			}
		} else { //Parameters were not found, and input char is not part of the macro
			pushBufferedInputChar(inputChar, inputLine, inputCol); //There is no parameter list, so push whatever char you found instead 
			for(int i = whiteSpacesSkipped-1; i >= 0; i--) //Push the white spaces we skipped back
				pushBufferedInputChar(' ', line, firstWhitespace+i);
			if(macro.getMacroParameterCount() != 0) {
				log(getFileName(), line, col, ERROR, "No parameters were passed to the macro '%s' requirering %d parameters", 
						macro.getMacroName(), macro.getMacroParameterCount());
				return USE_STACK_OR_FILE_FOR_NEXT;
			}
		}
		
		String value = macro.getMacroValue();
		if(value == null) {
			log(getFileName(), line, col, ERROR, "Tried evaluating a macro that doeasn't have a value: %s", macro.getMacroName());
			return USE_STACK_OR_FILE_FOR_NEXT;
		}
		for(int i = value.length()-1; i>=0; i--) { //Pushing the evaluated macro
			pushBufferedInputChar(value.charAt(i), line, col);
		}
		
		if(parameters != null) {
			MacroMap map = macro.makeMacroMapFromParameters(parameters);
			System.out.printf("%n\t\tPushing MacroMap: %s%n", map.toString());
			System.out.printf("\t\tPushed %d to the timeToPopMacroMap%n%n", value.length()+1);
			macros.push(map);
			macroPoppingCounters.push(timeToPopMacroMap);
			timeToPopMacroMap = value.length()+1;
		}
		
		return USE_STACK_OR_FILE_FOR_NEXT;
	}
	
	private void pushBufferedInputChar(char c, int line, int col) {
		charBuffer.push(c);
		lineNumBuffer.push(line);
		colNumBuffer.push(col);
	}
	
	public String getFileName() {
		return file.fileName;
	}
	
	public void close() {
		fileText.close();
	}
	
	public char getCurrentChar() {
		return current;
	}
	
	public int getCurrentLine() {
		return line;
	}
	
	public int getCurrentCol() {
		return col;
	}
}
