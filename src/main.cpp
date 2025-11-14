#include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include <iostream>

using namespace llvm;
using namespace llvm::orc;

int main() {
    // 初始化本地目标机
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();

    // 创建 LLJIT（现代推荐方式）
    auto JIT = ExitOnErr(LLJITBuilder().create());
    auto &JD = JIT->getMainJITDylib();

    // 创建一个简单的模块：int add(int a, int b) { return a + b; }
    LLVMContext Context;
    auto M = std::make_unique<Module>("jit_module", Context);
    M->setDataLayout(JIT->getDataLayout());

    // 定义函数类型：int (int, int)
    FunctionType *FT = FunctionType::get(Type::getInt32Ty(Context),
                                         {Type::getInt32Ty(Context), Type::getInt32Ty(Context)}, false);

    // 创建函数
    Function *AddFunc = Function::Create(FT, Function::ExternalLinkage, "add", M.get());

    // 创建基本块
    BasicBlock *BB = BasicBlock::Create(Context, "EntryBlock", AddFunc);
    IRBuilder<> Builder(BB);
    Value *A = AddFunc->arg_begin();
    Value *B = std::next(AddFunc->arg_begin());
    Value *Sum = Builder.CreateAdd(A, B, "sum");
    Builder.CreateRet(Sum);

    // 打印 IR
    M->print(errs(), nullptr);

    // 添加模块到 JIT
    ExitOnErr(JIT->addIRModule(ThreadSafeModule(std::move(M), Context)));

    // 查找符号
    auto AddSym = ExitOnErr(JIT->lookup("add"));
    int (*add)(int, int) = AddSym.getAddress().toPtr<int(int, int)>();

    // 调用 JIT 编译的函数
    std::cout << "3 + 4 = " << add(3, 4) << std::endl;

    return 0;
}