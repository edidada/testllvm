#include "llvm/ADT/StringRef.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Error.h"
#include <system_error>

using namespace llvm;

int main() {
    // 示例1: 使用StringRef处理字符串
    StringRef str = "Hello, LLVM 17!";
    errs() << "String: " << str << "\n";
    errs() << "Length: " << str.size() << "\n";
    errs() << "Contains 'LLVM': " << str.contains("LLVM") << "\n";

    // 示例2: 从文件创建MemoryBuffer
    auto bufferOrError = MemoryBuffer::getFile("example.txt");
    if (auto err = bufferOrError.getError()) {
        errs() << "Error reading file: " << err.message() << "\n";
        return 1;
    }

    std::unique_ptr<MemoryBuffer> buffer = std::move(*bufferOrError);
    StringRef content = buffer->getBuffer();

    errs() << "File size: " << content.size() << " bytes\n";
    errs() << "First 50 chars: " << content.substr(0, 50) << "\n";

    return 0;
}