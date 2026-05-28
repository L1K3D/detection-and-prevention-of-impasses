# Quick Reference Guide & Project Index

## Navigation Guide

This document provides quick access to all information about the Detection and Prevention of Deadlocks project.

---

## Core Documents

### Document 1: [01_UNDERSTANDING_DEADLOCKS.md](01_UNDERSTANDING_DEADLOCKS.md)
**Best For**: Learning the foundational concepts

**Contains**:
- What is concurrent programming?
- What is a deadlock? (with real-world analogies)
- Coffman's 4 Conditions (detailed explanations with code examples)
- Why deadlocks occur (timing, abstraction, nested calls)
- Prevention strategies (5 major approaches)

**Key Sections**:
- Lines: Introduction to Concurrent Programming
- Lines: Coffman's Conditions Explained (Conditions 1-4)
- Lines: Prevention Strategies

**When to Read**: First - before studying specific cases

---

### Document 2: [02_DETAILED_CASE_ANALYSIS.md](02_DETAILED_CASE_ANALYSIS.md)
**Best For**: Understanding the first 4 cases in depth

**Contains**:
- **Case 1**: Bank Transfers with Reverse Lock Order
  - Problem, timeline, analysis, corrected code
- **Case 2**: Three Threads with Complete Circular Dependency
  - Circular chain visualization, solution walkthrough
- **Case 3**: Auxiliary Function Hiding Risk
  - Self-deadlock pattern, two solutions (early release + recursive mutex)
- **Case 4**: Trylock and Livelock Risk
  - Livelock vs. deadlock distinction, synchronization issue
- Summary table for Cases 5-10
- Universal principle (global lock ordering)

**When to Read**: Second - for concrete examples of prevention

---

### Document 3: [03_CODE_IMPLEMENTATION_GUIDE.md](03_CODE_IMPLEMENTATION_GUIDE.md)
**Best For**: Understanding how to read and write concurrent C code

**Contains**:
- Code structure overview (typical corrected implementation pattern)
- Global lock ordering pattern recognition
- Multi-thread synchronization examples
- Recursive function handling
- Common pitfalls (5 major mistakes)
- Code review checklist
- Testing strategy
- Performance implications

**Key Checklist**:
- All mutexes initialized before thread usage
- Lock ordering identical across all threads
- Locks released in reverse order
- No lock held during external function calls
- No early returns without unlocking

**When to Read**: While or after studying case codes

---

### Document 4: [04_COMPLETE_CASE_STUDIES.md](04_COMPLETE_CASE_STUDIES.md)
**Best For**: Reference guide for all 10 cases

**Contains**:
- **Cases 1-3**: Full explanations with code
- **Case 1**: Bank Transfers - reverse order problem
- **Case 2**: Three Threads - complete circular dependency
- **Case 3**: Auxiliary Function - hidden risk
- **Cases 4-10**: Summary table with key information
- Universal principle recap
- Compilation and testing instructions

**File References**:
- Each case links to both problematic and corrected C files
- Shows exact location: `reference-c-scripts/` vs. `corrected-c-scripts/`

**When to Read**: For complete reference on all cases

---

### Document 5: [05_PRACTICAL_LEARNING_GUIDE.md](05_PRACTICAL_LEARNING_GUIDE.md)
**Best For**: Self-study and hands-on learning

**Contains**:
- Week-by-week self-study plan
- Exercise 1: Detect the bug
- Exercise 2: Fix it yourself
- Exercise 3: Trace execution timeline
- Exercise 4: Modify and break it
- Comparative analysis framework
- Advanced topics (POSIX mutex attributes)
- Teaching others (presentation outline)
- Assessment checklist
- Further learning resources

**When to Read**: For structured learning plan and hands-on practice

---

## The 10 Case Studies at a Glance

### Quick Lookup Table

