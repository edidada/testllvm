#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/Reassociate.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"

using namespace llvm;

// 创建更复杂的测试函数
Function* createComplexFunction(Module* module) {
    auto& context = module->getContext();

    FunctionType* funcType = FunctionType::get(
        Type::getInt32Ty(context),
        {Type::getInt32Ty(context), Type::getInt32Ty(context)},
        false
    );

    Function* func = Function::Create(
        funcType,
        Function::ExternalLinkage,
        "complex_calc",
        module
    );

    BasicBlock* entry = BasicBlock::Create(context, "entry", func);
    IRBuilder<> builder(entry);

    auto args = func->arg_begin();
    Value* a = &*args++;
    Value* b = &*args;

    // 多个冗余计算
    Value* add1 = builder.CreateAdd(a, ConstantInt::get(Type::getInt32Ty(context), 0));  // a + 0
    Value* mul1 = builder.CreateMul(add1, ConstantInt::get(Type::getInt32Ty(context), 1)); // * 1

    Value* sub1 = builder.CreateSub(b, ConstantInt::get(Type::getInt32Ty(context), 0));   // b - 0
    Value* mul2 = builder.CreateMul(sub1, ConstantInt::get(Type::getInt32Ty(context), 1)); // * 1

    Value* result = builder.CreateAdd(mul1, mul2);

    // 冗余的条件判断
    Value* cmp = builder.CreateICmpEQ(a, a);  // a == a (总是 true)

    BasicBlock* thenBB = BasicBlock::Create(context, "then", func);
    BasicBlock* elseBB = BasicBlock::Create(context, "else", func);
    BasicBlock* mergeBB = BasicBlock::Create(context, "merge", func);

    builder.CreateCondBr(cmp, thenBB, elseBB);

    // Then 块
    builder.SetInsertPoint(thenBB);
    Value* thenVal = builder.CreateAdd(result, ConstantInt::get(Type::getInt32Ty(context), 0));
    builder.CreateBr(mergeBB);

    // Else 块（实际上不会执行）
    builder.SetInsertPoint(elseBB);
    Value* elseVal = builder.CreateSub(result, ConstantInt::get(Type::getInt32Ty(context), 0));
    builder.CreateBr(mergeBB);

    // Merge 块
    builder.SetInsertPoint(mergeBB);
    PHINode* phi = builder.CreatePHI(Type::getInt32Ty(context), 2);
    phi->addIncoming(thenVal, thenBB);
    phi->addIncoming(elseVal, elseBB);

    builder.CreateRet(phi);

    return func;
}

void printFunction(Function* func) {
    errs() << "Function: " << func->getName() << "\n";
    for (auto& bb : *func) {
        errs() << "Basic Block: " << bb.getName() << ":\n";
        for (auto& inst : bb) {
            errs() << "  ";
            inst.print(errs());
            errs() << "\n";
        }
    }
    errs() << "\n";
}

int main() {
    LLVMContext context;
    std::unique_ptr<Module> module = std::make_unique<Module>("optimization_demo", context);

    Function* func = createComplexFunction(module.get());

    errs() << "=== Before Optimization ===\n";
    printFunction(func);

    // 验证函数
    if (verifyFunction(*func, &errs())) {
        errs() << "Function verification failed!\n";
        return 1;
    }

    // 创建并配置 Pass Manager
    legacy::FunctionPassManager fpm(module.get());

    // 添加多个优化 Pass
    fpm.add(createInstructionCombiningPass());  // 指令组合
    fpm.add(createReassociatePass());            // 重关联表达式
    fpm.add(createGVNPass());                    // 全局值编号
    fpm.add(createCFGSimplificationPass());      // CFG 简化

    // 运行优化
    fpm.doInitialization();
    fpm.run(*func);
    fpm.doFinalization();

    errs() << "=== After Optimization ===\n";
    printFunction(func);

    // 再次验证
    if (verifyFunction(*func, &errs())) {
        errs() << "Function verification failed after optimization!\n";
        return 1;
    }

    errs() << "All optimizations completed successfully!\n";
    return 0;
}