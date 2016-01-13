package me.haved.dafParser;

public class DafParserMain {
	public static void main(String[] args) {
		System.out.println("The daf parser!");
		if(args.length==0) {
			System.out.println("You need to pass arguments. -h for help");
			return;
		}
		if(args[0].equals("-h") | args[0].equals("--help") | args[0].equals("?")) {
			System.out.println("Help file for the daf parser.\n"
					+ "Turns .daf files into .h and .cpp files\n"
					+ "daf inputDafFile\n"
					+ "daf inputDafFile outputDir");
		}
	}
}
