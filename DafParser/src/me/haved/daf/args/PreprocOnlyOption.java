package me.haved.daf.args;

public class PreprocOnlyOption implements CommandOption {
	
	private SetToDoOnlyPreProc stdopp;
	
	public PreprocOnlyOption(SetToDoOnlyPreProc stdopp) {
		this.stdopp = stdopp;
	}
	
	@Override
	public int parseOption(String[] args, int pos) {
		
		if(args[pos].equals("-P") | args[pos].equals("--preproc")) {
			stdopp.doOnlyPreProc();
			return 1;
		}
			
		return WRONG_PARSE;
	}

	@Override
	public String getName() {
		return "-P --preproc";
	}

	@Override
	public String getDescription() {
		return "Only do pre-processing and store it in the output directory as <name>-preproc.daf";
	}
	
	public static interface SetToDoOnlyPreProc {
		void doOnlyPreProc();
	}
}
