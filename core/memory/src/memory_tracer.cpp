#include "memory_tracer.hpp"
#include <sstream>
#include <iomanip>

namespace pynovage {
namespace memory {

std::string getSymbolNameFromAddress(void* addr) {
#ifdef _WIN32
    char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
    PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)buffer;
    pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    pSymbol->MaxNameLen = MAX_SYM_NAME;

    DWORD64 displacement = 0;
    if (SymFromAddr(GetCurrentProcess(), (DWORD64)addr, &displacement, pSymbol)) {
        std::stringstream ss;
        ss << pSymbol->Name << "+0x" << std::hex << displacement;
        return ss.str();
    }
    return "unknown";
#else
    void* stack[1] = { addr };
    char** symbols = backtrace_symbols(stack, 1);
    if (symbols != nullptr) {
        std::string result = symbols[0];
        free(symbols);
        return result;
    }
    return "unknown";
#endif
}

class MemoryTracerUtils {
public:
    static std::string formatStackTrace(const std::vector<void*>& trace) {
        std::stringstream ss;
        for (std::size_t i = 0; i < trace.size(); ++i) {
            ss << "#" << i << ": " << getSymbolNameFromAddress(trace[i]) << "\n";
        }
        return ss.str();
    }

    static std::string formatAllocationEvent(const AllocationEvent& event) {
        std::stringstream ss;
        ss << "Address: " << event.address
           << "\nSize: " << event.size << " bytes"
           << "\nThread: " << event.thread_name
           << "\nAccess Count: " << event.access_count
           << "\nHot: " << (event.is_hot ? "Yes" : "No")
           << "\nStack Trace:\n" << formatStackTrace(event.stack_trace);
        return ss.str();
    }

    static std::string generateAccessHeatmap(const AccessTracker& tracker, void* start, std::size_t size) {
        std::stringstream ss;
        ss << "Memory Heat Map for range [" << start << ", " << (void*)((char*)start + size) << "]\n";
        
        const int WIDTH = 80;  // characters per line
        const std::size_t BLOCK_SIZE = size / WIDTH;
        
        for (std::size_t i = 0; i < WIDTH; ++i) {
            void* block_start = (char*)start + (i * BLOCK_SIZE);
            bool is_hot = tracker.isHot(block_start);
            ss << (is_hot ? "█" : ".");
        }
        
        return ss.str();
    }
};

} // namespace memory
} // namespace pynovage