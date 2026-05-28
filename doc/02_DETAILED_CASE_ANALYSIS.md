# Case Study Analysis: 10 Deadlock Scenarios

## Complete Detailed Explanations of Each Scenario

---

## Case 1: Bank Transfers with Reverse Lock Order

### The Scenario

Imagine a banking system where two threads perform concurrent transfers between two accounts:
- **Thread 1**: Transfers money from Account 1 to Account 2
- **Thread 2**: Transfers money from Account 2 to Account 1

### The Problematic Code

```c
void *transferencia_1(void *arg) {
    pthread_mutex_lock(&conta1);        // Lock Account 1
    sleep(1);                           // Simulate processing
    pthread_mutex_lock(&conta2);        // Lock Account 2
    // Perform transfer
    pthread_mutex_unlock(&conta2);
    pthread_mutex_unlock(&conta1);
}

void *transferencia_2(void *arg) {
    pthread_mutex_lock(&conta2);        // Lock Account 2 (REVERSE ORDER!)
    sleep(1);
    pthread_mutex_lock(&conta1);        // Lock Account 1
    // Perform transfer
    pthread_mutex_unlock(&conta1);
    pthread_mutex_unlock(&conta2);
}
```

### Why This Deadlocks

**Execution Timeline:**

```
Time 1: Thread 1 executes
        └─ lock(conta1) ✓ Success
        └─ sleep(1) - CONTEXT SWITCH

Time 2: Thread 2 executes
        └─ lock(conta2) ✓ Success
        └─ lock(conta1) ✗ Blocked! (Thread 1 holds it)
        └─ Waits...

Time 3: Thread 1 wakes up
        └─ lock(conta2) ✗ Blocked! (Thread 2 holds it)
        └─ Waits...

Result: DEADLOCK
        Thread 1 waiting for conta2 (held by Thread 2)
        Thread 2 waiting for conta1 (held by Thread 1)
        Neither can proceed - system stalls
```

### Coffman's Conditions Analysis

✓ **Mutual Exclusion**: Yes - mutexes enforce exclusive access  
✓ **Hold and Wait**: Yes - each thread holds one account while waiting for another  
✓ **No Preemption**: Yes - OS can't forcibly take locks  
✓ **Circular Wait**: Yes - Thread 1 → conta2 → Thread 2 → conta1 → Thread 1  

**All four conditions present = DEADLOCK**

### Visualization

```
Resource Allocation Graph:

        conta1 ←─────── Thread 1
         ↑                   ↓
         │              (waiting for)
         │                   ↓
      Thread 2 ←────── conta2
      
Circle detected: Thread 1 → conta2 → Thread 2 → conta1 → Thread 1
```

### The Solution: Global Lock Ordering

**Key Rule**: ALL threads must acquire locks in the SAME order, regardless of the transfer direction.

```c
// Define global order: conta1 ALWAYS before conta2

void *transferencia_1(void *arg) {
    pthread_mutex_lock(&conta1);        // Order: 1st
    sleep(1);
    pthread_mutex_lock(&conta2);        // Order: 2nd
    printf("Transfer from A to B completed\n");
    pthread_mutex_unlock(&conta2);
    pthread_mutex_unlock(&conta1);
}

void *transferencia_2(void *arg) {
    // FIX: Even though logically B→A, respect global order!
    pthread_mutex_lock(&conta1);        // Order: 1st (CHANGED!)
    sleep(1);
    pthread_mutex_lock(&conta2);        // Order: 2nd (CHANGED!)
    printf("Transfer from B to A completed\n");
    pthread_mutex_unlock(&conta2);
    pthread_mutex_unlock(&conta1);
}
```

### Why This Prevents Deadlock

```
Time 1: Thread 1 executes
        └─ lock(conta1) ✓ Success
        └─ sleep(1) - CONTEXT SWITCH

Time 2: Thread 2 executes
        └─ lock(conta1) ✗ Blocked! (Thread 1 holds it)
        └─ Waits...

Time 3: Thread 1 wakes up
        └─ lock(conta2) ✓ Success (Thread 2 isn't holding it!)
        └─ Complete work
        └─ unlock(conta2)
        └─ unlock(conta1)
        └─ RETURNS

Time 4: Thread 2 continues
        └─ lock(conta1) ✓ Success (now available)
        └─ lock(conta2) ✓ Success
        └─ Complete work
        └─ Unlock...

Result: NO DEADLOCK - Both threads complete successfully
```

