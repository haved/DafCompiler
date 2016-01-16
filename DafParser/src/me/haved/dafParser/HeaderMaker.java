package me.haved.dafParser;

import java.io.BufferedReader;
import java.io.PrintWriter;

import static me.haved.dafParser.LogHelper.*;

public class HeaderMaker extends FileMaker {

	protected HeaderMaker(DafParser parser) {
		super(parser);
	}

	@Override
	protected void readWriteFileStream(String inputFileName, BufferedReader reader, PrintWriter writer) throws Exception {
		String line = null;
		while((line=reader.readLine())!=null) {
			if(line.startsWith("#include")) {
				writer.println(line);
				continue;
			}
		}
		writer.flush();
	}

	@Override
	protected String getOutputFileName(String fileName) {
		if(!fileName.contains("."))
			log(WARNING, "The infile name does not contain a '.'");
		return String.format("%s.h", fileName.split("\\.")[0]); // We use \. because of regular expressions
	}
}
