package me.haved.daf.lexer;

import static me.haved.daf.LogHelper.*;

public class SupplierTester {
	public static void testSupplier(Supplier supplier) {
		try {
			println("Starting supplier test of '%s':", supplier.toString());
			println(Supplier.supplierToString(supplier));
		} catch(Exception e) {
			log(e);
			println("Exception thrown when testing Supplier! '%s'", supplier.toString());
		}
	}
}
