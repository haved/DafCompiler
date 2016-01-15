package me.haved.dafParser;

import java.io.PrintStream;

public class LogHelper {
	public static PrintStream out = System.out;
	
	public static final String COMPILER_NAME = "daf";
	
	public static final int INFO = 0;
	public static final int MESSAGE = 1;
	public static final int WARNING = 2;
	public static final int ERROR = 3;
	public static final int FATAL_ERROR = 4;
	public static final int DEBUG = 5;
	
	public static final String[] logLevels = {"info", "message", "warning", "error", "fatal error", "debug"};
	
	private static int[] logCounts = new int[logLevels.length];
	private static int[] maxLogCounts = new int[logLevels.length];
	
	private static boolean summarize = false;
	
	static {
		maxLogCounts[WARNING] = 20;
		maxLogCounts[ERROR]   = 20;
		maxLogCounts[FATAL_ERROR] = 20; //Should only ever happen once, but just in case it happens more.
	}
	
	public static void log(String system, int logLevel, String message) {
		logCounts[logLevel]++;
		if(logCounts[logLevel]<maxLogCounts[logLevel])
			out.printf("%s: %s: %s%n", system, logLevels[logLevel], message);
		if(logLevel == FATAL_ERROR) {
			trySummarize();
			System.exit(1);
		}
	}
	
	public static void log(int logLevel, String message) {
		log(COMPILER_NAME, logLevel, message);
	}
	
	public static void log(int logLevel, String format, Object... args) {
		log(logLevel, String.format(format, args));
	}
	
	public static void println(String message) {
		out.println(message);
	}
	
	public static void println(String format, Object... args) {
		out.println(String.format(format, args));
	}
	
	public static String getLogSystem(String systemName, int lineNumber, int lineChar) {
		return String.format("%s:%d:%d", systemName, lineNumber, lineChar);
	}
	
	public static void setMaxLogCount(int logLevel, int max) {
		maxLogCounts[logLevel] = max;
	}
	
	public static void setToSummarize(boolean sum) {
		summarize = sum;
	}
	
	public static void trySummarize() {
		if(summarize)
			printLoggingInfo();
	}
	
	private static long startTime;
	
	public static void startTime() {
		startTime = System.currentTimeMillis();
	}
	
	public static void printLoggingInfo() {
		out.printf("Execution finished with %d infos, %d messages, %d warnings, %d errors and %d fatal errors%n",
				logCounts[0], logCounts[1], logCounts[2], logCounts[3], logCounts[4]);
		out.printf("Execution time: %.2fs%n", (System.currentTimeMillis()-startTime)/1000f);
	}
}
