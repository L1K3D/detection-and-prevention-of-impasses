# Detection and Prevention of Deadlocks in C

**A Comprehensive Study on Concurrent Programming with POSIX Threads**

## Overview

This project is a rigorous academic and practical study on the detection and prevention of deadlocks (impasses) in concurrent C programs using POSIX Threads (pthreads). It encompasses ten detailed case studies that demonstrate common pitfalls in multithreaded programming, their root causes, and systematic solutions based on **Coffman's Conditions** and **global lock ordering** principles.

## Project Structure

```
detection-and-prevention-of-impasses/
├── README.md                                    # Main documentation
├── reference-c-scripts/                         # Original problematic code examples
│   ├── rcs-bank-transfers-with-reverse-order.c
│   ├── rcs-three-threads-and-complete-cicle.c
│   ├── rcs-auxiliar-function-wich-hides-risk.c
│   ├── rcs-hide-deadlock-bypass-model.c
│   ├── rcs-multiple-resources-with-dependence-cicle.c
│   ├── rcs-printer-and-spooner-with-incorrect-model.c
│   ├── rcs-production-maintence-and-auxiliar-function.c
│   ├── rcs-resources-getted-in-recursive-function.c
│   ├── rcs-trylock-and-livelock-risk.c
│   └── rcs-parcial-timeout-and-inconsistent-solution.c
├── corrected-c-scripts/                         # Fixed implementations
│   ├── ccs-bank-transfers-with-reverse-order.c
│   ├── ccs-three-threads-and-complete-cicle.c
│   ├── ccs-auxiliar-function-wich-hides-risk.c
│   ├── ccs-hide-deadlock-bypass-model.c
│   ├── ccs-multiple-resources-with-dependence-cicle.c
│   ├── ccs-printer-and-spooner-with-incorrect-model.c
│   ├── ccs-production-maintence-and-auxiliar-function.c
│   ├── ccs-resources-getted-in-recursive-function.c
│   ├── ccs-trylock-and-livelock-risk.c
│   └── css-parcial-timeout-and-inconsistent-solution.c
└── doc/                                         # Documentation

```

## Core Concepts: Coffman's Conditions

A deadlock occurs when **all four conditions** are simultaneously present:

1. **Mutual Exclusion**: Resources are protected by mutexes, allowing only one thread exclusive access
2. **Hold and Wait**: A thread holds a resource while waiting for another resource
3. **No Preemption**: The operating system cannot forcibly revoke resources from threads
4. **Circular Wait**: A cyclic chain of dependencies exists where each thread waits for a resource held by another

**Key Strategy**: Breaking any one of these conditions prevents deadlock. This project demonstrates that breaking circular wait through **global lock ordering** is the most robust and practical solution.

## Case Studies

### Case 1: Bank Transfers with Reverse Lock Order
**Problem**: Two threads acquiring accounts (resources) in opposite orders, creating a circular dependency.

**Root Cause**: Threads lock `account1` → `account2` and `account2` → `account1` respectively.

**Solution**: Enforce a global ordering where all threads must acquire `account1` **before** `account2`.

### Case 2: Three Threads with Complete Circular Dependency
**Problem**: Three threads forming a complete circular dependency: T1 holds R1 and waits for R2; T2 holds R2 and waits for R3; T3 holds R3 and waits for R1.

**Root Cause**: No consistent lock ordering across threads.

**Solution**: Establish strict hierarchical lock ordering (R1 → R2 → R3) for all threads.

### Case 3: Auxiliary Function Hiding Risk
**Problem**: Self-deadlock caused by nested function calls that indirectly require the same mutex.

**Root Cause**: Thread1 locks `log_mutex`, then calls `atualizar_banco()`, which calls `registrar_log()` attempting to lock `log_mutex` again (non-recursive mutex).

**Solution**: Release the primary lock **before** invoking auxiliary functions that may require additional locks.

