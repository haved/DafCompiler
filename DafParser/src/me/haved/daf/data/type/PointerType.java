package me.haved.daf.data.type;

import java.util.ArrayList;

import me.haved.daf.LogHelper;
import me.haved.daf.lexer.tokens.TokenType;
import me.haved.daf.syxer.TokenBufferer;

public class PointerType implements Type {
	public static TokenType POINTER = TokenType.getAddressType();
	
	public static enum TypeOfPointer {
		IMMUT(null, "& "), MUT(TokenType.MUT, "&mut "), SHARED(TokenType.SHARED, "&shared "), UNIQUE(TokenType.UNIQUE, "&unique");
		
		private TokenType type;
		private String signature;
		TypeOfPointer(TokenType token, String signature) {
			this.type = token;
			this.signature = signature;
		}
	}
	
	private Type target;
	private TypeOfPointer[] pointers;
	
	public PointerType(Type target, TypeOfPointer[] pointers) {
		LogHelper.logAssert(pointers != null && target != null);
		this.target = target;
		this.pointers = pointers;
	}
	
	@Override
	public String getSignature() {
		StringBuilder builder = new StringBuilder();
		for(TypeOfPointer top:pointers) {
			builder.append(top.signature);
		}
		return builder.append(target.getSignature()).toString();
	}
	
	@Override
	public String toString() {
		return getSignature();
	}

	@Override
	public boolean isInteger() {
		return true;
	}
	
	@Override
	public String codegenCpp() {
		StringBuilder builder = new StringBuilder();
		doCodegen(builder, 0);
		return builder.toString();
	}
	
	private void doCodegen(StringBuilder builder, int index) {
		if(index >= pointers.length)
			builder.append(target.codegenCpp());
		else {
			switch(pointers[index]) {
			case MUT:
				doCodegen(builder, index+1); builder.append(" *"); break;
			case IMMUT:
				builder.append("( const ("); doCodegen(builder, index+1); builder.append(" *))"); break;
			default:
				break;
			}
		}
	}

	public static TypeOfPointer[] parsePointers(TokenBufferer bufferer) {
		if(!bufferer.isCurrentTokenOfType(POINTER))
			return null;
		
		ArrayList<TypeOfPointer> pointers = new ArrayList<>();
		
		outerLoop:
		do {
			bufferer.advance();
			for(TypeOfPointer p:TypeOfPointer.values()) {
				if(bufferer.isCurrentTokenOfType(p.type)) {
					pointers.add(p);
					bufferer.advance();
					continue outerLoop;
				}
			}
			pointers.add(TypeOfPointer.IMMUT);
		} while(bufferer.isCurrentTokenOfType(POINTER));
		
		return pointers.toArray(new TypeOfPointer[pointers.size()]);
	}
}
