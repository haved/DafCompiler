package me.haved.daf.codegen;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.List;

import me.haved.daf.data.definition.Definition;

import static me.haved.daf.LogHelper.*;

public class MainCodegen {
	public static void generateCppAndHeader(List<Definition> definitions, File cppFile, File header) {
		try {
			PrintWriter cppOut = new PrintWriter(new FileWriter(cppFile));
			PrintWriter hOut   = new PrintWriter(new FileWriter(header));
			cppOut.printf("#include \"%s\"%n", header.getName());
			hOut.println("#pragma once");
			hOut.println("#include \"daf.h\"");
			writeCpp(definitions, cppOut, hOut);
			cppOut.flush();
			hOut.flush();
			cppOut.close();
			hOut.close();
		} catch(IOException e) {
			e.printStackTrace();
			log(FATAL_ERROR, "Failed to open file writer for output files");
		}
	}
	
	private static void writeCpp(List<Definition> definitions, PrintWriter cpp, PrintWriter h) {
		for(int i = 0; i < definitions.size(); i++) {
			definitions.get(i).codegenDefinitionCpp(cpp, h);
		}
	}
}
