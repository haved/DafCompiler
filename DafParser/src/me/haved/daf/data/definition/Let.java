package me.haved.daf.data.definition;

import static me.haved.daf.LogHelper.*;

import java.io.PrintWriter;

import me.haved.daf.data.NodeBase;
import me.haved.daf.data.expression.Expression;
import me.haved.daf.data.statement.Statement;
import me.haved.daf.data.type.Type;

public class Let extends NodeBase implements Definition, Statement {

	private String name;
	private boolean mut;
	private Type type;
	private Expression expression;
	private boolean pub;
	
	public Let(String name, boolean mut, Type type, Expression expression, boolean pub) {
		this.name = name;
		this.mut = mut;
		this.type = type;
		this.expression = expression;
		this.pub = pub;
	}
	
	public Let(String name, boolean mut, Type type, Expression expression) {
		this(name, mut, type, expression, false);
	}
	
	@Override
	public String getSignature() {
		if(expression!=null)
			return String.format("%slet %s%s : %s = %s;", pub?"pub ":"", mut?"mut ":"", name, type==null?"":type.getSignature(), expression.toString());
		else
			return String.format("%slet %s%s : %s;", pub?"pub ":"", mut?"mut ":"", name, type==null?"":type.getSignature());
	}

	@Override
	public boolean isPublic() {
		return pub;
	}

	@Override
	public boolean isValidStatement() {
		return true;
	}

	@Override
	public void codegenDefinitionCpp(PrintWriter cpp, PrintWriter h) {
		if(type == null)
			log(FATAL_ERROR, "Sorry, but a global let statement can't be without type");
		if(expression == null)
			log(FATAL_ERROR, "Sorry, but a global let statement can't be without expression");
		cpp.printf("%s %s = ", type.codegenCpp(), name);
		expression.codegenExpressionCpp(cpp);
		cpp.println(";");
		h.printf("extern %s %s;%n", type.codegenCpp(), name);
	}
	
	@Override
	public void codegenStatementCpp(PrintWriter cpp) {
		logAssert(false);
	}
}