**Mathematical Guarantee**: 
- At any moment, exactly one thread can hold conta1
- When that thread holds conta1, it's positioned to acquire conta2 next
- No thread is ever waiting for conta1 while holding conta2
- Therefore, no circular wait is possible

---

## Case 2: Three Threads with Complete Circular Dependency

### The Scenario

Three threads competing for three resources in a circular pattern:
- **Thread 1**: Holds Resource 1, needs Resource 2
- **Thread 2**: Holds Resource 2, needs Resource 3
- **Thread 3**: Holds Resource 3, needs Resource 1

### The Problematic Code

```c
void *t1(void *arg) {
    pthread_mutex_lock(&r1);
    sleep(1);
    pthread_mutex_lock(&r2);        // Needs r2
    printf("t1 executed\n");
    pthread_mutex_unlock(&r2);
    pthread_mutex_unlock(&r1);
}

void *t2(void *arg) {
    pthread_mutex_lock(&r2);
    sleep(1);
    pthread_mutex_lock(&r3);        // Needs r3
    printf("t2 executed\n");
    pthread_mutex_unlock(&r3);
    pthread_mutex_unlock(&r2);
}

void *t3(void *arg) {
    pthread_mutex_lock(&r3);
    sleep(1);
    pthread_mutex_lock(&r1);        // Needs r1 (circles back to T1!)
    printf("t3 executed\n");
    pthread_mutex_unlock(&r1);
    pthread_mutex_unlock(&r3);
}
```

### Why This Deadlocks: The Circular Chain

```
Resource Allocation Chain:

  Thread 1
    ↓ (holds)
  Resource 1
    ↓ (needed by)
  Thread 3
    ↓ (holds)
  Resource 3
    ↓ (needed by)
  Thread 2
    ↓ (holds)
  Resource 2
    ↓ (needed by)
  Thread 1 ← CIRCLE!

Deadlock Cycle: T1 → R2 → T2 → R3 → T3 → R1 → T1
```

### The Execution Timeline

```
T1: lock(r1) ✓ → sleep(1) → [CONTEXT SWITCH]
T2: lock(r2) ✓ → sleep(1) → lock(r3) ✓ → [CONTEXT SWITCH]
T3: lock(r3) ✗ [BLOCKED - T2 holds it]

T1 wakes: lock(r2) ✗ [BLOCKED - T2 holds it]
T2 wakes: lock(r3) ✗ [BLOCKED - T3 holds it]

DEADLOCK: All three threads permanently blocked
```

### The Solution: Total Ordering

```c
// Global order: r1 < r2 < r3
// ALL threads must acquire in this order

void *t1(void *arg) {
    pthread_mutex_lock(&r1);        // 1st
    sleep(1);
    pthread_mutex_lock(&r2);        // 2nd (unchanged)
    printf("t1 executed\n");
    pthread_mutex_unlock(&r2);
    pthread_mutex_unlock(&r1);
}

void *t2(void *arg) {
    pthread_mutex_lock(&r1);        // 1st (CHANGED - was r2)
    sleep(1);
    pthread_mutex_lock(&r2);        // 2nd (CHANGED - was r3)
    printf("t2 executed\n");
    pthread_mutex_unlock(&r2);
    pthread_mutex_unlock(&r1);
}

void *t3(void *arg) {
    pthread_mutex_lock(&r1);        // 1st (CHANGED - was r3)
    sleep(1);
    pthread_mutex_lock(&r2);        // 2nd (CHANGED - was r1)
    printf("t3 executed\n");
    pthread_mutex_unlock(&r2);
    pthread_mutex_unlock(&r1);
}
```

**Key Insight**: Even though threads logically need different resources, they must ALL acquire them in the same sequence.

### Why It Works

With all threads respecting `r1 < r2 < r3`:

```
Execution with global ordering:

T1: lock(r1) ✓ → sleep(1) → [CONTEXT SWITCH]
T2: lock(r1) ✗ [BLOCKED - T1 holds it]
T3: lock(r1) ✗ [BLOCKED - T1 holds it]

T1 wakes: lock(r2) ✓ → sleep(1) → lock(r1) [locked, continue...]
T1: Complete → Release r2, r1

T2 wakes: lock(r1) ✓ → lock(r2) ✓ → Complete
T3 wakes: lock(r1) ✓ → lock(r2) ✓ → Complete

Result: All threads complete successfully - NO DEADLOCK
```

