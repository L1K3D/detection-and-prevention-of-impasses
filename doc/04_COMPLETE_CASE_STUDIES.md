# The 10 Case Studies: Complete Analysis

## Overview of All 10 Scenarios

This document provides detailed explanations of each case study, broken down by:
- Problem Description
- Why it Deadlocks (or Livelocks)
- Coffman's Conditions Analysis
- Corrected Solution with Code Walkthrough
- Key Lessons

---

## Case 1: Bank Transfers with Reverse Order

**File References:**
- Problem: `rcs-bank-transfers-with-reverse-order.c`
- Solution: `ccs-bank-transfers-with-reverse-order.c`

### Problem Description

Two accounts (Conta1 and Conta2) are accessed concurrently by two threads performing transfers in opposite directions:
- Thread 1: Transfers FROM Conta1 TO Conta2
- Thread 2: Transfers FROM Conta2 TO Conta1

Each thread uses a naive approach: acquire locks in the order of the transfer direction.

### The Deadlock Scenario

```
Initial state:
  Conta1: $1000
  Conta2: $500
  Thread 1: Transfer $100 from Conta1 → Conta2
  Thread 2: Transfer $100 from Conta2 → Conta1

Execution:
  T1: lock(Conta1) ✓ holds $1000
  T1: sleep(1) for realism → CONTEXT SWITCH
  
  T2: lock(Conta2) ✓ holds $500
  T2: lock(Conta1) ✗ WAITING (T1 holds it)
  
  T1 wakes: lock(Conta2) ✗ WAITING (T2 holds it)
  
DEADLOCK: T1 waits for Conta2 (held by T2), T2 waits for Conta1 (held by T1)
```

### Coffman's Analysis

| Condition | Present? | Reason |
|-----------|----------|--------|
| **Mutual Exclusion** | ✓ Yes | Mutexes provide exclusive access |
| **Hold and Wait** | ✓ Yes | Each thread holds one account while waiting for another |
| **No Preemption** | ✓ Yes | OS cannot forcibly revoke mutex locks |
| **Circular Wait** | ✓ Yes | T1→Conta2→T2→Conta1→T1 |

**Verdict: DEADLOCK** - All four conditions present.

### The Corrected Solution

**Key Principle**: Define a global ordering: **Conta1 < Conta2**

```c
// CORRECTED VERSION
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

pthread_mutex_t conta1, conta2;
int saldo_conta1 = 1000;
int saldo_conta2 = 500;

void *transferencia_1(void *arg) {
    printf("T1: Starting transfer from Conta1 to Conta2\n");
    
    // CORRECTED: Always acquire in order: conta1 < conta2
    pthread_mutex_lock(&conta1);        // 1st (even for opposite direction)
    printf("T1: Locked Conta1\n");
    sleep(1);                           // Simulate processing
    
    pthread_mutex_lock(&conta2);        // 2nd
    printf("T1: Locked Conta2\n");
    
    // Critical section: Perform transfer
    saldo_conta1 -= 100;
    saldo_conta2 += 100;
    printf("T1: Transferred $100: Conta1=%d, Conta2=%d\n", 
           saldo_conta1, saldo_conta2);
    
    // Release in reverse order
    pthread_mutex_unlock(&conta2);
    pthread_mutex_unlock(&conta1);
    printf("T1: Released both locks\n");
    
    return NULL;
}

void *transferencia_2(void *arg) {
    printf("T2: Starting transfer from Conta2 to Conta1\n");
    
    // CORRECTED: Same order as T1, even though direction is opposite!
    pthread_mutex_lock(&conta1);        // 1st (CHANGED from conta2)
    printf("T2: Locked Conta1\n");
    sleep(1);
    
    pthread_mutex_lock(&conta2);        // 2nd (CHANGED from conta1)
    printf("T2: Locked Conta2\n");
    
    // Critical section: Perform transfer
    saldo_conta2 -= 100;
    saldo_conta1 += 100;
    printf("T2: Transferred $100: Conta1=%d, Conta2=%d\n", 
           saldo_conta1, saldo_conta2);
    
    // Release in reverse order
    pthread_mutex_unlock(&conta2);
    pthread_mutex_unlock(&conta1);
    printf("T2: Released both locks\n");
    
    return NULL;
}

int main(void) {
    pthread_t tid1, tid2;
    
    // Initialize mutexes
    pthread_mutex_init(&conta1, NULL);
    pthread_mutex_init(&conta2, NULL);
    
    // Create threads
    pthread_create(&tid1, NULL, transferencia_1, NULL);
    pthread_create(&tid2, NULL, transferencia_2, NULL);
    
    // Wait for completion
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    
    // Cleanup
    pthread_mutex_destroy(&conta1);
    pthread_mutex_destroy(&conta2);
    
    printf("Final: Conta1=%d, Conta2=%d\n", saldo_conta1, saldo_conta2);
    printf("Total balance (should be 1500): %d\n", saldo_conta1 + saldo_conta2);
    
    return 0;
}
```