### Case 4: Trylock and Livelock Risk
**Problem**: Using `pthread_mutex_trylock()` with symmetric retry patterns causes livelock instead of deadlock.

**Root Cause**: Both threads repeatedly acquire-release-retry in perfect synchronization without making progress.

**Solution**: Replace non-blocking polling with deterministic global lock ordering.

### Case 5: Resources Acquired in Recursive Functions
**Problem**: Recursive lock acquisition combined with inter-thread circular waiting.

**Root Cause**: Recursive function holds mutex A while making nested calls; another thread locks B and waits for A.

**Solution**: Use recursive mutexes (`PTHREAD_MUTEX_RECURSIVE`) for re-entrant code and maintain global lock ordering across threads.

### Case 6: Producer, Maintenance, and Auxiliary Functions
**Problem**: Hidden deadlock due to auxiliary function calls while holding the primary lock.

**Root Cause**: Producer holds `fila_mutex` while calling `atualizar_estatisticas()`, which requires `estat_mutex`; maintenance thread holds `estat_mutex` and waits for `fila_mutex`.

**Solution**: Release primary resources **before** calling independent auxiliary functions (early release strategy).

### Case 7: Printer and Spooler with Incorrect Model
**Problem**: Direct resource contention violating the spooling architecture paradigm.

**Root Cause**: User threads directly compete for both printer and spooler locks.

**Solution**: Implement proper spooler architecture where users only lock the spooler queue, and a dedicated daemon thread manages direct hardware access.

### Case 8: Deadlock Hidden by Module Abstraction
**Problem**: Encapsulation obscures the true lock ordering dependency across module boundaries.

**Root Cause**: `atualizar_cache()` locks cache then disk; `flush_disco_para_cache()` locks disk then cache (inverse order).

**Solution**: Document and enforce consistent lock ordering across all module interfaces and callers.

### Case 9: Partial Timeout and Inconsistent Solution
**Problem**: Asymmetric timeout-based solution leads to task abandonment (starvation) instead of deadlock prevention.

**Root Cause**: Thread uses `trylock` and abandons work on failure, creating incomplete operations.

**Solution**: Replace asymmetric timeout patterns with symmetric global lock ordering for all threads.

### Case 10: Multiple Resources with Dependency Cycles
**Problem**: Complex scenarios with multiple partial circular dependencies.

**Root Cause**: Three threads with multiple resources creating interconnected circular wait conditions.

**Solution**: Enforce strict alphabetical (or ID-based) lock ordering: A before B before C for all threads.

## Universal Prevention Strategy

**The Golden Rule: Global Lock Ordering**

Establish a **total order** over all locks in the system. Ensure that every thread acquires locks **strictly** in this order:

```c
// Global Lock Order: mutex_a < mutex_b < mutex_c

// Thread 1
pthread_mutex_lock(&mutex_a);
pthread_mutex_lock(&mutex_b);
pthread_mutex_lock(&mutex_c);
// ... critical section ...
pthread_mutex_unlock(&mutex_c);
pthread_mutex_unlock(&mutex_b);
pthread_mutex_unlock(&mutex_a);

// Thread 2 - MUST follow the same order
pthread_mutex_lock(&mutex_a);
pthread_mutex_lock(&mutex_b);
// ... critical section ...
pthread_mutex_unlock(&mutex_b);
pthread_mutex_unlock(&mutex_a);

// Thread 3 - MUST follow the same order
pthread_mutex_lock(&mutex_a);
pthread_mutex_lock(&mutex_c);
// ... critical section ...
pthread_mutex_unlock(&mutex_c);
pthread_mutex_unlock(&mutex_a);
```

**Invariants**:
- Always unlock in **reverse order** of acquisition
- Never hold a lock while invoking external functions that may require other locks
- For recursive functions, use `PTHREAD_MUTEX_RECURSIVE` attributes
- Document lock dependencies clearly

## Compilation and Execution

