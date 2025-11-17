# README
llvm18可以，llvm14不行的
```shell
/mnt/d/develops/git/github/cpp/testllvm/cmake-build-debug-wsl24/testllvm
; ModuleID = 'jit_module'
source_filename = "jit_module"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"

define i32 @add(i32 %0, i32 %1) {
EntryBlock:
%sum = add i32 %0, %1
ret i32 %sum
}
3 + 4 = 7
```
Process finished with exit code 0

/mnt/d/develops/git/github/cpp/testllvm/cmake-build-debug-wsl24/testpass2
=== Before Optimization ===
Function: complex_calc
Basic Block: entry:
%2 = add i32 %0, 0
%3 = mul i32 %2, 1
%4 = sub i32 %1, 0
%5 = mul i32 %4, 1
%6 = add i32 %3, %5
%7 = icmp eq i32 %0, %0
br i1 %7, label %then, label %else
Basic Block: then:
%8 = add i32 %6, 0
br label %merge
Basic Block: else:
%9 = sub i32 %6, 0
br label %merge
Basic Block: merge:
%10 = phi i32 [ %8, %then ], [ %9, %else ]
ret i32 %10

=== After Optimization ===
Function: complex_calc
Basic Block: entry:
%2 = add i32 %1, %0
ret i32 %2

All optimizations completed successfully!