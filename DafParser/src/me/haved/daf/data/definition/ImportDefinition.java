package me.haved.daf.data.definition;

import java.io.PrintWriter;

import me.haved.daf.data.NodeBase;

public class ImportDefinition extends NodeBase implements Definition {
	private String[] parts;
	
	public ImportDefinition(String[] parts) {
		this.parts = parts;
	}
	
	@Override
	public String getSignature() {
		return null;
	}

	@Override
	public boolean isPublic() {
		return false;
	}

	@Override
	public void codegenDefinitionCpp(PrintWriter cpp, PrintWriter h) {
		
	}
}