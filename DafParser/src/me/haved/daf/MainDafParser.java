package me.haved.daf;

import static me.haved.daf.LogHelper.*;

import java.io.File;
import java.util.ArrayList;
import java.util.Scanner;

import me.haved.daf.args.CommandOption;
import me.haved.daf.args.HelpOption;
import me.haved.daf.lexer.LexicalParser;
import me.haved.daf.lexer.MacroMap;
import me.haved.daf.lexer.Token;

public class MainDafParser {
	
	private static boolean developer = true;	
	public  static void main(String[] args) {
		log(SUPER_DEBUG, "Welcome to the daf parser!");
		if(args.length == 0)
			if(developer) {
				LogHelper.startSummaryTime();
				log(SUPER_DEBUG, "Enter arguments:");
				String line;
				try (Scanner in = new Scanner(System.in)) {
					line = in.nextLine();
				}
				if(line.trim().isEmpty())
					log(FATAL_ERROR, "Give me somethig to work with, man!");
				args = line.split(" ");
			}
			else
				log(FATAL_ERROR, "No input files passed. -h for help");
		
		LogHelper.startSummaryTime();
		
		parseInput(args);
		
		LogHelper.printSummary(0);
	}

	private static void parseInput(String[] args) {
		CommandOption[] options = new CommandOption[] {null};
		options[0] = new HelpOption(()->printHelpMessage(options));
		
		String inputFile = null;
		String outputDirectory = null;
		
		argLoop:
		for(int i = 0; i < args.length;) {
			for(CommandOption option:options) {
				int r = option.parseOption(args, i);
				if(r==CommandOption.STOP_PARSING)
					return;
				if(r>0) {
					i+=r;
					continue argLoop;
				}
			}
			
			if(args[i].startsWith("-"))
				log(FATAL_ERROR, "Option '%s' not recognized!", args[i]);
			else if(inputFile == null)
				inputFile = args[i];
			else if(outputDirectory == null)
				outputDirectory = args[i];
			else
				log(FATAL_ERROR, "Too many arguments given. -h for help");
			
			i++;
		}
		
		if(inputFile == null) //Never got that far!
			log(FATAL_ERROR, "An input file needs to be specified. -h for help");
		else if(outputDirectory == null) //Never got that far!
			log(FATAL_ERROR, "An output directory needs to be specified. -h for help");
		
		parseFile(inputFile, outputDirectory);
	}

	private static void parseFile(String infileName, String outputDirName) {
		File inputFileObject = new File(infileName);
		File outputDir = new File(outputDirName);
		
		if(!inputFileObject.isFile())
			log(FATAL_ERROR, "The input file '%s' doesn't exist", infileName);
		if(!outputDir.isDirectory())
			log(FATAL_ERROR, "The output directory '%s' doesn't exist", outputDirName);
		
		log(DEBUG, "Parsing %s into %s", infileName, outputDirName);
	
		RegisteredFile inputFile = RegisteredFile.registerNewFile(inputFileObject, infileName);
		log(DEBUG, "Already registered: %s", RegisteredFile.getAlreadyRegisteredFile(inputFileObject).toString());
		
		MacroMap macros = new MacroMap();
		ArrayList<Token> tokens = LexicalParser.tokenizeFile(inputFile, macros);
		if(tokens == null)
			log(infileName, DEBUG, "tokenizeFile returned null!");
		terminateIfErrorsOccured();
		
		
		
		log(DEBUG, "Finished tokenizing, got %d tokens!", tokens.size());
	}
	
	private static void printHelpMessage(CommandOption[] options) {
		println("Usage: daf [options] <input file> <output directory>");
		println("Options:");
		
		String[] names = new String[options.length];
		String[] descs = new String[options.length];
		
		int longestName = 0;
		
		for(int i = 0; i < options.length; i++) {
			names[i] = options[i].getName();
			descs[i] = options[i].getDescription();
			longestName = Math.max(names[i].length(), longestName);
		}
		
		for(int i = 0; i < options.length; i++) {
			println(String.format("   %%%ds   %%s", -longestName), names[i], descs[i]);
		}
	}
}