| # | Name | Problematic File | Fixed File | Main Issue | Coffman Focus |
|---|------|------------------|------------|-----------|---------------|
| 1 | Bank Transfers | `rcs-bank-transfers-with-reverse-order.c` | `ccs-bank-transfers-with-reverse-order.c` | Reverse lock ordering | Circular Wait |
| 2 | Three Threads | `rcs-three-threads-and-complete-cicle.c` | `ccs-three-threads-and-complete-cicle.c` | 3-thread cycle | Circular Wait |
| 3 | Auxiliary Func | `rcs-auxiliar-function-wich-hides-risk.c` | `ccs-auxiliar-function-wich-hides-risk.c` | Self-deadlock/hidden dependency | Mutual Exclusion |
| 4 | Trylock Livelock | `rcs-trylock-and-livelock-risk.c` | `ccs-trylock-and-livelock-risk.c` | Synchronized retry | Circular Wait |
| 5 | Recursive Func | `rcs-resources-getted-in-recursive-function.c` | `ccs-resources-getted-in-recursive-function.c` | Recursive + circular | Mutual Exclusion |
| 6 | Producer Maint | `rcs-production-maintence-and-auxiliar-function.c` | `ccs-production-maintence-and-auxiliar-function.c` | Hold-and-wait | Hold and Wait |
| 7 | Printer Spooler | `rcs-printer-and-spooner-with-incorrect-model.c` | `ccs-printer-and-spooner-with-incorrect-model.c` | Direct device access | Mutual Exclusion |
| 8 | Hidden Deadlock | `rcs-hide-deadlock-bypass-model.c` | `ccs-hide-deadlock-bypass-model.c` | Module abstraction | Circular Wait |
| 9 | Timeout Inconsistent | `rcs-parcial-timeout-and-inconsistent-solution.c` | `css-parcial-timeout-and-inconsistent-solution.c` | Asymmetric timeout | Circular Wait |
| 10 | Multiple Resources | `rcs-multiple-resources-with-dependence-cicle.c` | `ccs-multiple-resources-with-dependence-cicle.c` | Multiple partial cycles | Circular Wait |

---

## Coffman's Conditions Reference

### The 4 Conditions (All Must Be Present for Deadlock)

**1. Mutual Exclusion**
- Definition: Only one thread can hold a resource
- POSIX Implementation: `pthread_mutex_lock/unlock()`
- Prevention: Allow shared read access (read-write locks) - rarely used
- Case Examples: All 10 cases

**2. Hold and Wait**
- Definition: Thread holds one resource while waiting for another
- Pattern: `lock(A); wait for lock(B);`
- Prevention: Release before nested call OR acquire all upfront
- Case Examples: Cases 1, 2, 4, 5, 6, 8, 9, 10

**3. No Preemption**
- Definition: OS cannot forcibly revoke a held resource
- Implementation: Mutex design principle (prevents data corruption)
- Prevention: Rare to violate due to correctness
- Case Examples: All 10 cases (fundamental to mutex design)

**4. Circular Wait**
- Definition: Resource allocation forms a cycle
- Pattern: T1→R2→T2→R3→T3→R1→T1
- **Prevention: GLOBAL LOCK ORDERING** (solves all 10 cases)
- Case Examples: Cases 1, 2, 4, 8, 9, 10 (explicit); others (implicit)

---

## Prevention Strategies Summary

### Strategy 1: Global Lock Ordering (RECOMMENDED)
**Principle**: All threads acquire locks in identical order
```c
// Global order: Lock_A < Lock_B < Lock_C
// ALL threads:
pthread_mutex_lock(&Lock_A);
pthread_mutex_lock(&Lock_B);
pthread_mutex_lock(&Lock_C);
// Work...
pthread_mutex_unlock(&Lock_C);  // Reverse order
pthread_mutex_unlock(&Lock_B);
pthread_mutex_unlock(&Lock_A);
```
**Breaks**: Circular Wait (Condition 4)
**Used In**: All 10 cases
**Effectiveness**: 100% prevents deadlock