### Why This Works

```
With global ordering (conta1 < conta2):

T1: lock(conta1) ✓
T1: sleep(1) → CONTEXT SWITCH

T2: lock(conta1) ✗ WAITS (T1 holds it)

T1 wakes: lock(conta2) ✓ (T2 doesn't hold it yet)
T1: Complete transfer
T1: unlock(conta2), unlock(conta1)

T2: lock(conta1) ✓ (now available)
T2: lock(conta2) ✓
T2: Complete transfer
T2: unlock(conta2), unlock(conta1)

Result: Both threads complete successfully - NO DEADLOCK
```

### Key Lesson

**The lock order must respect a global total ordering, independent of the logical flow of the operation.** Even though Thread 2 is transferring money "out" of Conta2, it must still acquire Conta1's lock first.

---

## Case 2: Three Threads with Complete Circular Dependency

**File References:**
- Problem: `rcs-three-threads-and-complete-cicle.c`
- Solution: `ccs-three-threads-and-complete-cicle.c`

### Problem Description

Three threads, three resources, each thread holds one resource and waits for another:
- Thread 1: Holds Resource1, needs Resource2
- Thread 2: Holds Resource2, needs Resource3
- Thread 3: Holds Resource3, needs Resource1

### The Deadlock Cycle

```
Resource Allocation Graph:

    Thread 1 ────── (holds) ────── Resource 1
       ▲                               ▲
       │                               │
       │ (waits for)              (wait for)
       │                               │
    Resource 2 ◄────────────── Thread 3
    
    Thread 2 ────── (holds) ────── Resource 3
       ▲
       │ (waits for)
       │
    Resource 2 ◄────────────── Thread 1

Complete Cycle: T1 → R2 → T2 → R3 → T3 → R1 → T1
```

### Problematic Code Pattern

```c
void *t1(void *arg) {
    pthread_mutex_lock(&r1);        // Lock Resource1
    printf("T1: Locked Resource1\n");
    sleep(1);
    
    pthread_mutex_lock(&r2);        // Wait for Resource2 (held by T2!)
    printf("T1: Locked Resource2\n");
    
    pthread_mutex_unlock(&r2);
    pthread_mutex_unlock(&r1);
    return NULL;
}

void *t2(void *arg) {
    pthread_mutex_lock(&r2);        // Lock Resource2
    printf("T2: Locked Resource2\n");
    sleep(1);
    
    pthread_mutex_lock(&r3);        // Wait for Resource3 (held by T3!)
    printf("T2: Locked Resource3\n");
    
    pthread_mutex_unlock(&r3);
    pthread_mutex_unlock(&r2);
    return NULL;
}

void *t3(void *arg) {
    pthread_mutex_lock(&r3);        // Lock Resource3
    printf("T3: Locked Resource3\n");
    sleep(1);
    
    pthread_mutex_lock(&r1);        // Wait for Resource1 (held by T1!) CIRCLE!
    printf("T3: Locked Resource1\n");
    
    pthread_mutex_unlock(&r1);
    pthread_mutex_unlock(&r3);
    return NULL;
}
```

### The Corrected Solution

**Key Principle**: All threads must acquire resources in order: **R1 < R2 < R3**

