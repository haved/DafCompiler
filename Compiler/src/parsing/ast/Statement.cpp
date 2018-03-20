#include "parsing/ast/Statement.hpp"
#include "parsing/semantic/ConcretableHelp.hpp"
#include "CodegenLLVM.hpp"
#include "parsing/semantic/TypeConversion.hpp"
#include <iostream>

Statement::Statement(const TextRange& range) : m_range(range) {
	//	std::cout << "Statement at: " << range.getLine() << ":" << range.getCol() << "-"
	//		  << range.getLastLine() << ":" << range.getEndCol() << std::endl;
}

Statement::~Statement() {}

const TextRange& Statement::getRange() {
	return m_range;
}

void Statement::addToMap(NamedDefinitionMap& map) {
	//@Speed: a lot of virtual calls that never do anything
	(void) map; //Definition overrides this, the others don't do nothing
}

ConcretableState Statement::retryMakeConcreteInternal(DependencyMap& depMap) {
	(void) depMap;
	return ConcretableState::CONCRETE;
}

DefinitionStatement::DefinitionStatement(unique_ptr<Definition>&& definition, const TextRange& range) : Statement(range), m_definition(std::move(definition))
{
	assert(m_definition && !m_definition->isPublic());
}

void DefinitionStatement::printSignature() {
	m_definition->printSignature();
}

void DefinitionStatement::addToMap(NamedDefinitionMap& map) {
	m_definition->addToMap(map);
}

ConcretableState DefinitionStatement::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
    ConcretableState state =  m_definition->makeConcrete(ns_stack, depMap);
	if(allConcrete() << state)
		return retryMakeConcreteInternal(depMap);
	if(anyLost() << state)
		return ConcretableState::LOST_CAUSE;
	depMap.makeFirstDependentOnSecond(this, m_definition.get());
	return ConcretableState::TRY_LATER;
}

void DefinitionStatement::codegenStatement(CodegenLLVM& codegen) {
	m_definition->localCodegen(codegen);
}


ExpressionStatement::ExpressionStatement(unique_ptr<Expression>&& expression, const TextRange& range)
	: Statement(range), m_expression(std::move(expression)) {
	assert(m_expression && m_expression->isStatement());
}

void ExpressionStatement::printSignature() {
	m_expression->printSignature();
	std::cout << ";" << std::endl; //Expressions are not used to this, you know
}

ConcretableState ExpressionStatement::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
    ConcretableState state =  m_expression->makeConcrete(ns_stack, depMap);
	if(allConcrete() << state)
		return retryMakeConcreteInternal(depMap);
	if(anyLost() << state)
		return ConcretableState::LOST_CAUSE;
	depMap.makeFirstDependentOnSecond(this, m_expression.get());
	return ConcretableState::TRY_LATER;
}

void ExpressionStatement::codegenStatement(CodegenLLVM& codegen) {
	m_expression->codegenExpression(codegen);
}

IfStatement::IfStatement(unique_ptr<Expression>&& condition, unique_ptr<Statement>&& body, unique_ptr<Statement>&& else_body, const TextRange& range)
	: Statement(range), m_condition(std::move(condition)), m_body(std::move(body)), m_else_body(std::move(else_body)) {
	assert(m_condition);
}

void IfStatement::printSignature() {
	std::cout << "if ";
	m_condition->printSignature();
	std::cout << " ";
	if(m_body) {
		m_body->printSignature();
	} else {
		std::cout << ";" << std::endl;
	}

	if(m_else_body) {
		std::cout << "else ";
		m_else_body->printSignature();
	}
}

ConcretableState IfStatement::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {

	auto conc = allConcrete();
	auto lost = anyLost();

	auto tryRequire = [&](Concretable* obj) {
		if(!obj) return;
		ConcretableState state = obj->makeConcrete(ns_stack, depMap);
		depMap.markSecondAsDependencyIfUnfinished(this, obj);
		conc = conc << state;
		lost = lost << state;
	};

	assert(m_condition);
	tryRequire(m_condition.get());
	tryRequire(m_body.get());
	tryRequire(m_else_body.get());

	if(conc)
		return retryMakeConcreteInternal(depMap);
	if(lost)
		return ConcretableState::LOST_CAUSE;
	return ConcretableState::TRY_LATER;
}

