package me.haved.daf.data.statement;

import me.haved.daf.data.NodeBase;

public class ScopeStatement extends NodeBase implements Statement{

	private Statement[] statements;
	
	public ScopeStatement(Statement[] statements) {
		this.statements = statements;
	}
	
	@Override
	public String getSignature() {
		StringBuilder builder = new StringBuilder().append("{");
		if(statements!=null) {
			builder.append("\n");
			for(int i = 0; i < statements.length; i++)
				builder.append(statements[i].getSignature()).append("\n"); //Statements that use semi-colon have that in their signature
		}
		builder.append("}");
		return builder.toString();
	}
}
