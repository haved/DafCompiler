package me.haved.daf.syxer;

import me.haved.daf.lexer.tokens.TokenType;

import static me.haved.daf.LogHelper.*;

import me.haved.daf.data.type.PointerType;
import me.haved.daf.data.type.Primitive;
import me.haved.daf.data.type.Type;

public class TypeParser {
	public static Type parseType(TokenBufferer buffer, boolean mutable) {
		
		PointerType topPointer = null;
		PointerType bottomPointer = null;
		
		while(buffer.isCurrentTokenOfType(TokenType.ADDRESS)) {
			if(!buffer.advance()) {
				log(buffer.getLastToken(), ERROR, "Expected type or '%s' after last token in file", TokenType.MUT.getText());
				return null;
			}
			
			if(bottomPointer != null) {
				PointerType newBottom = new PointerType(null, mutable);
				bottomPointer.setTarget(newBottom);
				bottomPointer = newBottom;
			} else {
				topPointer = new PointerType(null, mutable);
				bottomPointer = topPointer;
			}
			
			mutable = false;
			if(buffer.isCurrentTokenOfType(TokenType.MUT)) {
				mutable = true;
				if(!buffer.advance()) {
					log(buffer.getLastToken(), ERROR, "Expected type after last token in file");
					return null;
				}
			}
		}
		
		TokenType tokenType = buffer.getCurrentToken().getType();
		
		Type result = null;
		for(Primitive primitive:Primitive.values()) {
			if(primitive.fitsTokenType(tokenType)) {
				result = primitive;
				break;
			}
		}
		
		if(result == null) {
			log(buffer.getCurrentToken(), ERROR, "Expected a type (i.e. primitive or pointer)");
			return null;
		}
		
		buffer.advance(); //Skip the primitive
		
		if(bottomPointer != null) {
			bottomPointer.setTarget(result);
			return topPointer;
		}
		return result;
	}
}
