#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"

using namespace llvm;

// 创建一个简单的测试函数：计算 (a + 0) * 1
Function* createTestFunction(Module* module) {
    auto& context = module->getContext();

    // 创建函数类型：int @test(int a)
    FunctionType* funcType = FunctionType::get(
        Type::getInt32Ty(context),
        {Type::getInt32Ty(context)},
        false
    );

    Function* func = Function::Create(
        funcType,
        Function::ExternalLinkage,
        "test",
        module
    );

    // 创建基本块
    BasicBlock* entry = BasicBlock::Create(context, "entry", func);
    IRBuilder<> builder(entry);

    // 获取参数
    Value* argA = &*func->arg_begin();

    // 创建冗余计算：(a + 0) * 1
    Value* addZero = builder.CreateAdd(argA, ConstantInt::get(Type::getInt32Ty(context), 0));
    Value* mulOne = builder.CreateMul(addZero, ConstantInt::get(Type::getInt32Ty(context), 1));

    // 返回结果
    builder.CreateRet(mulOne);

    return func;
}

// 打印函数内容
void printFunction(Function* func) {
    errs() << "Function: " << func->getName() << "\n";
    for (auto& bb : *func) {
        errs() << "Basic Block: " << bb.getName() << "\n";
        for (auto& inst : bb) {
            errs() << "  ";
            inst.print(errs());
            errs() << "\n";
        }
    }
    errs() << "\n";
}

int main() {
    // 创建模块和上下文
    LLVMContext context;
    std::unique_ptr<Module> module = std::make_unique<Module>("test_module", context);

    // 创建测试函数
    Function* testFunc = createTestFunction(module.get());

    // 打印优化前的函数
    errs() << "=== Before Instruction Combining ===\n";
    printFunction(testFunc);

    // 验证函数
    if (verifyFunction(*testFunc, &errs())) {
        errs() << "Function verification failed!\n";
        return 1;
    }

    // 创建 Pass Manager
    legacy::FunctionPassManager fpm(module.get());

    // 添加 Instruction Combining Pass
    fpm.add(createInstructionCombiningPass());

    // 运行优化
    fpm.doInitialization();
    fpm.run(*testFunc);
    fpm.doFinalization();

    // 打印优化后的函数
    errs() << "=== After Instruction Combining ===\n";
    printFunction(testFunc);

    // 再次验证函数
    if (verifyFunction(*testFunc, &errs())) {
        errs() << "Function verification failed after optimization!\n";
        return 1;
    }

    errs() << "Optimization completed successfully!\n";
    return 0;
}