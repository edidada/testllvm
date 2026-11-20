#include "llvm/ADT/StringRef.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/FileSystem.h"
#include <memory>
#include <tuple>
#include <vector>

using namespace llvm;

// 处理文件内容的函数
void processFileContent(StringRef content) {
    errs() << "=== Processing File Content ===\n";

    // 使用StringRef的方法
    errs() << "Total lines: " << content.count('\n') + 1 << "\n";
    errs() << "First line: " << content.split('\n').first << "\n";

    // 查找特定字符串
    size_t pos = content.find("LLVM");
    if (pos != StringRef::npos) {
        errs() << "Found 'LLVM' at position: " << pos << "\n";
    }

    // 按行处理
    StringRef remaining = content;
    while (!remaining.empty()) {
        StringRef line;
        std::tie(line, remaining) = remaining.split('\n');
        if (line.contains("error") || line.contains("Error")) {
            errs() << "Error line: " << line.trim() << "\n";
        }
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        errs() << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }

    StringRef filename = argv[1];

    // MemoryBuffer::getFile 在所有 LLVM 14+ 版本都返回 ErrorOr
    auto bufferOrError = MemoryBuffer::getFile(filename);
    if (!bufferOrError) {
        errs() << "Error opening file: " << filename << "\n";
        return 1;
    }

    std::unique_ptr<MemoryBuffer> buffer = std::move(bufferOrError.get());
    processFileContent(buffer->getBuffer());

    return 0;
}