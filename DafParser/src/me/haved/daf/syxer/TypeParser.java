package me.haved.daf.syxer;

import me.haved.daf.data.type.PointerType;
import me.haved.daf.data.type.Primitive;
import me.haved.daf.data.type.Type;

import static me.haved.daf.LogHelper.*;

public class TypeParser {
	public static Type parseType(TokenBufferer bufferer) {
		if(!bufferer.hasCurrentToken()) {
			log(bufferer.getLastToken(), ERROR, "Expected a type! Not EOF");
			return null;
		}
		
		PointerType.TypeOfPointer[] pointer = PointerType.parsePointers(bufferer);
		
		if(!bufferer.hasCurrentToken()) {
			log(bufferer.getLastToken(), ERROR, "Expected a type! Not EOF");
			return null;
		}
		
		Primitive prim = Primitive.getPrimitiveForType(bufferer.getCurrentToken().getType());
		if(prim != null) {
			bufferer.advance();
			return pointer == null ? prim : new PointerType(prim, pointer);
		}
		
		log(bufferer.getCurrentToken(), ERROR, "Expected a type! Not this shit");
		return null;
	}
}
