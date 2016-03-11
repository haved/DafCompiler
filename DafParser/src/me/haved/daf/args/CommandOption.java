package me.haved.daf.args;

public interface CommandOption {
	public static final int WRONG_PARSE = 0;
	public static final int STOP_PARSING = -1;
	
	public int parseOption(String[] args, int pos);
	
	public String getName();
	public String getDescription();
}
