#include <cstdint>

#include "llvm/Config/llvm-config.h"
#include "llvm/ExecutionEngine/JITSymbol.h"
#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/ExecutionEngine/Orc/ThreadSafeModule.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include <iostream>

using namespace llvm;
using namespace llvm::orc;

int main(int argc, char* argv[]) {
    // 1. 初始化 LLVM
    InitLLVM X(argc, argv);
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();

    // 2. 创建 LLJIT
    auto JIT = cantFail(LLJITBuilder().create());

    // 3. 创建 LLVMContext
    auto Context = std::make_unique<LLVMContext>();

    // 4. 创建模块
    auto M = std::make_unique<Module>("jit_module", *Context);
    M->setDataLayout(JIT->getDataLayout());

    // 5. 函数：int add(int a, int b)
    auto *Int32Ty = Type::getInt32Ty(*Context);
    FunctionType *FT = FunctionType::get(Int32Ty, {Int32Ty, Int32Ty}, false);
    Function *AddFunc = Function::Create(FT, Function::ExternalLinkage, "add", M.get());

    // 6. 基本块
    BasicBlock *BB = BasicBlock::Create(*Context, "EntryBlock", AddFunc);
    IRBuilder<> Builder(BB);
    Value *A = AddFunc->arg_begin();
    Value *B = std::next(AddFunc->arg_begin());
    Value *Sum = Builder.CreateAdd(A, B, "sum");
    Builder.CreateRet(Sum);

    // 7. 打印 IR
    M->print(errs(), nullptr);

    // 8. 线程安全上下文（Context 移入）
    ThreadSafeContext TSC(std::move(Context));  // Context 移入 TSC
    cantFail(JIT->addIRModule(ThreadSafeModule(std::move(M), TSC)));

    // 9. 查找符号（LLVM 14 与 15+ 的返回类型不同，按版本选择写法）
    using AddFunction = int(*)(int, int);

    auto AddSym = cantFail(JIT->lookup("add"));
    // LLVM 14: lookup 返回 JITEvaluatedSymbol，需把 JITTargetAddress 转成函数指针；
    // LLVM 15+: lookup 返回 ExecutorAddr，可直接 toPtr<T>()。
#if LLVM_VERSION_MAJOR >= 15
    auto *add = AddSym.toPtr<AddFunction>();
#else
    auto *add = jitTargetAddressToFunction<AddFunction>(AddSym.getAddress());
#endif

    std::cout << "3 + 4 = " << add(3, 4) << std::endl;

    return 0;
}