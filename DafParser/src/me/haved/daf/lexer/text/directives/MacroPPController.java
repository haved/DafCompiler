package me.haved.daf.lexer.text.directives;

import me.haved.daf.lexer.text.Macro;
import me.haved.daf.lexer.text.PreProcessor;
import me.haved.daf.lexer.text.TextParserUtil;
import me.haved.daf.lexer.text.PreProcessor.InputHandler;

import static me.haved.daf.LogHelper.*;

public class MacroPPController implements PreProcessorController {
	
	private int line, col;
	private StringBuilder definition;
	private char endChar;
	
	public MacroPPController(int line, int col) {
		this.line = line;
		this.col = col;
		definition = new StringBuilder();
		endChar = '\n';
	}
	
	@Override
	public boolean allowAdvanceToReturn(PreProcessor pp, InputHandler inputHandler) {
		definition.append(pp.getCurrentChar());
		if(pp.getCurrentChar() == endChar) {
			Macro macro = Macro.makeMacroFromString(definition.toString());
			if(macro == null)
				log(inputHandler.getFile(), line, col, ERROR, "Aborting macro definition due to previous errors!");
			else
				inputHandler.addMacro(macro);
			pp.popBackControll();
		}
		else if(pp.getCurrentChar() == TextParserUtil.ENCLOSE_MACRO) {
			logAssert(endChar=='\n');
			endChar = TextParserUtil.ENCLOSE_MACRO;
		}
		return false;
	}

	@Override
	public boolean allowDirectiveToHappen(String directiveText, int line, int col, PreProcessor pp,
			InputHandler inputHandler) {
		
		definition.append('#');
		definition.append(directiveText);
		inputHandler.advanceInput();
		if(!TextParserUtil.isStartOfMacroParameters(inputHandler.getInputChar()))
			definition.append("<>");
		inputHandler.pushCurrentChar();
		
		return false;
	}

	@Override
	public String getName() {
		return "Macro definition controller";
	}
}
