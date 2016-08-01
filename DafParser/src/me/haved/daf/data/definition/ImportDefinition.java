package me.haved.daf.data.definition;

import me.haved.daf.data.NodeBase;

public class ImportDefinition extends NodeBase implements Definition {	
	private String[] path;
	
	public ImportDefinition(String[] path) {
		this.path = path;
	}
	
	@Override
	public String getSignature() {
		StringBuilder out = new StringBuilder();
		out.append("import ");
		for(int i = 0; i < path.length; i++) {
			out.append(path[i]);
			out.append(i<path.length-1 ? "." : ";");
		}
		return out.toString();
	}

	@Override
	public boolean isPublic() {
		return false;
	}
}