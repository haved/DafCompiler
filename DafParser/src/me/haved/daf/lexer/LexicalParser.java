package me.haved.daf.lexer;

import java.util.ArrayList;
import me.haved.daf.RegisteredFile;

import static me.haved.daf.LogHelper.*;

public class LexicalParser {
	
	public static ArrayList<Token> tokenizeFile(RegisteredFile file, MacroMap map) {
		try {
			log(DEBUG, "tokenizeFile(%s)", file.toString());
			
			ArrayList<Token> tokens = new ArrayList<>();
			
			try {
				FileTextSupplier supplier = new FileTextSupplier(file);
				FileCodeSupplier codeSupplier = new FileCodeSupplier(supplier, map);
				SupplierTester.testSupplier(codeSupplier);
				
				codeSupplier.close(); //Just in case
			} catch(Exception e) {
				log(e);
				log(FATAL_ERROR, "Error occured during file reading of file '%s': %s", file.fileName, e.getClass().getName());
			}
			
			return tokens;
		}
		catch(Exception e) {
			e.printStackTrace();
			log(file, FATAL_ERROR, "Some sort of Exception was thrown in tokenizeFile()");
			return null;
		}
	}
}
