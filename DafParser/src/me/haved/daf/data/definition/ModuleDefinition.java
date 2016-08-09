package me.haved.daf.data.definition;

import me.haved.daf.data.NodeBase;

public class ModuleDefinition extends NodeBase implements Definition {
	
	private String name;
	private Definition[] definitons;
	private boolean pub;
	
	public ModuleDefinition(String name, Definition[] definitions, boolean pub) {
		this.name = name;
		this.definitons = definitions;
		this.pub = pub;
	}

	@Override
	public String getSignature() {
		StringBuilder builder = new StringBuilder();
		if(pub)
			builder.append("pub ");
		builder.append("module ");
		builder.append(name);
		if(definitons != null) {
			builder.append(" {\n");
			for(int i = 0; i < definitons.length; i++) {
				builder.append(definitons[i]).append("\n");
			}
			builder.append("}");
		} else
			builder.append(";");
		return builder.toString();
	}

	@Override
	public boolean isPublic() {
		return false;
	}
}
