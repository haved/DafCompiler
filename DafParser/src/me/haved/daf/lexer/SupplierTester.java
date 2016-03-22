package me.haved.daf.lexer;

import static me.haved.daf.LogHelper.*;

public class SupplierTester {
	public static void testSupplier(Supplier supplier) {
		try {
			println("Starting supplier test of '%s':", supplier.toString());
			while(true) {
				if(supplier.hasChar())
					println("'%c'   %d:%d", supplier.getCurrentChar(), supplier.getCurrentLine(), supplier.getCurrentCol());
				if(!supplier.advance())
					break;
			}
			if(supplier.hasChar())
				println("'%c'   %d:%d", supplier.getCurrentChar(), supplier.getCurrentLine(), supplier.getCurrentCol());
		} catch(Exception e) {
			log(e);
			println("Exception thrown when testing Supplier! '%s'", supplier.toString());
		}
	}
}