---

## Case 3: Auxiliary Function Hiding Risk

### The Scenario

A thread locks a resource, then calls an auxiliary function that indirectly requires the same resource.

### The Problematic Code

```c
void *thread1(void *arg) {
    pthread_mutex_lock(&log_mutex);      // Lock log
    printf("Thread 1: Starting log operation\n");
    atualizar_banco();                   // Call auxiliary function
    pthread_mutex_unlock(&log_mutex);
}

void atualizar_banco(void) {
    pthread_mutex_lock(&banco_mutex);    // Lock database
    printf("Bank updated\n");
    registrar_log();                     // Call nested function
    pthread_mutex_unlock(&banco_mutex);
}

void registrar_log(void) {
    pthread_mutex_lock(&log_mutex);      // TRY TO LOCK LOG AGAIN!
    printf("Log recorded\n");
    pthread_mutex_unlock(&log_mutex);
}
```

### Why This Self-Deadlocks

```
Execution:

Thread 1:
  lock(log_mutex) ✓                      // Lock acquired
  call atualizar_banco()
    ├─ lock(banco_mutex) ✓
    └─ call registrar_log()
      └─ lock(log_mutex) ✗ DEADLOCK!
         (Same thread tries to lock again, but mutex is not recursive)
         
The thread waits for itself to release the lock - IMPOSSIBLE!
```

### Why Regular Mutexes Fail

```c
// PTHREAD_MUTEX_NORMAL behavior:
// If same thread tries to lock again → UNDEFINED BEHAVIOR or DEADLOCK

// PTHREAD_MUTEX_RECURSIVE allows re-entrancy:
pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
```

### The Problem Hidden by Abstraction

In real code, the problem is hidden:
1. Function `thread1()` doesn't explicitly show that it calls something locking `log_mutex`
2. Function `atualizar_banco()` doesn't show it calls something locking `log_mutex`
3. Only `registrar_log()` explicitly shows the lock requirement
4. **Code reviewer looking at thread1 might never notice this chain!**

### Solution 1: Early Release (Recommended)

Release the primary lock before calling auxiliary functions:

```c
void *thread1(void *arg) {
    pthread_mutex_lock(&log_mutex);
    printf("Thread 1: Starting log operation\n");
    pthread_mutex_unlock(&log_mutex);    // Release BEFORE calling
    atualizar_banco();                   // Now safe
    return NULL;
}

void atualizar_banco(void) {
    pthread_mutex_lock(&banco_mutex);
    printf("Bank updated\n");
    registrar_log();                     // Can safely lock log_mutex
    pthread_mutex_unlock(&banco_mutex);
}

void registrar_log(void) {
    pthread_mutex_lock(&log_mutex);
    printf("Log recorded\n");
    pthread_mutex_unlock(&log_mutex);
}
```

### Solution 2: Recursive Mutexes

Configure `log_mutex` as recursive:

```c
pthread_mutexattr_t attr;
pthread_mutexattr_init(&attr);
pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
pthread_mutex_init(&log_mutex, &attr);

// Now same thread can lock multiple times:
pthread_mutex_lock(&log_mutex);         // 1st lock
pthread_mutex_lock(&log_mutex);         // OK - same thread
pthread_mutex_unlock(&log_mutex);       // 1st unlock
pthread_mutex_unlock(&log_mutex);       // 2nd unlock
```

**Best Practice**: Combine both - use recursive mutexes AND maintain global lock ordering across threads to prevent inter-thread deadlocks.

---

## Case 4: Trylock and Livelock Risk

### The Scenario

Using non-blocking mutex attempts (`pthread_mutex_trylock()`) with symmetric retry logic, causing livelock instead of traditional deadlock.

### The Problematic Code