### Prerequisites
- GCC or Clang C compiler
- POSIX-compliant system (Linux, macOS, or Windows with WSL)
- `pthread` library

### Compile a Corrected Example
```bash
gcc -o ccs-bank-transfers ccs-bank-transfers-with-reverse-order.c -lpthread
./ccs-bank-transfers
```

### Compile All Examples
```bash
cd corrected-c-scripts/
for file in ccs-*.c css-*.c; do
    gcc -o "${file%.c}" "$file" -lpthread
    echo "Compiled: $file"
done
```

### Run All Executables
```bash
for exe in ccs-* css-*; do
    [ -x "$exe" ] && echo "=== Running $exe ===" && ./"$exe"
done
```

## Key Takeaways and Best Practices

### ✓ DO
- Establish a **clear, documented lock hierarchy** at design time
- Always acquire locks in the **same order** across all threads
- Always release locks in **reverse order** of acquisition
- Use **recursive mutexes** when lock re-entrancy is necessary
- **Release locks early** before invoking auxiliary functions
- Document lock dependencies and protected resources
- Use static analysis tools to detect potential deadlock patterns
- Test concurrent code under high contention scenarios

### ✗ DON'T
- Mix lock acquisition orders across different threads
- Hold locks across external function calls (unless documented)
- Rely on `pthread_mutex_trylock()` for deadlock prevention (livelock risk)
- Assume lock ordering will be enforced implicitly or by the OS
- Encapsulate lock operations without documenting their impact
- Use timeout-based solutions as the primary deadlock prevention strategy
- Ignore Coffman's Conditions in system design

## Interdisciplinary Concepts

This project demonstrates fundamental principles applicable beyond C/pthreads:
- **Database transaction management**: Lock ordering prevents transaction deadlocks
- **Distributed systems**: Deadlock detection and prevention in resource allocation
- **Resource scheduling**: Cycle detection in dependency graphs
- **Operating systems**: Process synchronization and scheduling
- **Graph theory**: Detecting cycles in wait-for dependency graphs

## Conclusion

Concurrent programming using POSIX Threads requires rigorous architectural discipline. **Deadlock-freedom by design** is achieved through a combination of:

1. **Strict global lock ordering** (most effective)
2. **Early resource release** before auxiliary function calls
3. **Proper abstraction** of synchronization primitives
4. **Recursive mutex support** for re-entrant code
5. **Comprehensive documentation** of lock dependencies

The ten case studies presented demonstrate that seemingly disparate concurrent problems (deadlock, self-deadlock, livelock, starvation) often share common root causes. By applying these universal prevention strategies, developers can build robust, deterministic concurrent systems that are provably immune to circular wait conditions.

## Academic Context

**Institution**: CEFSA – Centro Educacional da Fundação Salvador Arena  
**University**: FESA – Faculdade Engenheiro Salvador Arena  
**Program**: Computer Engineering, 8th Semester (EC8)  
**Course**: Operating Systems  
**Assignment**: N2 B2 Part 02 – Detection and Prevention of Deadlocks in C  
**Date**: May 27, 2026  

**Authors**:
- Enzo Brito Alves de Oliveira (RA: 082220040)
- Erikson Vieira Queiroz (RA: 082220021)
- Heitor Santos Ferreira (RA: 081230042)
- William Santim (RA: 082220033)

**Instructor**: Prof. Dr. Vinicius Borges

## References

- POSIX Threads Programming: https://computing.llnl.gov/tutorials/pthreads/
- Coffman Conditions for Deadlock: https://en.wikipedia.org/wiki/Deadlock#Necessary_and_sufficient_conditions
- Linux man pages: `pthread_mutex_lock(3)`, `pthread_mutex_init(3)`
- Operating Systems Concepts (Silberschatz, Galvin, Gagne)

## License

This educational project is provided as-is for academic and learning purposes.

---

**Last Updated**: May 28, 2026