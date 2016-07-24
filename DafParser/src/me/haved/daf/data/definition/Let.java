package me.haved.daf.data.definition;

import me.haved.daf.data.NodeBase;
import me.haved.daf.data.expression.Expression;
import me.haved.daf.data.statement.Statement;
import me.haved.daf.data.type.Type;

public class Let extends NodeBase implements Definition, Statement {

	private String name;
	private Type type;
	private Expression expression;
	private boolean pub;
	
	public Let(String name, Type type, Expression expression, boolean pub) {
		this.name = name;
		this.type = type;
		this.expression = expression;
		this.pub = pub;
	}
	
	public Let(String name, Type type, Expression expression) {
		this(name, type, expression, false);
	}
	
	@Override
	public String toString() {
		if(expression!=null)
			return String.format("%slet %s : %s = %s;", pub?"pub ":"", name, type==null?"null":type.getSignature(), expression.toString());
		else
			return String.format("%slet %s : %s;", pub?"pub ":"", name, type==null?"null":type.getSignature());
	}

	@Override
	public boolean isPublic() {
		return pub;
	}
}