```c
void *t1(void *arg) {
    for (int attempt = 0; attempt < 100; attempt++) {
        if (pthread_mutex_trylock(&m1) == 0) {
            if (pthread_mutex_trylock(&m2) == 0) {
                // Got both locks
                printf("t1 executed\n");
                pthread_mutex_unlock(&m2);
                pthread_mutex_unlock(&m1);
                break;
            }
            // Didn't get m2, release m1 and retry
            pthread_mutex_unlock(&m1);
        }
        usleep(100);  // Wait before retry - SAME TIME FOR BOTH THREADS!
    }
}

void *t2(void *arg) {
    for (int attempt = 0; attempt < 100; attempt++) {
        if (pthread_mutex_trylock(&m2) == 0) {  // Different order!
            if (pthread_mutex_trylock(&m1) == 0) {
                printf("t2 executed\n");
                pthread_mutex_unlock(&m1);
                pthread_mutex_unlock(&m2);
                break;
            }
            pthread_mutex_unlock(&m2);
        }
        usleep(100);  // Same sleep time - SYNCHRONIZED!
    }
}
```

### Why This Causes Livelock

```
Time 100ms: T1 and T2 wake simultaneously

T1:  trylock(m1) ✓ → sleep(100) → [PREEMPT]
T2:  trylock(m2) ✓ → sleep(100) → [PREEMPT]

T1 wakes: trylock(m2) ✗ (T2 holds it) → unlock(m1) → sleep(100)
T2 wakes: trylock(m1) ✗ (T1 holds it) → unlock(m2) → sleep(100)

T1 and T2 wake SIMULTANEOUSLY again after 100ms:

T1:  trylock(m1) ✓ → trylock(m2) ✗ → unlock(m1) → sleep(100)
T2:  trylock(m2) ✓ → trylock(m1) ✗ → unlock(m2) → sleep(100)

... REPEATS FOREVER ...

LIVELOCK: Threads are executing but making NO PROGRESS
```

### Difference Between Deadlock and Livelock

| Aspect | Deadlock | Livelock |
|--------|----------|----------|
| **Threads Blocked** | Yes - completely stuck | No - actively running |
| **CPU Usage** | Minimal - threads sleeping | High - threads executing |
| **Detection** | Easier (no thread progress) | Harder (threads appear busy) |
| **Observable State** | Static - nothing changes | Dynamic - state changes but no progress |

### Why Symmetric Retry Fails

The critical problem: **Both threads retry with the same timing**, creating a synchronized pattern where they continuously interfere with each other.

### The Solution: Global Lock Ordering

```c
// Global order: m1 ALWAYS before m2

void *t1(void *arg) {
    pthread_mutex_lock(&m1);            // Order: 1st
    pthread_mutex_lock(&m2);            // Order: 2nd
    printf("t1 executed\n");
    pthread_mutex_unlock(&m2);
    pthread_mutex_unlock(&m1);
    return NULL;
}

void *t2(void *arg) {
    pthread_mutex_lock(&m1);            // Order: 1st (CHANGED!)
    pthread_mutex_lock(&m2);            // Order: 2nd (CHANGED!)
    printf("t2 executed\n");
    pthread_mutex_unlock(&m2);
    pthread_mutex_unlock(&m1);
    return NULL;
}
```

**Why it works**: One thread WILL get m1 first. That thread can proceed to get m2. No retry loops needed.

---

## Cases 5-10: Summary

| Case | Problem | Root Cause | Solution |
|------|---------|-----------|----------|
| **5** | Recursive locks + inter-thread circle | Non-recursive mutex + reverse ordering | Recursive mutex + global lock ordering |
| **6** | Hold-and-call pattern | Auxiliary function needs other resources | Early release before nested calls |
| **7** | Spooler architecture violation | Direct user-device contention | Daemon pattern: users use queue only |
| **8** | Abstraction hides lock ordering | Indirect lock chains in modules | Explicit global ordering across modules |
| **9** | Timeout-based attempts | Task abandonment via asymmetric trylock | Symmetric global lock ordering |
| **10** | Multiple partial cycles | Multiple resources in different threads | Hierarchy covering all resources |

---

## Universal Principle

**Every case in this project demonstrates the same universal solution:**

> **Enforce a Global Lock Ordering**
> 
> Define a total order on ALL mutexes in the system:
> Lock_A < Lock_B < Lock_C < ... < Lock_Z
>
> EVERY thread MUST acquire locks in this order.
> 
> This mathematically eliminates circular wait.
> No deadlock is possible.

