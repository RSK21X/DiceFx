// macOS-only test instrumentation. dyld interposes callers in other images;
// these wrappers' own malloc calls still resolve to the original allocator.
#include <cstdlib>
#include <cstdint>

namespace
{
thread_local bool tracking = false;
thread_local std::uint64_t allocations = 0;
void* auditMalloc (std::size_t n) { if (tracking) ++allocations; return std::malloc (n); }
void* auditCalloc (std::size_t n, std::size_t size) { if (tracking) ++allocations; return std::calloc (n, size); }
void* auditRealloc (void* p, std::size_t n) { if (tracking) ++allocations; return std::realloc (p, n); }
struct Interpose { const void* replacement; const void* original; };
__attribute__((used)) const Interpose interposers[] __attribute__((section("__DATA,__interpose"))) {
    { reinterpret_cast<const void*> (&auditMalloc), reinterpret_cast<const void*> (&std::malloc) },
    { reinterpret_cast<const void*> (&auditCalloc), reinterpret_cast<const void*> (&std::calloc) },
    { reinterpret_cast<const void*> (&auditRealloc), reinterpret_cast<const void*> (&std::realloc) }
};
}
extern "C" void diceAuditBegin() { allocations = 0; tracking = true; }
extern "C" std::uint64_t diceAuditEnd() { tracking = false; return allocations; }
