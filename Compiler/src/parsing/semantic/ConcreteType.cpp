#include "parsing/semantic/ConcreteType.hpp"
#include "parsing/ast/ExprTypeInfo.hpp"
#include "parsing/semantic/TypeConversion.hpp"
#include "CodegenLLVM.hpp"
#include <map>

void printConcreteTypeKind(ConcreteTypeKind kind, std::ostream& out) {
    switch(kind) {
	case ConcreteTypeKind::FUNCTION: out << "function"; break;
	case ConcreteTypeKind::POINTER: out << "pointer"; break;
	case ConcreteTypeKind::PRIMITIVE: out << "primitive"; break;
	case ConcreteTypeKind::VOID: out << "void"; break;
	default: assert(false); break;
	}
}

CTypeKindFilter::CTypeKindFilter(int filter) : filter(filter) {}
CTypeKindFilter CTypeKindFilter::allowingNothing() {return 0;}
CTypeKindFilter CTypeKindFilter::alsoAllowing(ConcreteTypeKind kind) const {
    int bitMask = 1 << (int)kind;
	assert(bitMask);
	return filter|bitMask;
}
CTypeKindFilter CTypeKindFilter::butDisallowing(ConcreteTypeKind kind) const {
	return inversed().alsoAllowing(kind).inversed();}
CTypeKindFilter CTypeKindFilter::ored(const CTypeKindFilter& other) const {return filter | other.filter;}
CTypeKindFilter CTypeKindFilter::unioned(const CTypeKindFilter& other) const {return filter & other.filter;}
CTypeKindFilter CTypeKindFilter::inversed() const {return ~filter;}
bool CTypeKindFilter::allows(ConcreteTypeKind kind) {return filter & (1<<(int)kind); }
bool CTypeKindFilter::allows(ConcreteType* type) { return allows(type->getConcreteTypeKind()); }

ConcretePointerType::ConcretePointerType(bool mut, ConcreteType* target) : m_mut(mut), m_target(target) {
	assert(m_target);
}

void ConcretePointerType::printSignature() {
	auto& out = std::cout << "& ";
	if(m_mut)
		out << "mut ";
	m_target->printSignature();
}

ConcreteTypeKind ConcretePointerType::getConcreteTypeKind() const {
	return ConcreteTypeKind::POINTER;
}

bool ConcretePointerType::hasSize() {
	return m_target->hasSize();
}

ExprTypeInfo ConcretePointerType::getDerefResultExprTypeInfo() {
	return ExprTypeInfo(m_target, m_mut ? ValueKind::MUT_LVALUE : ValueKind::LVALUE);
}


CastPossible ConcretePointerType::canConvertTo(ValueKind fromKind, ExprTypeInfo& to) {
    if(to.type->getConcreteTypeKind() != ConcreteTypeKind::POINTER)
		return CastPossible::IMPOSSIBLE;

	ConcretePointerType* to_ptr_type = static_cast<ConcretePointerType*>(to.type);
	if(to_ptr_type->m_target != m_target)
		return CastPossible::IMPOSSIBLE;

	if(to_ptr_type->m_mut && !m_mut)
		return CastPossible::IMPOSSIBLE;

	return (getValueKindScore(fromKind) >= getValueKindScore(to.valueKind))
		? CastPossible::IMPLICITLY : CastPossible::IMPOSSIBLE;
}

optional<ExprTypeInfo> ConcretePointerType::getPossibleConversionTarget(ValueKind fromKind, CTypeKindFilter filter, ValueKind kind, CastPossible rights) {
	(void) fromKind; (void) filter; (void) kind; (void) rights;
    return boost::none; //TODO: Allow casting pointers to integers
}

optional<EvaluatedExpression> ConcretePointerType::codegenTypeConversionTo(CodegenLLVM& codegen, EvaluatedExpression from, ExprTypeInfo* target) {
	return castEvaluatedExpression(codegen, from, target);
}

llvm::Type* ConcretePointerType::codegenType(CodegenLLVM& codegen) {
	llvm::Type* targetType = m_target->codegenType(codegen);
	if(!targetType)
		return nullptr;
	return llvm::PointerType::getUnqual(targetType);
}


