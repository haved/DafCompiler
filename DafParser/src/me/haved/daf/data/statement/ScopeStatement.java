package me.haved.daf.data.statement;

import static me.haved.daf.LogHelper.logAssert;

import java.io.PrintWriter;

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
			for(int i = 0; i < statements.length; i++)
				builder.append("\n").append(statements[i].getSignature()); //Statements that use semi-colon have that in their signature
		}
		builder.append("\n}");
		return builder.toString();
	}

	@Override
	public boolean isValidStatement() {
		return true;
	}
	
	@Override
	public void codegenStatementCpp(PrintWriter cpp) {
		if(statements != null)
		cpp.println("{ ");
		for(Statement statement:statements) {
			logAssert(statement != null); //No real point, but rehearsing for later, you know.
			statement.codegenStatementCpp(cpp);
		}
		cpp.println("}");
	}
}
