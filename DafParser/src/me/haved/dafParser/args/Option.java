package me.haved.dafParser.args;

import me.haved.dafParser.DafParser;

public interface Option {
	
	/**
	 * 
	 * @param s the argument from the command line
	 * @return if this class can handle the option
	 */
	public boolean canParseOption(String s);
	
	/** A method that does everything to be done with an option
	 * 
	 * @param args The array of arguments passed to the compiler
	 * @param arg The index of the argument currently being parsed
	 * @return Amount of arguments it took. 0 means it will stop taking more arguments. (Failed)
	 */
	public int parseOption(DafParser parser, String[] args, int arg);
	
	public String getOptionName();
	
	public String getOptionsForHelpText();
	
	public String getExplanationForHelpText();
}
