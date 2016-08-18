package me.haved.daf.data.statement;

import me.haved.daf.data.NodeBase;
import me.haved.daf.data.expression.Expression;
import me.haved.daf.lexer.tokens.Token;

import static me.haved.daf.LogHelper.*;

import java.io.PrintWriter;

public class ReturnStatement extends NodeBase implements Statement{

	private Expression value;
	
	public ReturnStatement(Expression value) {
		logAssert(value != null);
		this.value = value;
		this.endLine = value.getEndLine();
		this.endCol = value.getEndCol();
	}
	
	public ReturnStatement setStart(Token start) {
		this.line = start.getLine();
		this.col = start.getCol();
		return this;
	}
	
	@Override
	public String getSignature() {
		return String.format("return %s;", value);
	}

	@Override
	public boolean isValidStatement() {
		return true;
	}
	
	@Override
	public void codegenStatementCpp(PrintWriter cpp) {
		cpp.print("return ");
		if(value != null)
			value.codegenExpressionCpp(cpp);
		cpp.println(";");
	}
}
