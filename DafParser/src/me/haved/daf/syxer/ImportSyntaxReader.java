package me.haved.daf.syxer;

import me.haved.daf.data.Definition;
import me.haved.daf.data.ImportDefinition;
import me.haved.daf.lexer.text.TextParserUtil;
import me.haved.daf.lexer.tokens.Token;
import me.haved.daf.lexer.tokens.TokenType;

import static me.haved.daf.LogHelper.*;

import java.util.ArrayList;

public class ImportSyntaxReader {

	public static Definition makeImportDefinition(TokenBufferer bufferer, boolean pub) {
		if(!bufferer.isCurrentTokenOfType(TokenType.IMPORT))
			return null;
		
		Token imp = bufferer.getCurrentToken();
		
		if(pub)
			log(imp, ERROR, "An import token can't be public!");
		
		ArrayList<String> parts = new ArrayList<String>();
		boolean lookingForSeparator = false;
		while(true) {
			if(!bufferer.advance()) {
				log(imp, ERROR, "File ended in the middle of an import statement!");
				break;
			}
			
			Token t = bufferer.getCurrentToken();
			if(t.getType() == TokenType.CLASS_ACCESS) {
				if(!lookingForSeparator) {
					log(t, ERROR, "An undexpected %s was found in the import statement!", t.getText());
				} else {
					lookingForSeparator = false;
				}
			}
			else if(t.getType() == TokenType.SEMICOLON)
				break;
			else if(lookingForSeparator) {
				log(t, ERROR, "Separator not found in import statement!"); //Lots of exclamation marks in this file's errors!
			}
			else {
				if(!TextParserUtil.areLetters(t.getText())) {
					log(t, ERROR, "Part of import statement path is not a legal text!");
					continue;
				}
				
				parts.add(t.getText());
				lookingForSeparator = true;
			}
		}

		bufferer.updateBase(1); //Will set the base to the next token after the semicolon
		
		return new ImportDefinition(parts.toArray(new String[parts.size()]));
	}
}
