package me.haved.dafParser;

import java.io.PrintStream;

public class LogHelper {
	public static PrintStream out = System.out;
	
	public static final String COMPILER_NAME = "daf";
	
	public static final int INFO = 0;
	public static final int MESSAGE = 1;
	public static final int SUGGESTION = 2;
	public static final int WARNING = 3;
	public static final int ERROR = 4;
	public static final int FATAL_ERROR = 5;
	public static final int DEBUG = 6;
	
	public static final String[] logLevels = {"info", "message", "suggestion", "warning", "error", "fatal error", "debug"};
	
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
			assert(false); //Just to be sure the program doesn't get past System.exit()
		}
	}
	
	public static void log(String system, int logLevel, String format, Object... args) {
		log(system, logLevel, String.format(format, args));
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
	
	public static void logAssert(boolean p_assert, String error) {
		if(!p_assert)
			log(FATAL_ERROR, "Assertion failed: %s", error);
	}
	
	public static void terminateIfErrorsLogged() {
		if(logCounts[ERROR]!=0) {
			log(FATAL_ERROR, "Aborting due to previous %d errors!", logCounts[ERROR]);
		}
	}
	
	public static String getLogSystem(String systemName, int lineNumber, int lineChar) {
		return String.format("%s:%d:%d", systemName, lineNumber, lineChar);
	}
	
	public static String fileLocation(String fileName, int line, int col) {
		return String.format("%s:%d:%d", fileName, line, col);
	}
	
	public static void setMaxLogCount(int logLevel, int max) {
		maxLogCounts[logLevel] = max;
	}
	
	public static void enableVerboseLogging() {
		setMaxLogCount(INFO, 200);
		setMaxLogCount(MESSAGE, 200);
		setMaxLogCount(SUGGESTION, 200);
	}
	
	public static void setToSummarize(boolean sum) {
		summarize = sum;
	}
	
	public static void trySummarize() {
		if(!summarize)
			for(int i = 0; i < logCounts.length; i++)
				if(maxLogCounts[i] > 0 && logCounts[i]>maxLogCounts[i]) {
					log(WARNING, "Number of %s log entries exceeded the max number (%d)", logLevels[i], maxLogCounts[i]);
					summarize = true;
				}
		if(summarize)
			printLoggingInfo();
	}
	
	private static long startTime;
	
	public static void startTime() {
		startTime = System.currentTimeMillis();
	}
	
	public static void printLoggingInfo() {
		out.printf("Execution finished with %d infos, %d messages, %d suggestions, %d warnings, %d errors and %d fatal errors%n",
				logCounts[0], logCounts[1], logCounts[2], logCounts[3], logCounts[4], logCounts[5]);
		out.printf("Execution time: %.2fs%n", (System.currentTimeMillis()-startTime)/1000f);
	}
}