### Strategy 2: Early Release
**Principle**: Release lock before calling functions
```c
pthread_mutex_lock(&lock);
do_work();
pthread_mutex_unlock(&lock);     // Release BEFORE
call_external_function();        // Now safe
```
**Breaks**: Hold and Wait (Condition 2)
**Used In**: Cases 3, 6, 7
**Limitation**: Only works for non-dependent operations

### Strategy 3: Recursive Mutexes
**Principle**: Allow same thread to lock multiple times
```c
pthread_mutexattr_t attr;
pthread_mutexattr_init(&attr);
pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
pthread_mutex_init(&lock, &attr);
```
**Breaks**: Mutual Exclusion (Condition 1) for single-thread
**Used In**: Cases 3, 5
**Limitation**: Only solves intra-thread deadlock

### Strategy 4: Timeout Locking (NOT RECOMMENDED)
**Principle**: Use time limits on lock acquisition
```c
struct timespec timeout;
clock_gettime(CLOCK_REALTIME, &timeout);
timeout.tv_sec += 1;  // 1 second timeout
pthread_mutex_timedlock(&lock, &timeout);
```
**Breaks**: No Preemption (Condition 3) - approximates forcing release
**Problems**: Causes starvation, livelock, unpredictable behavior
**Verdict**: Palliative, not curative

### Strategy 5: Spooler Architecture
**Principle**: Centralize resource access through daemon
```c
// Users: only interact with queue
pthread_mutex_lock(&queue);
enqueue_job();
pthread_mutex_unlock(&queue);

// Daemon: exclusive device access
for each job in queue:
    pthread_mutex_lock(&device);
    execute_job();
    pthread_mutex_unlock(&device);
```
**Breaks**: Circular Wait (Condition 4) via architectural separation
**Used In**: Case 7
**When Applicable**: Hardware device management

---

## Key Code Patterns

### Pattern 1: Simple Two-Lock Ordering

```c
pthread_mutex_t lock_a, lock_b;

void *thread_func(void *arg) {
    pthread_mutex_lock(&lock_a);        // 1st
    // Use lock_a
    pthread_mutex_lock(&lock_b);        // 2nd
    // Use both
    pthread_mutex_unlock(&lock_b);      // Reverse order
    pthread_mutex_unlock(&lock_a);
    return NULL;
}
```

**All threads must use this identical order.**

---

### Pattern 2: N-Lock Ordering

```c
pthread_mutex_t locks[N];

void *thread_func(void *arg) {
    // Acquire in order: 0, 1, 2, ..., N-1
    for (int i = 0; i < N; i++) {
        pthread_mutex_lock(&locks[i]);
    }
    
    // Critical section
    
    // Release in reverse: N-1, N-2, ..., 1, 0
    for (int i = N-1; i >= 0; i--) {
        pthread_mutex_unlock(&locks[i]);
    }
    return NULL;
}
```

**Scalable pattern for any number of locks.**

---

### Pattern 3: Recursive Mutex

```c
pthread_mutexattr_t attr;
pthread_mutexattr_init(&attr);
pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
pthread_mutex_t recursive_lock;
pthread_mutex_init(&recursive_lock, &attr);

void recursive_function(int depth) {
    pthread_mutex_lock(&recursive_lock);
    
    if (depth > 0) {
        recursive_function(depth - 1);      // Same thread can lock again
    }
    
    pthread_mutex_unlock(&recursive_lock);
}
```

**Allows re-entrancy for single-thread recursion.**

---

### Pattern 4: Early Release

```c
void *thread_func(void *arg) {
    pthread_mutex_lock(&primary_lock);
    // Do critical work
    pthread_mutex_unlock(&primary_lock);    // Release BEFORE nested call
    
    external_function_that_needs_other_locks();
    
    return NULL;
}
```

**Minimizes lock hold time and dependencies.**

---

## Compilation Quick Reference

### Compile Reference (Problematic) Version
```bash
cd reference-c-scripts
gcc rcs-bank-transfers-with-reverse-order.c -lpthread -o test_problem
timeout 5 ./test_problem    # Should hang or timeout
```

### Compile Corrected Version
```bash
cd corrected-c-scripts
gcc ccs-bank-transfers-with-reverse-order.c -lpthread -o test_fixed
./test_fixed                # Should complete normally
```