ConcretableState IfStatement::retryMakeConcreteInternal(DependencyMap& depMap) {
	(void) depMap;
	ExprTypeInfo condTypeInfo = m_condition->getTypeInfo();
	CastPossible poss = canConvertTypeFromTo(condTypeInfo, *getAnonBooleanTyI());
	if(poss != CastPossible::IMPLICITLY) {
		complainThatTypeCantBeConverted(condTypeInfo, *getAnonBooleanTyI(), poss, m_condition->getRange());
		return ConcretableState::LOST_CAUSE;
	}
	return ConcretableState::CONCRETE;
}

void IfStatement::codegenStatement(CodegenLLVM& codegen) {
	optional<EvaluatedExpression> cond = m_condition->codegenExpression(codegen);
	cond = codegenTypeConversion(codegen, cond, getAnonBooleanTyI());
	if(!cond)
		return;

	llvm::Function* func = codegen.Builder().GetInsertBlock()->getParent();
	llvm::BasicBlock* MergeBB = llvm::BasicBlock::Create(codegen.Context(), "merge");
	llvm::BasicBlock* ThenBB = m_body ? llvm::BasicBlock::Create(codegen.Context(), "then") : MergeBB;
	llvm::BasicBlock* ElseBB = m_else_body ? llvm::BasicBlock::Create(codegen.Context(), "else") : MergeBB;


	codegen.Builder().CreateCondBr(cond->getValue(codegen), ThenBB, ElseBB);

	if(m_body) {
		func->getBasicBlockList().push_back(ThenBB);
		codegen.Builder().SetInsertPoint(ThenBB);
		m_body->codegenStatement(codegen);
		codegen.Builder().CreateBr(MergeBB);
	}

	if(m_else_body) {
		func->getBasicBlockList().push_back(ElseBB);
		codegen.Builder().SetInsertPoint(ElseBB);
		m_else_body->codegenStatement(codegen);
		codegen.Builder().CreateBr(MergeBB);
	}

	func->getBasicBlockList().push_back(MergeBB);
	codegen.Builder().SetInsertPoint(MergeBB);
}


WhileStatement::WhileStatement(unique_ptr<Expression>&& condition, unique_ptr<Statement>&& body, const TextRange& range)
	: Statement(range), m_condition(std::move(condition)), m_body(std::move(body)) {
	assert(m_condition);
}

void WhileStatement::printSignature() {
	std::cout << "while ";
	m_condition->printSignature();
	std::cout << " ";
	if(m_body) {
		m_body->printSignature();
	} else {
		std::cout << ";" << std::endl;
	}
}

ConcretableState WhileStatement::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
	auto conc = allConcrete();
	auto lost = anyLost();
	auto require = [&](Concretable* c) {
		ConcretableState state = c->makeConcrete(ns_stack, depMap);
		conc <<= state;
		lost <<= state;
		if(state == ConcretableState::TRY_LATER)
			depMap.makeFirstDependentOnSecond(this, c);
	};
	require(m_condition.get());
	require(m_body.get());
	if(conc)
		return ConcretableState::CONCRETE;
	if(lost)
		return ConcretableState::LOST_CAUSE;
	return ConcretableState::TRY_LATER;
}

ConcretableState WhileStatement::retryMakeConcreteInternal(DependencyMap& depMap) {
	(void) depMap;
	ExprTypeInfo condType = m_condition->getTypeInfo();
	CastPossible poss = canConvertTypeFromTo(condType, *getAnonBooleanTyI());
	if(poss != CastPossible::IMPLICITLY) {
		complainThatTypeCantBeConverted(condType, *getAnonBooleanTyI(), poss, m_condition->getRange());
		return ConcretableState::LOST_CAUSE;
	}
	return ConcretableState::CONCRETE;
}

void WhileStatement::codegenStatement(CodegenLLVM& codegen) {
	llvm::Function* func = codegen.Builder().GetInsertBlock()->getParent();
	llvm::BasicBlock* TestBB = llvm::BasicBlock::Create(codegen.Context(), "whileTest");
	llvm::BasicBlock* BodyBB = llvm::BasicBlock::Create(codegen.Context(), "whileBody");
	llvm::BasicBlock* MergeBB = llvm::BasicBlock::Create(codegen.Context(), "whileEnd");

	codegen.Builder().CreateBr(TestBB);
	func->getBasicBlockList().push_back(TestBB);
	codegen.Builder().SetInsertPoint(TestBB);

	optional<EvaluatedExpression> eval = m_condition->codegenExpression(codegen);
	optional<EvaluatedExpression> eval_cast = codegenTypeConversion(codegen, eval, getAnonBooleanTyI());
	if(!eval_cast)
		return;
	codegen.Builder().CreateCondBr(eval_cast->getValue(codegen), BodyBB, MergeBB);

	func->getBasicBlockList().push_back(BodyBB);
	codegen.Builder().SetInsertPoint(BodyBB);
	m_body->codegenStatement(codegen);
	codegen.Builder().CreateBr(TestBB);

	func->getBasicBlockList().push_back(MergeBB);
	codegen.Builder().SetInsertPoint(MergeBB);
}