```c
void *t1(void *arg) {
    printf("T1: Starting\n");
    
    // CORRECTED: Respect global ordering R1 < R2 < R3
    pthread_mutex_lock(&r1);        // 1st
    printf("T1: Locked Resource1\n");
    sleep(1);
    
    pthread_mutex_lock(&r2);        // 2nd
    printf("T1: Locked Resource2\n");
    
    // Critical section
    printf("T1: Performing work...\n");
    
    pthread_mutex_unlock(&r2);
    pthread_mutex_unlock(&r1);
    printf("T1: Completed\n");
    
    return NULL;
}

void *t2(void *arg) {
    printf("T2: Starting\n");
    
    // CORRECTED: Same order as T1!
    pthread_mutex_lock(&r1);        // 1st (CHANGED from r2)
    printf("T2: Locked Resource1\n");
    sleep(1);
    
    pthread_mutex_lock(&r2);        // 2nd (CHANGED from r3)
    printf("T2: Locked Resource2\n");
    
    // Critical section
    printf("T2: Performing work...\n");
    
    pthread_mutex_unlock(&r2);
    pthread_mutex_unlock(&r1);
    printf("T2: Completed\n");
    
    return NULL;
}

void *t3(void *arg) {
    printf("T3: Starting\n");
    
    // CORRECTED: Same order as T1 and T2!
    pthread_mutex_lock(&r1);        // 1st (CHANGED from r3)
    printf("T3: Locked Resource1\n");
    sleep(1);
    
    pthread_mutex_lock(&r2);        // 2nd (CHANGED from r1)
    printf("T3: Locked Resource2\n");
    
    // Critical section
    printf("T3: Performing work...\n");
    
    pthread_mutex_unlock(&r2);
    pthread_mutex_unlock(&r1);
    printf("T3: Completed\n");
    
    return NULL;
}
```

### Why It Works

With all threads respecting R1 < R2 < R3:

```
One possible execution sequence:

T1: lock(r1) ✓
    sleep(1)
T2: lock(r1) ✗ WAITS (T1 holds it)
T3: lock(r1) ✗ WAITS (T1 holds it)

T1 wakes:
    lock(r2) ✓
    [Critical work]
    unlock(r2)
    unlock(r1)

T2 wakes:
    lock(r1) ✓
    sleep(1)
    lock(r2) ✓
    [Critical work]
    unlock(r2)
    unlock(r1)

T3 wakes:
    lock(r1) ✓
    lock(r2) ✓
    [Critical work]
    unlock(r2)
    unlock(r1)

Result: All threads complete without deadlock
```

### Key Lesson

**With N resources, establish ordering R1 < R2 < ... < RN. All N threads must acquire in this exact order.** No matter what logical operations threads perform, respect the global ordering.

---

## Case 3: Auxiliary Function Hiding Risk

**File References:**
- Problem: `rcs-auxiliar-function-wich-hides-risk.c`
- Solution: `ccs-auxiliar-function-wich-hides-risk.c`

### Problem Description

A thread locks Mutex A, then calls a function that internally locks Mutex B, which then calls another function that tries to lock Mutex A again (without being a recursive mutex).

### The Self-Deadlock Pattern

```c
void *thread1(void *arg) {
    pthread_mutex_lock(&log_mutex);      // Lock log
    atualizar_banco();                   // Call function
    pthread_mutex_unlock(&log_mutex);
}

void atualizar_banco(void) {
    pthread_mutex_lock(&banco_mutex);    // Lock database
    registrar_log();                     // Call nested function
    pthread_mutex_unlock(&banco_mutex);
}

void registrar_log(void) {
    pthread_mutex_lock(&log_mutex);      // TRY AGAIN? SELF-DEADLOCK!
    pthread_mutex_unlock(&log_mutex);
}
```

### Execution

```
Thread 1:
  lock(log_mutex) ✓              Acquire log_mutex
  call atualizar_banco()
    ├─ lock(banco_mutex) ✓       Acquire banco_mutex
    └─ call registrar_log()
      └─ lock(log_mutex) ✗       SELF-DEADLOCK!
         Thread 1 waits for log_mutex held by... Thread 1!
         This is impossible - thread blocked forever
```

### Solution 1: Early Release

```c
void *thread1(void *arg) {
    pthread_mutex_lock(&log_mutex);
    // Do some work
    printf("Logging initial state\n");
    pthread_mutex_unlock(&log_mutex);    // Release BEFORE calling
    
    atualizar_banco();                   // Now safe
    
    return NULL;
}

void atualizar_banco(void) {
    pthread_mutex_lock(&banco_mutex);
    printf("Updating bank\n");
    registrar_log();
    pthread_mutex_unlock(&banco_mutex);
}

void registrar_log(void) {
    // If we need to log:
    pthread_mutex_lock(&log_mutex);      // OK to lock now
    printf("Operation logged\n");
    pthread_mutex_unlock(&log_mutex);
}
```

### Solution 2: Recursive Mutex

