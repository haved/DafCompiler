package me.haved.daf;

import java.io.PrintStream;

import me.haved.daf.lexer.Token;

public class LogHelper {
	
	public static final int SUPER_DEBUG = 0;
	public static final int DEBUG = 1;
	public static final int INFO = 2;
	public static final int MESSAGE = 3;
	public static final int SUGGESTION = 4;
	public static final int WARNING = 5;
	public static final int ERROR = 6;
	public static final int FATAL_ERROR = 7;
	
	private static final String[] LOG_LEVEL_NAMES = {"super_debug", "debug", "info", "message",
					"suggestion", "warning", "error", "fatal_error"};
	
	private static final String COMPILER_NAME = "daf";
	
	public static PrintStream out = System.out;
	
	public static void println(String text) {
		out.println(text);
	}
	
	public static void println(String format, Object... objects) {
		out.println(String.format(format, objects));
	}
	
	public static void log(int logLevel, String format, Object... objects) {
		log(logLevel, String.format(format, objects));
	}
	
	public static void log(int logLevel, String text) {
		log(COMPILER_NAME, logLevel, text);
	}
	
	public static void log(String location, int logLevel, String format, Object... objects) {
		log(location, logLevel, String.format(format, objects));
	}
	
	public static void log(String location, int logLevel, String text) {
		println("%s: %s: %s", location, LOG_LEVEL_NAMES[logLevel], text);
		if(logLevel == FATAL_ERROR) {
			printSummary(-1);
			System.exit(-1);
			assert(false);
		}
	}
	
	public static void log(Token token, int logLevel, String format, Object... objects) {
		if(token==null)
			log(logLevel, String.format(format, objects));
		else
			log(String.format("%s:\"%s\"", token.getErrorLocation(), token.getTokenContents()), logLevel, String.format(format, objects));
	}
	
	private static long startTime;
	
	public static void startSummaryTime() {
		startTime = System.currentTimeMillis();
	}
	
	public static void printSummary(int errorStatus) {
		println("Finished in %.2fs with error status %d",
						(System.currentTimeMillis()-startTime)/1000f, errorStatus);
	}
}
