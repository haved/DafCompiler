package me.haved.daf.data.definition;

import java.io.PrintWriter;

import me.haved.daf.data.NodeBase;

public class ImportDefinition extends NodeBase implements Definition {
	private String[] parts;
	private String cppLibName;
	private boolean cppLib;
	
	public ImportDefinition(String[] parts) {
		this.parts = parts;
		this.cppLib = false;
	}
	
	public ImportDefinition(String name) {
		this.cppLibName = name;
		this.cppLib = true;
	}
	
	@Override
	public String getSignature() {
		if(cppLib)
			return String.format("import \"%s\";", cppLibName);
		StringBuilder out = new StringBuilder();
		out.append("import ");
		for(int i = 0; i < parts.length; i++) {
			if(i!=0)
				out.append(".");
			out.append(parts[i]);
		}
		out.append(";");
		return out.toString();
	}

	@Override
	public boolean isPublic() {
		return false;
	}

	@Override
	public void codegenDefinitionCpp(PrintWriter cpp, PrintWriter h) {
		
	}
}