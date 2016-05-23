package me.haved.daf.data;

import java.io.PrintStream;

public class ImportDefinition implements Definition {
	
	private String[] path;
	
	public ImportDefinition(String[] path) {
		this.path = path;
	}
	
	@Override
	public void print(PrintStream out) {
		out.print("import ");
		for(int i = 0; i < path.length; i++) {
			out.println(path[i]);
			out.print(i<path.length-1 ? "." : ";\n");
		}
	}

	@Override
	public boolean isPublic() {
		return false;
	}
}