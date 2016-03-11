package me.haved.daf;

import static me.haved.daf.LogHelper.*;

import java.util.Scanner;

import me.haved.daf.args.CommandOption;
import me.haved.daf.args.HelpOption;

public class MainDafParser {
	
	private static boolean developer = true;
	
	public static void main(String[] args) {
		log(SUPER_DEBUG, "Welcome to the daf parser!");
		if(args.length == 0)
			if(developer) {
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
		
		parseInput(args);
		
		LogHelper.printSummary(0);
	}
	
	public static void parseInput(String[] args) {
		
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
			
			if(inputFile == null)
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
		
		log(DEBUG, "Starting parsing with inputFile: '%s' and outputDir: '%s'", inputFile, outputDirectory);
	}
	
	private static void printHelpMessage(CommandOption[] options) {
		println("Usage: daf [options] <input file> <output directory>");
		println("Options:");
		
	}
}
