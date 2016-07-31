package me.haved.daf.syxer;

import me.haved.daf.data.type.FunctionType;
import me.haved.daf.data.type.FunctionType.FunctionParameter;
import me.haved.daf.data.type.PointerType;
import me.haved.daf.data.type.Primitive;
import me.haved.daf.data.type.Type;
import me.haved.daf.data.type.TypedefType;
import me.haved.daf.lexer.tokens.TokenType;

import static me.haved.daf.LogHelper.*;

import java.util.ArrayList;

public class TypeParser {
	public static Type parseType(TokenBufferer bufferer) {
		if(!bufferer.hasCurrentToken()) {
			log(bufferer.getLastToken(), ERROR, "Expected a type! Not EOF");
			return null;
		}
		
		PointerType.TypeOfPointer[] pointers = PointerType.parsePointers(bufferer);
		
		if(pointers != null)
			return new PointerType(parsePrimary(bufferer), pointers);
		return parsePrimary(bufferer);
	}
	
	public static Type parsePrimary(TokenBufferer bufferer) {
		if(!bufferer.hasCurrentToken()) {
			log(bufferer.getLastToken(), ERROR, "Expected a type! Not EOF");
			return null;
		}
		
		if(bufferer.isCurrentTokenOfType(TokenType.LEFT_PAREN)) //Hurrah! A function
			return parseFunctionSignature(bufferer);
		else if(bufferer.isCurrentTokenOfType(TokenType.IDENTIFER)) //A typedef!
			return parseTypedef(bufferer);
		
		
		Primitive prim = Primitive.getPrimitiveForType(bufferer.getCurrentToken().getType());
		if(prim != null) {
			bufferer.advance();
			return prim;
		}
		
		log(bufferer.getCurrentToken(), ERROR, "Expected a type! Not this shit");
		bufferer.advance(); //Eat shit
		return null;
	}
	
	private static Type parseTypedef(TokenBufferer bufferer) {
		logAssert(bufferer.isCurrentTokenOfType(TokenType.IDENTIFER));
		String ident = bufferer.getCurrentToken().getText();
		bufferer.advance(); //Eat identifier
		return new TypedefType(ident);
	}
	
	private static FunctionType parseFunctionSignature(TokenBufferer bufferer) {
		logAssert(bufferer.isCurrentTokenOfType(TokenType.LEFT_PAREN));
		bufferer.advance();
		ArrayList<FunctionParameter> params = null;
		if(!bufferer.isCurrentTokenOfType(TokenType.RIGHT_PAREN)) {
			params = new ArrayList<>();
			while(true) {
				if(!bufferer.hasCurrentToken()) {
					log(bufferer.getLastToken(), ERROR, "Expected function parameter after. Not EOF");
					return null;
				}
				FunctionParameter param = parseFunctionParameter(bufferer);
				params.add(param); //May be null
				if(bufferer.isCurrentTokenOfType(TokenType.RIGHT_PAREN))
					break;
				if(!bufferer.isCurrentTokenOfType(TokenType.COMMA)) {
					log(bufferer.getLastOrCurrent(), ERROR, "Expected ')' or ',' after function signature parameter");
					return null;
				}
				bufferer.advance(); //Eat the comma
			}
		}
		logAssert(bufferer.isCurrentTokenOfType(TokenType.RIGHT_PAREN));
		bufferer.advance(); //Eat the ')'
		
		Type returnType = null;
		if(bufferer.isCurrentTokenOfType(TokenType.COLON)) {
			bufferer.advance(); //Eat ':'
			returnType = parseType(bufferer);
		}
		
		//Get return type
		return new FunctionType(params.toArray(new FunctionParameter[params.size()]), returnType);
	}
	
	//[[&|&mut|&move| ] [id| ] : | ] Type
	private static FunctionParameter parseFunctionParameter(TokenBufferer bufferer) {
		int refType = FunctionParameter.NOT_REFRENCE;
		if(bufferer.isCurrentTokenOfType(TokenType.getAddressType())) {
			if(!bufferer.advance()) {
				log(bufferer.getLastToken(), ERROR, "Expected something after");
				return null;
			}
			TokenType type = bufferer.getCurrentToken().getType();
			if(type == TokenType.MUT) {
				refType = FunctionParameter.MUTABLE_REF;
				bufferer.advance(); //Eat 'mut'
			}
			else if(type == TokenType.MOVE) {
				refType = FunctionParameter.MOVE_REF;
				bufferer.advance(); //Eat 'move'
			}
			
			if(!bufferer.isCurrentTokenOfType(TokenType.IDENTIFER) && bufferer.isCurrentTokenOfType(TokenType.COLON)) {
				log(bufferer.getLastOrCurrent(), ERROR, "Expected an identifier or '%s' in the function signature", TokenType.COLON);
				return null;
			}
		}
		
		String name = null;
		if(bufferer.isCurrentTokenOfType(TokenType.IDENTIFER)) {
			name = bufferer.getCurrentToken().getText();
			bufferer.advance(); //Eat identifier
		}
		
		if(bufferer.isCurrentTokenOfType(TokenType.COLON)) {
			bufferer.advance(); //Eat ':'
		}
		
		//Now expecting a type
		Type type = TypeParser.parseType(bufferer);
		if(type == null)
			return null;
		
		return new FunctionParameter(refType, name, type);
	}
}
