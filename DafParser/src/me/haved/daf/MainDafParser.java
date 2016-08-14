package me.haved.daf;

import static me.haved.daf.LogHelper.*;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.List;
import java.util.Scanner;

import me.haved.daf.args.CommandOption;
import me.haved.daf.args.HelpOption;
import me.haved.daf.args.MacroOption;
import me.haved.daf.args.PreprocOnlyOption;
import me.haved.daf.codegen.MainCodegen;
import me.haved.daf.data.definition.Definition;
import me.haved.daf.lexer.LiveTokenizer;
import me.haved.daf.lexer.text.PreProcessor;
import me.haved.daf.lexer.text.TextParserUtil;
import me.haved.daf.lexer.text.MacroMap;
import me.haved.daf.syxer.SyntaxicParser;

public class MainDafParser {
	
	private static final boolean DEVELOPER = true;
	private static final String DEFAULT_ARGS = "../TestCode/TestFile.daf .";
	public  static void main(String[] args) {
		if(args.length == 0)
			if(DEVELOPER) {
				
				String line = null;
				
				if(DEFAULT_ARGS==null) {
					log(SUPER_DEBUG, "Enter arguments:");
					try (Scanner in = new Scanner(System.in)) {
						line = in.nextLine();
					}
				}
				
				if(line == null || line.isEmpty())
					line = DEFAULT_ARGS;
				
				ArrayList<String> myArgs = new ArrayList<>();
				int start = 0;
				boolean inQuotes = false;
				for(int i = 0; i < line.length()+1; i++) {
					char c = i < line.length() ? line.charAt(i) : ' ';
					if(TextParserUtil.isNormalWhitespace(c) & !inQuotes) {
						if(start<i)
							myArgs.add(line.substring(start, i));
						start = i+1;
					} else if(TextParserUtil.isDoubleQuoteChar(c)) {
						if(inQuotes)
							myArgs.add(line.substring(start, i)); //Not including the quotes
						inQuotes = !inQuotes;
						start = i+1;
					}
				}
				
				args = new String[myArgs.size()];
				myArgs.toArray(args);
			}
			else
				log(FATAL_ERROR, "No input files passed. -h for help");
		
		LogHelper.startSummaryTime();
		
		parseInput(args);
		
		LogHelper.printSummary(0);
	}

	private static boolean onlyDoPreProc;
	
	private static void parseInput(String[] args) {
		MacroMap macros = new MacroMap();
		CommandOption[] options = new CommandOption[] {
				null, 
				new MacroOption((text)->macros.tryAddMacro(text)), 
				new PreprocOnlyOption(MainDafParser::doPreprocOnly)};
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
				log(FATAL_ERROR, "Too many unresolved arguments given. -h for help");
			
			i++;
		}
		
		if(inputFile == null) //Never got that far!
			log(FATAL_ERROR, "An input file needs to be specified. -h for help");
		else if(outputDirectory == null) { //Never got that far!
			outputDirectory = ".";
			log(INFO, "No output directory passed. Using the current directory");
			//log(FATAL_ERROR, "An output directory needs to be specified. -h for help");
		}
		
		if(onlyDoPreProc)
			doOnlyPreproc(inputFile, outputDirectory, macros);
		else
			parseFile(inputFile, outputDirectory, macros);
	}

	private static void parseFile(String infileName, String outputDirName, MacroMap macros) {
		File inputFileObject = new File(infileName);
		if(!inputFileObject.isFile())
			log(FATAL_ERROR, "The input file '%s' doesn't exist", infileName);
		
		File outputDir = new File(outputDirName);
		String pathBase = getPathAndFirstName(inputFileObject, outputDir);
		File cppOutput = new File(pathBase+".cpp");
		File hOutput   = new File(pathBase+".h");
		if(outputDir.isFile())
			log(FATAL_ERROR, "The supplied output directory is a file!");
		if(!outputDir.isDirectory())
			log(FATAL_ERROR, "The supplied output directory doesn't exist!");
		if(cppOutput.isFile() && !cppOutput.canWrite())
			log(FATAL_ERROR, "Can't write to '%s'", cppOutput.getPath());
		else if(hOutput.isFile() && !hOutput.canWrite())
			log(FATAL_ERROR, "Can't write to '%s'", hOutput  .getPath());
		
		RegisteredFile inputFile = RegisteredFile.registerNewFile(inputFileObject, infileName);
		parseFile(inputFile, cppOutput, hOutput, macros);
	}
	
	private static String getPathAndFirstName(File inputFileObject, File outputDir) {
		String fullInfileName = inputFileObject.getName();
		int indexOfDot = fullInfileName.indexOf('.');
		String firstName = indexOfDot < 0 ? fullInfileName : fullInfileName.substring(0, indexOfDot);
		
		return outputDir.getPath()+File.separatorChar + firstName;
	}
	
	private static void parseFile(RegisteredFile inputFile, File cppOutput, File hOutput, MacroMap macros) {
		LiveTokenizer tokenizer = new LiveTokenizer(inputFile, macros); //The future is here
		terminateIfErrorsOccured(); //Highly unlikely, but might happen (?)
		List<Definition> definitions = SyntaxicParser.getDefinitions(inputFile, tokenizer);
		if(definitions != null)
			for(Definition d:definitions) {
				println(d.getSignature());
			}
		
		terminateIfErrorsOccured(); //A lot more likely
		
		MainCodegen.generateCppAndHeader(definitions, cppOutput, hOutput);
	}
	
	private static void doOnlyPreproc(String infileName, String outputDirName, MacroMap macros) {
		File inputFileObject = new File(infileName);
		if(!inputFileObject.isFile())
			log(FATAL_ERROR, "The input file '%s' doesn't exist", infileName);
		
		RegisteredFile inputFile = RegisteredFile.registerNewFile(inputFileObject, infileName);
		
		PreProcessor preProcessor = new PreProcessor(inputFile, macros);
		
		String outfileName = (infileName.endsWith(".daf") ? infileName.substring(0, infileName.length()-4) : infileName)+"-preproc.daf";
		try(PrintWriter out = new PrintWriter(new FileWriter(outfileName))) {
			while(true) {
				out.print(preProcessor.getCurrentChar());
				if(!preProcessor.advance())
					break;
			}
			out.flush();
			out.close();
		} catch(IOException e) {
			e.printStackTrace();
			log(ERROR, "Could not write to file '%s'!", outfileName);
		}
		
		log(MESSAGE, "Writing of pre-processed file finished!");
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

	private static void doPreprocOnly() {
		if(onlyDoPreProc) {
			log(WARNING, "Option to only do pre processing was used twice!");
		}
		onlyDoPreProc = true;
	}
}