//@Optimize: unordered_map?
std::map<std::pair<bool, ConcreteType*>, unique_ptr<ConcretePointerType> > PointerToTypeMap;

ConcretePointerType* ConcretePointerType::toConcreteType(bool mut, ConcreteType* type) {
	std::pair<bool, ConcreteType*> key{mut, type};
    auto find = PointerToTypeMap.find(key);
	if(find != PointerToTypeMap.end())
		return find->second.get();
	ConcretePointerType* result = new ConcretePointerType(mut, type);
	PointerToTypeMap.insert({key, unique_ptr<ConcretePointerType>(result)});
	return result;
}


PrimitiveType::PrimitiveType(LiteralKind literalKind, TokenType token, bool floatingPoint, Signed isSigned, int bitCount) : m_literalKind(literalKind), m_token(token), m_floatingPoint(floatingPoint), m_signed(isSigned == Signed::Yes), m_bitCount(bitCount) {}

void PrimitiveType::printSignature() {
	std::cout << getTokenTypeText(m_token);
}

LiteralKind PrimitiveType::getLiteralKind() {
	return m_literalKind;
}

TokenType PrimitiveType::getTokenType() {
	return m_token;
}

bool PrimitiveType::isFloatingPoint() {
	return m_floatingPoint;
}

bool PrimitiveType::isSigned() {
	return m_signed;
}

int PrimitiveType::getBitCount() {
	return m_bitCount;
}

bool PrimitiveType::hasSize() {
	return true;
}

CastPossible PrimitiveType::canConvertTo(ValueKind fromKind, ExprTypeInfo& to) {
	(void) fromKind;
    if(isReferenceValueKind(to.valueKind))
		return CastPossible::IMPOSSIBLE;
	ConcreteType* B_t = to.type;
	ConcreteTypeKind B_k = B_t->getConcreteTypeKind();
	if(B_k != ConcreteTypeKind::PRIMITIVE) {
		assert(false && "TODO: Casting from primitives to non-primitives");
		return CastPossible::IMPOSSIBLE;
	}

	PrimitiveType* to_prim = castToPrimitveType(to.type);
	if(isFloatingPoint() && !to_prim->isFloatingPoint())
		return CastPossible::EXPLICITLY; //float to int
	if(to_prim->getBitCount() == 1)
		return CastPossible::IMPLICITLY; //'truncate' to bool is implicit
	if(getBitCount() > to_prim->getBitCount())
		return CastPossible::EXPLICITLY; //truncating is otherwise explicit
	return CastPossible::IMPLICITLY;
}

optional<ExprTypeInfo> PrimitiveType::getPossibleConversionTarget(ValueKind fromKind, CTypeKindFilter filter, ValueKind kind, CastPossible rights) {
	(void) fromKind; (void) filter; (void) kind; (void) rights;
    return boost::none; //TODO: Allow casting primitves to pointer
}

optional<EvaluatedExpression> PrimitiveType::codegenTypeConversionTo(CodegenLLVM& codegen, EvaluatedExpression from, ExprTypeInfo* target) {
	assert(target && !target->isReference());

	ConcreteType* to = target->type;
	PrimitiveType* to_prim = castToPrimitveType(to);

    if(isFloatingPoint() || to_prim->isFloatingPoint())
		assert(false && "We don't support");
	int fromBits = getBitCount();
	int toBits = to_prim->getBitCount();

	llvm::Value* val = from.getValue(codegen);

	if(fromBits != toBits) {
		if(toBits == 1) {
			llvm::Type* from_LLVM = codegenType(codegen);
			if(!from_LLVM)
				return boost::none;
			llvm::Value* zero = llvm::ConstantInt::get(from_LLVM, 0, false);
			val = codegen.Builder().CreateICmpNE(val, zero);
		} else {
			llvm::Type* to_LLVM = to->codegenType(codegen);
			if(!to_LLVM)
				return boost::none;
			bool SnotZ = isSigned();
			val = SnotZ ?
				codegen.Builder().CreateSExtOrTrunc(val, to_LLVM):
				codegen.Builder().CreateZExtOrTrunc(val, to_LLVM);
		}
	}

	return EvaluatedExpression(val, false, target);
}

