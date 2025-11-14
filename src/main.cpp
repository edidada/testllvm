// main.cpp - LLVM 18 完全兼容版
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
    InitLLVM X(argc, argv);
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();

    // 创建 LLJIT
    auto JIT = cantFail(LLJITBuilder().create());
    auto &JD = JIT->getMainJITDylib();

    // 创建模块
    LLVMContext Context;
    auto M = std::make_unique<Module>("jit_module", Context);
    M->setDataLayout(JIT->getDataLayout());

    // 函数类型：int add(int, int)
    auto *Int32Ty = Type::getInt32Ty(Context);
    FunctionType *FT = FunctionType::get(Int32Ty, {Int32Ty, Int32Ty}, false);
    Function *AddFunc = Function::Create(FT, Function::ExternalLinkage, "add", M.get());

    // 基本块
    BasicBlock *BB = BasicBlock::Create(Context, "EntryBlock", AddFunc);
    IRBuilder<> Builder(BB);
    Value *A = AddFunc->arg_begin();
    Value *B = std::next(AddFunc->arg_begin());
    Value *Sum = Builder.CreateAdd(A, B, "sum");
    Builder.CreateRet(Sum);

    // 打印 IR
    M->print(errs(), nullptr);

    // 关键修复：使用 ThreadSafeContext
    ThreadSafeContext TSC(std::make_unique<LLVMContext>(std::move(Context)));
    cantFail(JIT->addIRModule(ThreadSafeModule(std::move(M), TSC)));

    // 查找符号
    auto AddSym = cantFail(JIT->lookup("add"));
    int (*add)(int, int) = AddSym.getAddress().toPtr<int(*)(int, int)>();

    // 调用
    std::cout << "3 + 4 = " << add(3, 4) << std::endl;

    return 0;
}