### Compile All Reference Versions
```bash
cd reference-c-scripts
for f in rcs-*.c; do
    gcc "$f" -lpthread -o "${f%.c}"
done
```

### Stress Test (Run Multiple Times)
```bash
cd corrected-c-scripts
for i in {1..100}; do
    timeout 2 ./ccs-bank-transfers-with-reverse-order
    if [ $? -eq 124 ]; then
        echo "DEADLOCK on run $i"
        break
    fi
done
```

---

## Testing Strategies

### Quick Sanity Check
```bash
timeout 5 ./program
if [ $? -eq 0 ]; then
    echo "✓ No obvious deadlock"
else
    echo "✗ Deadlock or timeout detected"
fi
```

### Detect Intermittent Deadlock
```bash
DEADLOCK_COUNT=0
for i in {1..50}; do
    timeout 3 ./program > /dev/null 2>&1
    if [ $? -eq 124 ]; then
        DEADLOCK_COUNT=$((DEADLOCK_COUNT+1))
    fi
done
echo "Deadlocks: $DEADLOCK_COUNT out of 50"
```

### Trace with Tools
```bash
# ThreadSanitizer (if available)
gcc -fsanitize=thread -g program.c -lpthread
./a.out

# Helgrind (if Valgrind installed)
valgrind --tool=helgrind ./program
```

---

## Debugging Deadlock

### Signs of Deadlock
- Program hangs indefinitely
- CPU usage drops to ~0% (threads blocked)
- No output after certain point
- Timeout triggers consistently at same point

### Debugging Steps

1. **Add Debug Output**
```c
fprintf(stderr, "T%d: Attempting lock(%s)\n", thread_id, lock_name);
pthread_mutex_lock(&lock);
fprintf(stderr, "T%d: Acquired lock(%s)\n", thread_id, lock_name);
```

2. **Reduce Complexity**
- Start with single thread
- Add second thread
- Add third thread
- Gradually increase complexity

3. **Add Timing Information**
```c
struct timespec start, now;
clock_gettime(CLOCK_MONOTONIC, &start);

pthread_mutex_lock(&lock);
clock_gettime(CLOCK_MONOTONIC, &now);
printf("Lock acquired in %ld ns\n", 
       (now.tv_sec - start.tv_sec)*1e9 + (now.tv_nsec - start.tv_nsec));
```

4. **Verify Lock Ordering**
- Print which locks are acquired in which order
- Compare across all threads
- Ensure order is identical

---

## Common Questions & Answers

### Q: Why does my corrected code still sometimes deadlock?

**A**: Check for:
1. Are ALL threads following the same lock order? (Check each thread function)
2. Are there hidden locks in called functions? (Check external calls)
3. Is lock order truly total (no ambiguity)? (List all locks and their order)
4. Is there nested function that breaks the order? (Trace all call chains)

---

### Q: Can I use `pthread_mutex_trylock()` safely?

**A**: Only if you implement it correctly:
- DON'T: Use asymmetric retry logic (will cause livelock)
- DO: Either get all locks immediately OR release and retry globally
- Better: Just use blocking locks with global ordering

---

### Q: What's the difference between deadlock and starvation?

| Aspect | Deadlock | Starvation |
|--------|----------|-----------|
| **Definition** | All blocked, no progress | Some threads never get resource |
| **Visibility** | System halts completely | System keeps running |
| **Thread State** | Blocked (waiting) | Runnable (but never scheduled) |
| **Cause** | Circular dependency | Unfair scheduling |

---

### Q: Do I need to worry about deadlock in single-threaded code?

**A**: No. Deadlock requires multiple threads competing for resources. Single-threaded code cannot deadlock.

---

### Q: Is global lock ordering always the best solution?

**A**: For this project, yes. It's:
- Simple to understand
- Universally applicable
- No performance penalty
- Proven mathematically

Alternative approaches (timeouts, spooler, read-write locks) are specialized solutions for specific scenarios.

