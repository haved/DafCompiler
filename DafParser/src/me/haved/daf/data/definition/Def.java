package me.haved.daf.data.definition;

import java.io.PrintStream;

import me.haved.daf.data.NodeBase;
import me.haved.daf.data.expression.Expression;
import me.haved.daf.data.statement.Statement;
import me.haved.daf.data.type.Type;

public class Def extends NodeBase implements Definition, Statement {
	
	private String name;
	private Type type;
	private Expression expression;
	private boolean pub;
	
	public Def(String name, Type type, Expression expression, boolean pub) {
		this.name = name;
		this.type = type;
		this.expression = expression;
		this.pub = pub;
	}
	
	@Override
	public void print(PrintStream out) {
		out.printf("%sdef %s : %s = %s;%n", pub?"pub ":"", name, type==null?"null":type.getSignature(), expression==null?"null":expression.toString());
	}

	@Override
	public boolean isPublic() {
		return pub;
	}

}