ForStatement::ForStatement(unique_ptr<Expression>&& iterator, unique_ptr<Statement>&& body, const TextRange& range)
	: Statement(range), m_iterator(std::move(iterator)), m_body(std::move(body)) {
	assert(m_iterator); //Body my be ';', a.k.a. null
}

void ForStatement::printSignature() {
	std::cout << "for ";
	m_iterator->printSignature(); //asserted not null
	if(m_body) {
		std:: cout << " ";
		m_body->printSignature();
	} else {
		std::cout << ";";
	}
}

ReturnStatement::ReturnStatement(unique_ptr<Expression>&& value, const TextRange& range) : Statement(range), m_returnValue(std::move(value)), m_funcExpr(), m_returnTypeExpected(getNoneTypeInfo()) {} //Don't assert a return value

void ReturnStatement::printSignature() {
	if(m_returnValue) {
		std::cout << "return ";
		m_returnValue->printSignature();
		std::cout << ";" << std::endl;
	} else
		std::cout << "return;" << std::endl;
}

bool ReturnStatement::isEndOfBlock() {
	return true;
}

ConcretableState ReturnStatement::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {

	auto lost = anyLost();
	auto conc = allConcrete();

	m_funcExpr = ns_stack.getBlockLevelInfo().getCurrentFunction();
    ConcretableState state = m_funcExpr->getConcretableState();
    lost <<= state;
	conc <<= state;
	if(!lost && !conc)
		depMap.makeFirstDependentOnSecond(this, m_funcExpr);

	if(m_returnValue) {
		state = m_returnValue->makeConcrete(ns_stack, depMap);
	    conc <<= state;
		conc <<= state;
		if(tryLater(state))
			depMap.makeFirstDependentOnSecond(this, m_returnValue.get());
	}

	return conc ? ConcretableState::CONCRETE : ConcretableState::TRY_LATER;
}

ConcretableState ReturnStatement::retryMakeConcreteInternal(DependencyMap& depMap) {
	(void) depMap;
	m_returnTypeExpected = m_funcExpr->getReturnTypeInfo();

	if(m_returnValue) {
		ExprTypeInfo givenType = m_returnValue->getTypeInfo();
		CastPossible poss = canConvertTypeFromTo(givenType, m_returnTypeExpected);
		if(poss != CastPossible::IMPLICITLY) {
			complainThatTypeCantBeConverted(givenType, m_returnTypeExpected, poss, m_returnValue->getRange());
			return ConcretableState::LOST_CAUSE;
		}
	} else {
		ExprTypeInfo anonVoidTypeI(getVoidType(), ValueKind::ANONYMOUS);
		CastPossible poss = canConvertTypeFromTo(anonVoidTypeI, m_returnTypeExpected);
		if(poss != CastPossible::IMPLICITLY) {
			complainThatTypeCantBeConverted(anonVoidTypeI, m_returnTypeExpected, poss, getRange());
			return ConcretableState::LOST_CAUSE;
		}
	}
	return ConcretableState::CONCRETE;
}

void ReturnStatement::codegenStatement(CodegenLLVM& codegen) {
	if(m_returnValue) {
		optional<EvaluatedExpression> retEval = m_returnValue->codegenExpression(codegen);
		optional<EvaluatedExpression> castedRet = codegenTypeConversion(codegen, retEval, &m_returnTypeExpected);
		if(!castedRet)
			return;
		bool refRet = m_funcExpr->hasReferenceReturn();
		codegen.Builder().CreateRet(refRet ? castedRet->getPointerToValue(codegen) : castedRet->getValue(codegen));
	} else {
		codegen.Builder().CreateRetVoid();
	}
}

LoopStatement::LoopStatement(LoopStatementType type, const TextRange& range) : Statement(range), m_type(type) {}

void LoopStatement::printSignature() {
	switch(m_type) {
	case LoopStatementType::BREAK:    std::cout << "break;"    << std::endl; break;
	case LoopStatementType::CONTINUE: std::cout << "continue;" << std::endl; break;
	case LoopStatementType::RETRY:    std::cout << "retry;"    << std::endl; break;
	default: assert(false);
	}
}

bool LoopStatement::isEndOfBlock() {
	return true;
}