```c
void setup(void) {
    pthread_mutexattr_t attr;
    
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&log_mutex, &attr);
    pthread_mutexattr_destroy(&attr);
    
    // Regular (non-recursive) for banco_mutex
    pthread_mutex_init(&banco_mutex, NULL);
}

void *thread1(void *arg) {
    pthread_mutex_lock(&log_mutex);      // 1st lock (count=1)
    atualizar_banco();
    pthread_mutex_unlock(&log_mutex);    // Unlock (count=0)
}

void atualizar_banco(void) {
    pthread_mutex_lock(&banco_mutex);
    registrar_log();
    pthread_mutex_unlock(&banco_mutex);
}

void registrar_log(void) {
    pthread_mutex_lock(&log_mutex);      // 2nd lock (count=2) - OK!
    printf("Logged\n");
    pthread_mutex_unlock(&log_mutex);    // Unlock (count=1)
}
```

### Key Lesson

**Encapsulation can hide dependencies. Function calls may acquire locks you don't expect. Either:**
1. **Release locks before nested calls** (preferred for clear lock scopes)
2. **Use recursive mutexes** (only for intra-thread recursion)
3. **Document lock requirements** (for function interfaces)

---

## Cases 4-10: Summary Table

| # | Name | Problem | Root Cause | Key Solution | Code Order |
|---|------|---------|-----------|--------------|------------|
| 4 | Trylock & Livelock | Non-blocking attempts | Synchronized retry timing | Global ordering + blocking locks | m1 < m2 |
| 5 | Recursive Function | Recursive + inter-thread | Non-recursive + circular | Recursive mutex + ordering | A < B |
| 6 | Producer & Maintain | Hold-and-wait | Dependency chain | Early release before call | Release before subsidiary |
| 7 | Printer/Spooler | Architecture violation | Direct device access | Daemon spooler pattern | Queue-only user access |
| 8 | Abstraction Hidden | Hidden circular chain | Module encapsulation | Explicit ordering across boundaries | All modules |
| 9 | Partial Timeout | Starvation via timeout | Asymmetric threads | Global ordering replaces timeout | Full ordering |
| 10 | Multiple Cycles | Multiple partial circles | Multiple threads/resources | Hierarchy covering all | Complete ordering |

---

## Universal Solution Across All 10 Cases

Every case, despite surface differences, demonstrates the same core principle:

### The Global Lock Ordering Principle

```
DEFINITION: Establish a total order on all mutexes:
    Mutex_1 < Mutex_2 < ... < Mutex_N

RULE: Every thread must acquire mutexes in this order.

GUARANTEE: No circular wait possible.
           Therefore: No deadlock possible.

IMPLEMENTATION:
    for (each critical section in each thread)
        acquire mutexes in order Mutex_1, Mutex_2, ..., Mutex_K
        where Mutex_1 < Mutex_2 < ... < Mutex_K (all < N+1)

VERIFICATION:
    1. Identify all mutex operations
    2. Verify no thread acquires in different order
    3. Ensure all threads respect same ordering
    4. Test with multiple runs and thread counts
```

### Why This Works

**Mathematical Proof (Informal)**:
- Suppose a circular wait exists: T1→M_i→T2→M_j→...→T1
- In a circular wait, at least one thread acquires M_j before M_i
- But global ordering says M_i < M_j for all threads
- This thread violates global ordering
- Contradiction: Therefore no circular wait possible
- If circular wait is impossible, deadlock is impossible (Coffman's condition 4 broken)

---

## Compilation and Testing

### Compile Any Corrected Implementation

```bash
cd /path/to/project

# Compile a specific corrected case
gcc ccs-bank-transfers-with-reverse-order.c -lpthread -o test1

# Run it
./test1

# Expected: Both threads complete, no deadlock

# Or compile all:
for f in ccs-*.c; do
    gcc "$f" -lpthread -o "${f%.c}"
done

# Run all:
for f in ccs-*; do
    echo "=== Running $f ==="
    ./"$f"
    echo
done
```

### Detect if You've Introduced Deadlock

```bash
# With timeout - if it hangs, deadlock present
timeout 5 ./test1
if [ $? -eq 124 ]; then
    echo "DEADLOCK DETECTED - process timed out"
fi

# Run multiple times - if inconsistent, timing-dependent bug
for i in {1..10}; do
    echo "Run $i:"
    ./test1
done
```

---

## Key Takeaways

1. **Deadlock requires all four Coffman conditions** - remove any one and no deadlock
2. **Global lock ordering is most practical** - prevents circular wait mathematically
3. **Order must be consistent across ALL threads** - not just within a single thread
4. **Order is independent of logical operations** - both threads may want opposite resource directions
5. **Test thoroughly** - deadlocks are often timing-dependent and intermittent