---

## File Organization

```
detection-and-prevention-of-impasses/
├── README.md                          ← Main project documentation
│
├── doc/                               ← Educational documentation
│   ├── 01_UNDERSTANDING_DEADLOCKS.md      ← Theory & concepts
│   ├── 02_DETAILED_CASE_ANALYSIS.md       ← Cases 1-4 detailed
│   ├── 03_CODE_IMPLEMENTATION_GUIDE.md    ← How to write code
│   ├── 04_COMPLETE_CASE_STUDIES.md        ← All 10 cases reference
│   ├── 05_PRACTICAL_LEARNING_GUIDE.md     ← Self-study guide
│   └── 06_QUICK_REFERENCE.md              ← This file
│
├── reference-c-scripts/               ← Problematic implementations (10 files)
│   ├── rcs-bank-transfers-with-reverse-order.c
│   ├── rcs-three-threads-and-complete-cicle.c
│   ├── ... (7 more)
│   └── output/
│
└── corrected-c-scripts/               ← Corrected implementations (10 files)
    ├── ccs-bank-transfers-with-reverse-order.c
    ├── ccs-three-threads-and-complete-cicle.c
    ├── ... (7 more)
    └── output/
```

---

## Getting Started Checklist

- [ ] Read [01_UNDERSTANDING_DEADLOCKS.md](01_UNDERSTANDING_DEADLOCKS.md) - understand concepts
- [ ] Read [02_DETAILED_CASE_ANALYSIS.md](02_DETAILED_CASE_ANALYSIS.md) - see examples
- [ ] Compile reference version: `gcc rcs-bank-transfers-with-reverse-order.c -lpthread`
- [ ] Run with timeout: `timeout 5 ./a.out` - observe deadlock
- [ ] Compile corrected version: `gcc ccs-bank-transfers-with-reverse-order.c -lpthread`
- [ ] Run corrected: `./a.out` - observe success
- [ ] Compare source files side-by-side
- [ ] Identify lock ordering differences
- [ ] Study [03_CODE_IMPLEMENTATION_GUIDE.md](03_CODE_IMPLEMENTATION_GUIDE.md)
- [ ] Complete hands-on exercises from [05_PRACTICAL_LEARNING_GUIDE.md](05_PRACTICAL_LEARNING_GUIDE.md)
- [ ] Review all 10 cases using [04_COMPLETE_CASE_STUDIES.md](04_COMPLETE_CASE_STUDIES.md)
- [ ] Implement a custom concurrent program using global lock ordering

---

## Glossary

**Deadlock**: Situation where all threads are blocked, unable to proceed.

**Livelock**: Threads execute but make no progress (busy-waiting).

**Starvation**: Some threads perpetually denied resources while others execute.

**Mutex**: Mutual exclusion lock - allows only one thread to access protected resource.

**Critical Section**: Code segment accessing shared resources, protected by locks.

**Race Condition**: Result depends unpredictably on thread scheduling.

**Circular Wait**: Cycle in resource allocation graph (A→B→C→A).

**Lock Ordering**: Total order on acquisition sequence (Lock_A < Lock_B < Lock_C).

**Recursive Mutex**: Allows same thread to lock multiple times.

**POSIX**: Portable Operating System Interface (UNIX standard).

**pthread**: POSIX thread library in C.

---

## Version Information

**Project**: Detection and Prevention of Deadlocks in C  
**Institution**: FESA - Faculdade Engenheiro Salvador Arena  
**Course**: EC8 Semester  
**Date**: May 27, 2026  
**Language**: C (POSIX Threads)  
**Platform**: Linux/Unix  
**Compiler**: GCC/Clang  
**Standard**: C99 or C11

---

## Contact & Attribution

**Course Instructor**: Dr. Vinicius Borges

**Educational Purpose**: This project serves as an in-depth case study for concurrent programming education, demonstrating common pitfalls and systematic prevention strategies.

---

**End of Quick Reference Guide**

For detailed information, refer to specific documents listed at the top of this guide.