llvm::Type* PrimitiveType::codegenType(CodegenLLVM& codegen) {
	return llvm::Type::getIntNTy(codegen.Context(), m_bitCount);
}


PrimitiveType primitiveTypes[] = {           //FLOAT
	PrimitiveType(LiteralKind::U8, U8_TOKEN,   false, Signed::No,  8),
	PrimitiveType(LiteralKind::I8, I8_TOKEN,   false, Signed::Yes, 8),
	PrimitiveType(LiteralKind::U16, U16_TOKEN, false, Signed::No,  16),
	PrimitiveType(LiteralKind::I16, I16_TOKEN, false, Signed::Yes, 16),
	PrimitiveType(LiteralKind::U32, U32_TOKEN, false, Signed::No,  32),
	PrimitiveType(LiteralKind::I32, I32_TOKEN, false, Signed::Yes, 32),
	PrimitiveType(LiteralKind::U64, U64_TOKEN, false, Signed::No,  64),
	PrimitiveType(LiteralKind::I64, I64_TOKEN, false, Signed::Yes, 64),
	PrimitiveType(LiteralKind::F32, F32_TOKEN, true,  Signed::NA,  32),
	PrimitiveType(LiteralKind::F64, F64_TOKEN, true,  Signed::NA,  64),
	PrimitiveType(LiteralKind::BOOL, BOOLEAN,  false, Signed::No,  1),
	PrimitiveType(LiteralKind::USIZE, USIZE,   false, Signed::No,  USIZE_BIT_COUNT),
	PrimitiveType(LiteralKind::ISIZE, ISIZE,   false, Signed::Yes, ISIZE_BIT_COUNT),
	PrimitiveType(LiteralKind::CHAR, CHAR,     false, Signed::No,  CHAR_BIT_COUNT),
};

int PrimitiveTypeCount = sizeof(primitiveTypes)/sizeof(*primitiveTypes);

//@Optimize: use a map
PrimitiveType* tokenTypeToPrimitiveType(TokenType type) {
	for(int i = 0; i < PrimitiveTypeCount; i++) {
		if(primitiveTypes[i].getTokenType() == type)
			return &primitiveTypes[i];
	}
	return nullptr;
}

//@Optimize: Use a map
PrimitiveType* literalKindToPrimitiveType(LiteralKind kind) {
	for(int i = 0; i < PrimitiveTypeCount; i++) {
		if(primitiveTypes[i].getLiteralKind() == kind)
			return &primitiveTypes[i];
	}
	return nullptr;
}

PrimitiveType* castToPrimitveType(ConcreteType* type) {
	assert(type && type->getConcreteTypeKind() == ConcreteTypeKind::PRIMITIVE);
	return static_cast<PrimitiveType*>(type);
}

VoidType voidType;

void VoidType::printSignature() {
	std::cout << "void";
}

bool VoidType::hasSize() {
	return false;
}

CastPossible VoidType::canConvertTo(ValueKind fromKind, ExprTypeInfo& to) {
	(void) fromKind; (void) to;
	return CastPossible::IMPOSSIBLE;
}

optional<ExprTypeInfo> VoidType::getPossibleConversionTarget(ValueKind fromKind, CTypeKindFilter filter, ValueKind kind, CastPossible rights) {
	(void) fromKind; (void) filter; (void) kind; (void) rights;
    return boost::none;
}


optional<EvaluatedExpression> VoidType::codegenTypeConversionTo(CodegenLLVM& codegen, EvaluatedExpression from, ExprTypeInfo* target) {
	(void) codegen; (void) from; (void) target;
	return boost::none;
}

llvm::Type* VoidType::codegenType(CodegenLLVM& codegen) {
    return llvm::Type::getVoidTy(codegen.Context());
}

VoidType* getVoidType() {
	return &voidType;
}
