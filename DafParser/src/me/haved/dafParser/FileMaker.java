package me.haved.dafParser;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;

import static me.haved.dafParser.LogHelper.*;

public abstract class FileMaker {
	protected DafParser parser;
	
	protected FileMaker(DafParser parser) {
		this.parser = parser;
	}
	
	public void readFile(String inputFileName, File inputFile, File outputDir) {
		logAssert(inputFile.isFile(), "The input file passed to the file maker didn't exist!");
		logAssert(outputDir.isDirectory(), "The output directory passed to the file maker didn't exist!");
		
		log(INFO, "FileMaker running in the folder '%s' taking the input file '%s'", outputDir.getAbsolutePath(), inputFileName);
		String outputFileName = getOutputFileName(parser.subfolderOutput ? inputFileName : inputFile.getName());
		log(MESSAGE, "Making the file '%s' in the output dir", outputFileName);
		File outputFile = new File(outputDir.getAbsolutePath()+"/"+outputFileName);
		outputFile.getParentFile().mkdirs(); //Make directories. If input is folder/file.daf, it will create outputFolder/folder/
		
		try {
			if(outputFile.exists())
				outputFile.delete();
			try (BufferedReader reader = new BufferedReader(new FileReader(inputFile))) {
				try (PrintWriter writer = new PrintWriter(new FileWriter(outputFile, false))) {
					readWriteFileStream(inputFileName, reader, writer);
					reader.close();
					writer.flush(); //Just to be sure
					writer.close();
				}catch(IOException ioe) {
					ioe.printStackTrace(LogHelper.out);
					log(FATAL_ERROR, "An IOException was thrown when making a PrintWriter for '%s'", outputFile.getAbsolutePath());
				}
			}catch(IOException ioe) {
				ioe.printStackTrace(LogHelper.out);
				log(FATAL_ERROR, "An IOException was thrown when making a BufferedReader for '%s'", inputFile.getAbsolutePath());
			}
		} catch(Exception e) {
			e.printStackTrace(LogHelper.out);
			log(FATAL_ERROR, "An exception was thrown in the file maker!");
		}
	}
	
	protected abstract void readWriteFileStream(String inputFileName, BufferedReader reader, PrintWriter writer) throws Exception;
	
	protected abstract String getOutputFileName(String fileName);
}
