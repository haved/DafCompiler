package me.haved.dafParser;

import java.io.PrintStream;

public class LogHelper {
	
	private static PrintStream out = System.out;
	
	public static boolean ERRORS = true;
	public static boolean WARNINGS = true;
	public static boolean LOGS = false;
	public static boolean INFOS = false;
	
	public  static int errorCount = 0;
	private static int maxErrorCount = 20;
	
	public static void Error(String s) {
		if(!ERRORS || errorCount > maxErrorCount)
			return;
		
		if(errorCount==maxErrorCount)
			Println("Max ERROR count (%d) reached.", maxErrorCount);
		else
			out.printf("EROOR: %s%n", s);
		errorCount++;
	}
	
	public static void Error(String format, Object... args) {
		Error(String.format(format, args));
	}
	
	public static void LineError(String inputFile, int line, String format, Object... args) {
		Error(String.format("ERROR(%s:%d): %s%n", inputFile, line, String.format(format, args)));
	}
	
	public static void Warning(String s) {
		if(WARNINGS)
			out.printf("WARNING: %s%n", s);
	}
	
	public static void Warning(String format, Object... args) {
		if(WARNINGS)
			out.printf("WARNING: %s%n", String.format(format, args));
	}
	
	public static void LineWarning(String inputFile, int line, String format, Object... args) {
		if(WARNINGS)
			out.printf("WARNING(%s:%d): %s%n", inputFile, line, String.format(format, args));
	}
	
	public static void Log(String s) {
		if(LOGS)
			out.printf("LOG: %s%n", s);
	}
	
	public static void Log(String format, Object... args) {
		if(LOGS)
			out.printf("LOG: %s%n", String.format(format, args));
	}
	
	public static void LineLog(String inputFile, int line, String format, Object... args) {
		if(LOGS)
			out.printf("LOG(%s:%d): %s%n", inputFile, line, String.format(format, args));
	}
	
	public static void Info(String s) {
		if(INFOS)
			out.printf("INFO: %s%n", s);
	}
	
	public static void Info(String format, Object... args) {
		if(INFOS)
			out.printf("INFO: %s%n", String.format(format, args));
	}
	
	public static void LineInfo(String inputFile, int line, String format, Object... args) {
		if(INFOS)
			out.printf("INFO(%s:%d): %s%n", inputFile, line, String.format(format, args));
	}
	
	public static void Println(String line) {
		out.println(line);
	}
	
	public static void Println(String format, Object... args) {
		out.printf("%s%n", String.format(format, args));
	}
}
