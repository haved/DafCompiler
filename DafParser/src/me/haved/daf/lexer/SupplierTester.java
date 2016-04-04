package me.haved.daf.lexer;

import static me.haved.daf.LogHelper.*;

public class SupplierTester {
	public static void testSupplier(Supplier supplier) {
		try {
			println("Starting supplier test of '%s':", supplier.toString());
			while(true) {
				if(!supplier.hasChar())
					log(FATAL_ERROR, "Tested Supplier '%s'.hasChar() retruned false before advance() returned false");
				//println("'%c'   %d:%d", supplier.getCurrentChar(), supplier.getCurrentLine(), supplier.getCurrentCol());
				System.out.print(supplier.getCurrentChar());
				if(!supplier.advance())
					break;
			}
			if(supplier.hasChar())
				log(FATAL_ERROR, "Tested Supplier had more chars after advance() returned false: '%c'", supplier.getCurrentChar());
		} catch(Exception e) {
			log(e);
			println("Exception thrown when testing Supplier! '%s'", supplier.toString());
		}
	}
}
