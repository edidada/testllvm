# README

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

