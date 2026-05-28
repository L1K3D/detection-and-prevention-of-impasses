# Deep Dive: Cases 5-10 Complete Analysis

## Continuation of the 10 Case Studies

This document provides the same level of detail for Cases 5-10 as was given for Cases 1-4.

---

## Case 5: Resources Acquired in Recursive Function

**File References:**
- Problem: `rcs-resources-getted-in-recursive-function.c`
- Solution: `ccs-resources-getted-in-recursive-function.c`

### Problem Description

A function recursively calls itself, acquiring a lock each time. Additionally, another thread tries to acquire a different lock, creating a potential circular dependency.

**Scenario**: 
- Thread 1: Recursively acquires Lock A, then tries Lock B
- Thread 2: Acquires Lock B, then tries Lock A

### The Issue

```c
pthread_mutex_t lockA, lockB;

void recursive_function(int depth) {
    pthread_mutex_lock(&lockA);          // Each recursion acquires lockA
    printf("Depth %d\n", depth);
    
    if (depth > 0) {
        recursive_function(depth - 1);   // Recursive call - locks again!
    }
    
    pthread_mutex_unlock(&lockA);
}

void *thread_1(void *arg) {
    recursive_function(3);               // 3 levels of recursion
    pthread_mutex_lock(&lockB);          // After recursion, try lockB
    // Work...
    pthread_mutex_unlock(&lockB);
    return NULL;
}

void *thread_2(void *arg) {
    pthread_mutex_lock(&lockB);          // Different order!
    // Work...
    pthread_mutex_lock(&lockA);          // Circular dependency
    // Work...
    pthread_mutex_unlock(&lockA);
    pthread_mutex_unlock(&lockB);
    return NULL;
}
```

### Why This Deadlocks

**Two problems intersect:**

**Problem 1: Recursive Self-Locking**
- If `lockA` is a normal mutex, Thread 1's recursive call will deadlock with itself
- Thread 1 holds `lockA`, recursively tries to lock `lockA` again
- Result: Thread 1 self-deadlocks

**Problem 2: Circular Wait Between Threads**
- Even if Problem 1 is solved with recursive mutex:
  - Thread 1 ends up holding both `lockA` (via recursion) and `lockB`
  - Thread 2 holds `lockB` and waits for `lockA`
  - Thread 1 waits for `lockB` (held by Thread 2)
  - Result: Inter-thread deadlock

### Coffman's Analysis

| Condition | Present? | Why |
|-----------|----------|-----|
| **Mutual Exclusion** | ✓ Yes | Mutexes provide exclusion |
| **Hold and Wait** | ✓ Yes | Threads hold one lock waiting for another |
| **No Preemption** | ✓ Yes | Normal mutex behavior |
| **Circular Wait** | ✓ Yes | T1→B→T2→A→T1 (inter-thread) |

### The Corrected Solution

**Key Insight**: Use recursive mutex for intra-thread recursion, AND global lock ordering for inter-thread safety.

```c
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

pthread_mutex_t lockA, lockB;
int global_counter = 0;

void setup_mutexes(void) {
    // Lock A: Recursive (for recursive function)
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&lockA, &attr);
    pthread_mutexattr_destroy(&attr);
    
    // Lock B: Normal
    pthread_mutex_init(&lockB, NULL);
}

void recursive_function(int depth) {
    // Safe to lock multiple times due to recursive mutex
    pthread_mutex_lock(&lockA);
    printf("Thread entering recursion at depth %d\n", depth);
    
    global_counter++;
    
    if (depth > 0) {
        recursive_function(depth - 1);   // Same thread locks again
    }
    
    printf("Thread exiting recursion at depth %d\n", depth);
    pthread_mutex_unlock(&lockA);
}

void *thread_1(void *arg) {
    printf("T1: Starting recursive function\n");
    
    // CORRECTED: Respect global ordering: lockA < lockB
    recursive_function(2);               // This acquires lockA multiple times
    
    // After recursion completes, acquire second lock
    pthread_mutex_lock(&lockB);          // 2nd in order
    printf("T1: Acquired lockB\n");
    
    // Critical section with both locks
    printf("T1: Both locks held\n");
    sleep(1);
    
    pthread_mutex_unlock(&lockB);
    printf("T1: Completed\n");
    
    return NULL;
}

void *thread_2(void *arg) {
    printf("T2: Starting\n");
    sleep(1);  // Let T1 start first
    
    // CORRECTED: Same lock order as T1 - lockA before lockB!
    pthread_mutex_lock(&lockA);          // 1st in order (CHANGED)
    printf("T2: Acquired lockA\n");
    
    pthread_mutex_lock(&lockB);          // 2nd in order (CHANGED)
    printf("T2: Acquired lockB\n");
    
    // Critical section
    printf("T2: Both locks held\n");
    
    pthread_mutex_unlock(&lockB);
    pthread_mutex_unlock(&lockA);
    printf("T2: Completed\n");
    
    return NULL;
}

int main(void) {
    pthread_t tid1, tid2;
    
    setup_mutexes();
    
    pthread_create(&tid1, NULL, thread_1, NULL);
    pthread_create(&tid2, NULL, thread_2, NULL);
    
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    
    printf("Global counter: %d\n", global_counter);
    
    pthread_mutex_destroy(&lockA);
    pthread_mutex_destroy(&lockB);
    
    return 0;
}
```

### Why This Works

```
1. Recursive mutex allows Thread 1 to lock multiple times:
   lock(A) ✓ → call recursive_function()
   lock(A) ✓ (same thread, allowed by recursive mutex)
   lock(A) ✓ (same thread again)
   unlock(A) ← (count = 2)
   unlock(A) ← (count = 1)
   unlock(A) ← (count = 0)

2. Global ordering prevents inter-thread deadlock:
   T1 and T2 both follow: lockA < lockB
   One thread succeeds in getting lockA first
   That thread can then get lockB
   No circular wait possible
```

### Key Lessons

1. **Recursive mutexes solve intra-thread recursion** - same thread can acquire multiple times
2. **Global ordering still required between threads** - prevents circular wait
3. **Combine both approaches** - recursive mutex at leaf level, global ordering at orchestration level
4. **Must unlock same number of times as locked** - lock count must reach zero

---

## Case 6: Producer, Maintenance, and Auxiliary Functions

**File References:**
- Problem: `rcs-production-maintence-and-auxiliar-function.c`
- Solution: `ccs-production-maintence-and-auxiliar-function.c`

### Problem Description

A producer thread maintains a queue (with queue lock) while also updating statistics (with statistics lock). The maintenance is done through auxiliary functions that may need different locks.

**Scenario**:
- Producer Thread: Enqueue items (lock fila_mutex), then update statistics (lock estat_mutex)
- Maintenance Thread: Access statistics (lock estat_mutex), then dequeue from producer queue (lock fila_mutex)

### The Hold-and-Wait Pattern

```c
pthread_mutex_t fila_mutex, estat_mutex;
int queue_count = 0;
int processed_count = 0;

void *producer(void *arg) {
    while (items_to_produce) {
        pthread_mutex_lock(&fila_mutex);      // Lock queue
        enqueue_item();
        queue_count++;
        // Do work...
        
        pthread_mutex_lock(&estat_mutex);    // Hold fila, acquire estat
        update_statistics();
        pthread_mutex_unlock(&estat_mutex);
        
        pthread_mutex_unlock(&fila_mutex);
    }
    return NULL;
}

void *maintenance(void *arg) {
    while (true) {
        pthread_mutex_lock(&estat_mutex);     // Lock statistics
        check_metrics();
        
        pthread_mutex_lock(&fila_mutex);      // Different order! DEADLOCK
        if (queue_count > THRESHOLD) {
            // Adjust queue...
        }
        pthread_mutex_unlock(&fila_mutex);
        
        pthread_mutex_unlock(&estat_mutex);
    }
    return NULL;
}
```

### Deadlock Scenario

```
Producer:      holds fila_mutex, waits for estat_mutex
Maintenance:   holds estat_mutex, waits for fila_mutex

Circular Wait: Producer → estat → Maintenance → fila → Producer
```

### The Corrected Solution

**Strategy**: Early release before calling maintenance operations

```c
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

pthread_mutex_t fila_mutex, estat_mutex;
int queue_count = 0;
int processed_count = 0;

void enqueue_item(void) {
    queue_count++;
    printf("Item enqueued. Queue size: %d\n", queue_count);
}

void update_statistics(void) {
    processed_count++;
    printf("Statistics updated. Processed: %d\n", processed_count);
}

void *producer(void *arg) {
    printf("Producer: Starting\n");
    
    for (int i = 0; i < 5; i++) {
        // CORRECTED: Global ordering - fila < estat
        pthread_mutex_lock(&fila_mutex);      // 1st
        
        enqueue_item();
        
        // Do queue-specific work
        sleep(1);  // Simulate processing
        
        pthread_mutex_unlock(&fila_mutex);    // Release BEFORE maintenance
        
        // Now call maintenance function without holding fila_mutex
        pthread_mutex_lock(&estat_mutex);    // 2nd (CHANGED - now acquired separately)
        update_statistics();
        pthread_mutex_unlock(&estat_mutex);
        
        printf("Producer: Iteration %d complete\n", i);
    }
    
    return NULL;
}

void *maintenance(void *arg) {
    printf("Maintenance: Starting\n");
    
    for (int i = 0; i < 3; i++) {
        sleep(2);  // Let producer work
        
        // CORRECTED: Same global ordering - fila < estat
        // Acquire locks in defined order
        pthread_mutex_lock(&fila_mutex);      // 1st (CHANGED - acquire first)
        printf("Maintenance: Locked fila\n");
        
        pthread_mutex_lock(&estat_mutex);     // 2nd (CHANGED - acquire second)
        printf("Maintenance: Locked estat\n");
        
        // Critical section with both locks
        printf("Maintenance: Queue=%d, Processed=%d\n", 
               queue_count, processed_count);
        
        if (queue_count > 3) {
            printf("Maintenance: Queue size above threshold\n");
        }
        
        pthread_mutex_unlock(&estat_mutex);
        pthread_mutex_unlock(&fila_mutex);
        
        printf("Maintenance: Iteration %d complete\n", i);
    }
    
    return NULL;
}

int main(void) {
    pthread_t tid_producer, tid_maintenance;
    
    pthread_mutex_init(&fila_mutex, NULL);
    pthread_mutex_init(&estat_mutex, NULL);
    
    pthread_create(&tid_producer, NULL, producer, NULL);
    pthread_create(&tid_maintenance, NULL, maintenance, NULL);
    
    pthread_join(tid_producer, NULL);
    pthread_join(tid_maintenance, NULL);
    
    printf("Final: Queue=%d, Processed=%d\n", queue_count, processed_count);
    
    pthread_mutex_destroy(&fila_mutex);
    pthread_mutex_destroy(&estat_mutex);
    
    return EXIT_SUCCESS;
}
```

### Key Insight

**Breaking Hold-and-Wait Condition**:
- Original: Producer holds fila_mutex while acquiring estat_mutex (Hold-and-Wait)
- Corrected: Producer releases fila_mutex before acquiring estat_mutex (No Hold-and-Wait)
- Alternative: Could use global ordering where both functions always acquire in the same order

### Why This Works

```
Option 1: Early Release (used here)
Producer: lock(fila) → work → unlock(fila) → lock(estat) → work → unlock(estat)
Maintenance: lock(fila) → work → unlock(fila) → lock(estat) → work → unlock(estat)
No circular wait because locks are never held together from different threads

Option 2: Global Ordering (alternative)
Producer: lock(fila) → lock(estat) → work → unlock(estat) → unlock(fila)
Maintenance: lock(fila) → lock(estat) → work → unlock(estat) → unlock(fila)
No circular wait because all threads use the same order
```

### Key Lesson

**Hold-and-Wait can be prevented by releasing one lock before acquiring another.** This works when the operations on the two resources are independent.

---

## Case 7: Printer and Spooler with Incorrect Model

**File References:**
- Problem: `rcs-printer-and-spooner-with-incorrect-model.c`
- Solution: `ccs-printer-and-spooner-with-incorrect-model.c`

### Problem Description

Multiple threads try to directly access both a printer device and a spooler queue simultaneously.

**Incorrect Model**:
```
User Thread 1 ──→ Printer Device
User Thread 2 ──→ Printer Device  ← Direct access causes contention
User Thread 3 ──→ Printer Device

User Thread 1 ──→ Spooler Queue
User Thread 2 ──→ Spooler Queue   ← Conflicting access patterns
User Thread 3 ──→ Spooler Queue
```

### The Problem Code

```c
pthread_mutex_t impressora, spooler;

void *usuario_thread(void *arg) {
    int id = (intptr_t)arg;
    
    // Try to directly access both device and queue
    pthread_mutex_lock(&spooler);        // Acquire spooler
    enqueue_job(id);
    
    pthread_mutex_lock(&impressora);     // Try to acquire printer
    print_job();                          // Direct device access!
    
    pthread_mutex_unlock(&impressora);
    pthread_mutex_unlock(&spooler);
}
```

### The Architectural Problem

With multiple threads directly accessing both resources, complex lock ordering becomes necessary:
- Some threads want printer→spooler
- Some threads want spooler→printer
- Deadlock risk is high
- Architecture is "spaghetti" with multiple access patterns

### The Correct Architectural Solution: Spooler Daemon Pattern

```c
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <queue.h>

typedef struct {
    int job_id;
    int user_id;
} PrintJob;

pthread_mutex_t fila_spooler;
pthread_queue_t fila;  // Queue of PrintJob structures
int printer_ready = 1;
pthread_mutex_t impressora;

// Spooler daemon - only this accesses the printer directly
void *daemon_spooler(void *arg) {
    printf("Daemon: Starting spooler\n");
    
    while (1) {
        PrintJob job;
        int has_job = 0;
        
        // Step 1: Get job from queue
        pthread_mutex_lock(&fila_spooler);
        if (!queue_empty(&fila)) {
            job = queue_dequeue(&fila);
            has_job = 1;
        }
        pthread_mutex_unlock(&fila_spooler);
        
        // Step 2: No lock held here - safe to do other work
        if (has_job) {
            sleep(1);  // Simulate print processing
            
            // Step 3: Only daemon accesses printer
            pthread_mutex_lock(&impressora);
            printf("Daemon: Printing job %d from user %d\n", 
                   job.job_id, job.user_id);
            printf("Daemon: Job printed\n");
            pthread_mutex_unlock(&impressora);
        } else {
            usleep(100000);  // Sleep if no job
        }
    }
    
    return NULL;
}

// User threads: ONLY interact with queue
void *usuario_thread(void *arg) {
    int user_id = (intptr_t)arg;
    
    for (int i = 0; i < 3; i++) {
        PrintJob job;
        job.job_id = user_id * 10 + i;
        job.user_id = user_id;
        
        // ONLY lock: queue
        pthread_mutex_lock(&fila_spooler);
        queue_enqueue(&fila, job);
        printf("User %d: Submitted job %d\n", user_id, job.job_id);
        pthread_mutex_unlock(&fila_spooler);
        
        // No printer access!
        sleep(2);
    }
    
    printf("User %d: Done submitting jobs\n", user_id);
    return NULL;
}

int main(void) {
    pthread_t tid_users[3];
    pthread_t tid_daemon;
    
    queue_init(&fila);
    pthread_mutex_init(&fila_spooler, NULL);
    pthread_mutex_init(&impressora, NULL);
    
    // Start daemon
    pthread_create(&tid_daemon, NULL, daemon_spooler, NULL);
    
    // Start user threads
    for (int i = 0; i < 3; i++) {
        pthread_create(&tid_users[i], NULL, usuario_thread, 
                      (void *)(intptr_t)i);
    }
    
    // Wait for users
    for (int i = 0; i < 3; i++) {
        pthread_join(tid_users[i], NULL);
    }
    
    // Wait for daemon to finish remaining jobs
    sleep(10);
    // (In real code, signal daemon to exit)
    
    pthread_mutex_destroy(&fila_spooler);
    pthread_mutex_destroy(&impressora);
    
    return 0;
}
```

### Why This Architecture Prevents Deadlock

```
BEFORE (Incorrect):
  User threads directly access both printer and spooler
  → Complex lock ordering needed
  → Multiple contention points
  → High deadlock risk

AFTER (Correct Spooler Pattern):
  User threads: Only access queue (single lock)
  Daemon thread: Only accesses printer (single lock)
  → Separation of concerns
  → No circular dependencies possible
  → Clean architecture
```

### Key Insight

**Architectural separation can prevent deadlock by reducing contention and eliminating circular dependencies.** The spooler daemon is a classical OS design pattern.

---

## Case 8: Deadlock Hidden by Module Abstraction

**File References:**
- Problem: `rcs-hide-deadlock-bypass-model.c`
- Solution: `ccs-hide-deadlock-bypass-model.c`

### Problem Description

One module locks cache_mutex then disk_mutex. Another module tries to lock disk_mutex first then cache_mutex. The lock order is hidden inside function implementations.

### The Hidden Circular Wait

```c
// Module A: Cache implementation
void save_cache_item(int item) {
    pthread_mutex_lock(&cache_mutex);        // 1st lock: cache
    // Modify cache
    flush_to_disk(item);                     // Calls...
    // Inside flush_to_disk:
    pthread_mutex_lock(&disk_mutex);         // 2nd lock: disk
}

// Module B: Disk operations
void update_disk_directly(int item) {
    pthread_mutex_lock(&disk_mutex);         // Different order!
    // Modify disk
    update_cache(item);                      // Calls...
    // Inside update_cache:
    pthread_mutex_lock(&cache_mutex);        // Different order
}
```

### The Problem

Developers can't see from the caller level that:
- `save_cache_item()` acquires cache_mutex then disk_mutex
- `update_disk_directly()` acquires disk_mutex then cache_mutex

This creates a hidden circular wait:
```
Thread 1: save_cache_item() holds cache, waits for disk
Thread 2: update_disk_directly() holds disk, waits for cache

Deadlock: Neither thread can proceed
```

### The Corrected Solution

**Make the lock ordering explicit and global:**

```c
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

pthread_mutex_t cache_mutex, disk_mutex;
int cache_data = 0;
int disk_data = 0;

// GLOBAL ORDERING ESTABLISHED:
// Rule 1: cache_mutex ALWAYS acquired before disk_mutex
// Rule 2: This must be respected in ALL functions

void write_to_disk(int value) {
    // This function assumes cache_mutex is ALREADY held
    // It only acquires disk_mutex
    
    pthread_mutex_lock(&disk_mutex);      // 2nd lock (cache should be 1st)
    disk_data = value;
    printf("  Disk: Written %d\n", value);
    pthread_mutex_unlock(&disk_mutex);
}

void update_cache_with_disk_check(int value) {
    // This function assumes disk_mutex is ALREADY held
    // It only acquires cache_mutex
    
    // BUG FIX: This function must NOT acquire cache_mutex!
    // Instead, caller must already hold both or manage ordering
    
    cache_data = value;
    printf("  Cache: Updated %d\n", value);
}

void save_cache_item(int item) {
    printf("save_cache_item(%d) starting\n", item);
    
    // CORRECTED: Global ordering - cache BEFORE disk
    pthread_mutex_lock(&cache_mutex);     // 1st
    printf("  Locked cache\n");
    
    cache_data = item;
    printf("  Cache: Wrote %d\n", item);
    
    // DO NOT use write_to_disk() - it will lock disk
    // Instead, lock disk here in correct order
    pthread_mutex_lock(&disk_mutex);      // 2nd
    printf("  Locked disk\n");
    
    disk_data = item;  // Direct write, not via function
    printf("  Disk: Wrote %d\n", item);
    
    pthread_mutex_unlock(&disk_mutex);
    pthread_mutex_unlock(&cache_mutex);
    printf("save_cache_item complete\n\n");
}

void update_disk_directly(int item) {
    printf("update_disk_directly(%d) starting\n", item);
    
    // CORRECTED: Same global ordering - cache BEFORE disk
    pthread_mutex_lock(&cache_mutex);     // 1st (CHANGED!)
    printf("  Locked cache\n");
    
    cache_data = item;
    printf("  Cache: Updated %d\n", item);
    
    pthread_mutex_lock(&disk_mutex);      // 2nd (CHANGED!)
    printf("  Locked disk\n");
    
    disk_data = item;  // Direct write
    printf("  Disk: Updated %d\n", item);
    
    pthread_mutex_unlock(&disk_mutex);
    pthread_mutex_unlock(&cache_mutex);
    printf("update_disk_directly complete\n\n");
}

void *thread_1(void *arg) {
    for (int i = 0; i < 2; i++) {
        save_cache_item(100 + i);
        sleep(1);
    }
    return NULL;
}

void *thread_2(void *arg) {
    for (int i = 0; i < 2; i++) {
        sleep(1);
        update_disk_directly(200 + i);
    }
    return NULL;
}

int main(void) {
    pthread_t tid1, tid2;
    
    pthread_mutex_init(&cache_mutex, NULL);
    pthread_mutex_init(&disk_mutex, NULL);
    
    printf("Main: Starting threads with correct lock ordering\n\n");
    
    pthread_create(&tid1, NULL, thread_1, NULL);
    pthread_create(&tid2, NULL, thread_2, NULL);
    
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    
    printf("Main: Final values - Cache=%d, Disk=%d\n", 
           cache_data, disk_data);
    
    pthread_mutex_destroy(&cache_mutex);
    pthread_mutex_destroy(&disk_mutex);
    
    return EXIT_SUCCESS;
}
```

### Key Lesson

**Abstraction and modularization can hide lock requirements and lead to deadlock. Solution: Document lock ordering globally and verify all modules respect it.**

Best Practice: Create a lock hierarchy document:
```
GLOBAL LOCK ORDERING (Project-wide)
===================================
Priority 1: cache_mutex
Priority 2: disk_mutex
Priority 3: (others if any)

RULE: All modules must acquire in this order
```

---

## Case 9: Partial Timeout and Inconsistent Solution

**File References:**
- Problem: `rcs-parcial-timeout-and-inconsistent-solution.c`
- Solution: `css-parcial-timeout-and-inconsistent-solution.c` (note: css- not ccs-)

### Problem Description

One thread uses `trylock()` with timeout/retry, while another uses blocking lock. This asymmetry causes one thread to consistently fail while the other succeeds.

### The Asymmetric Pattern

```c
void *thread_1_with_trylock(void *arg) {
    for (int attempt = 0; attempt < 10; attempt++) {
        // Non-blocking attempt
        if (pthread_mutex_trylock(&x) == 0) {
            if (pthread_mutex_trylock(&y) == 0) {
                // Got both
                work();
                pthread_mutex_unlock(&y);
                pthread_mutex_unlock(&x);
                return NULL;  // Success
            }
            pthread_mutex_unlock(&x);
        }
        usleep(100);  // Retry after delay
    }
    printf("T1: FAILED - gave up\n");
    return NULL;
}

void *thread_2_blocking(void *arg) {
    // Blocking locks - will eventually get them
    pthread_mutex_lock(&y);        // Different order!
    pthread_mutex_lock(&x);
    
    work();
    
    pthread_mutex_unlock(&x);
    pthread_mutex_unlock(&y);
    return NULL;  // Success
}
```

### The Problem: Starvation Instead of Deadlock

```
T1 repeatedly tries: lock(x) → if success, lock(y) → if success, work → unlock
T2 blocks: lock(y) → waits...

Timing:
T1: lock(x) ✓ → lock(y) ✗ (T2 has it) → unlock(x) → retry
T2: lock(y) ✓ → lock(x) ✗ (T1 has it during retry) → waits

T1 keeps retrying but can never get both because T2 keeps y locked
T2 is stuck waiting for x while T1 keeps trying

Result: Both threads starve - neither makes progress
```

### The Corrected Solution: Global Blocking Lock Ordering

```c
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

pthread_mutex_t x, y;
int global_state = 0;

void do_work(int thread_id) {
    printf("Thread %d: Doing critical work\n", thread_id);
    global_state++;
    printf("Thread %d: Global state = %d\n", thread_id, global_state);
}

void *thread_1(void *arg) {
    printf("T1: Starting\n");
    
    for (int i = 0; i < 3; i++) {
        // CORRECTED: Blocking locks in global order: x < y
        printf("T1: Attempt %d\n", i);
        
        pthread_mutex_lock(&x);        // 1st
        printf("T1: Locked x\n");
        
        usleep(500);  // Simulate work
        
        pthread_mutex_lock(&y);        // 2nd
        printf("T1: Locked y\n");
        
        do_work(1);
        
        pthread_mutex_unlock(&y);
        pthread_mutex_unlock(&x);
        printf("T1: Released locks\n\n");
    }
    
    printf("T1: Completed successfully\n");
    return NULL;
}

void *thread_2(void *arg) {
    printf("T2: Starting\n");
    
    for (int i = 0; i < 3; i++) {
        printf("T2: Attempt %d\n", i);
        
        // CORRECTED: SAME order as T1 - x before y!
        pthread_mutex_lock(&x);        // 1st (CHANGED from y)
        printf("T2: Locked x\n");
        
        usleep(500);
        
        pthread_mutex_lock(&y);        // 2nd (CHANGED from x)
        printf("T2: Locked y\n");
        
        do_work(2);
        
        pthread_mutex_unlock(&y);
        pthread_mutex_unlock(&x);
        printf("T2: Released locks\n\n");
    }
    
    printf("T2: Completed successfully\n");
    return NULL;
}

int main(void) {
    pthread_t tid1, tid2;
    
    pthread_mutex_init(&x, NULL);
    pthread_mutex_init(&y, NULL);
    
    printf("Main: Creating threads with global lock ordering (x < y)\n\n");
    
    pthread_create(&tid1, NULL, thread_1, NULL);
    pthread_create(&tid2, NULL, thread_2, NULL);
    
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    
    printf("\nMain: Both threads completed\n");
    printf("Final global state: %d (should be 6)\n", global_state);
    
    pthread_mutex_destroy(&x);
    pthread_mutex_destroy(&y);
    
    return EXIT_SUCCESS;
}
```

### Key Insight

**Timeout-based approaches (`trylock`, `timedlock`) do NOT solve deadlock - they only replace it with starvation or livelock.** The correct solution is proper lock ordering.

### Why Global Ordering Works Here

```
With x < y ordering:

T1: lock(x) ✓ → lock(y) ✓ → work → unlock(y) → unlock(x)
T2: [waits for x]

T2: lock(x) ✓ → lock(y) ✓ → work → unlock(y) → unlock(x)

Result: Both threads complete successfully
No starvation because blocking locks are fair
No deadlock because ordering prevents circular wait
```

---

## Case 10: Multiple Resources with Dependency Cycles

**File References:**
- Problem: `rcs-multiple-resources-with-dependence-cicle.c`
- Solution: `ccs-multiple-resources-with-dependence-cicle.c`

### Problem Description

Three threads, three resources (A, B, C), with multiple partial circular wait cycles.

### Potential Circular Patterns

```
Configuration 1:
T1: A → B
T2: B → C
T3: C → A
Cycle: A → B → C → A

Configuration 2 (partial):
T1: A → B → C
T2: B → C
T3: C → A
Multiple incomplete cycles that combine into complete cycle
```

### The Problematic Code

```c
pthread_mutex_t a, b, c;

void *t1(void *arg) {
    pthread_mutex_lock(&a);
    sleep(1);
    pthread_mutex_lock(&b);
    work();
    pthread_mutex_unlock(&b);
    pthread_mutex_unlock(&a);
}

void *t2(void *arg) {
    pthread_mutex_lock(&b);
    sleep(1);
    pthread_mutex_lock(&c);
    work();
    pthread_mutex_unlock(&c);
    pthread_mutex_unlock(&b);
}

void *t3(void *arg) {
    pthread_mutex_lock(&c);
    sleep(1);
    pthread_mutex_lock(&a);     // Cycle: T1→b→T2→c→T3→a→T1
    work();
    pthread_mutex_unlock(&a);
    pthread_mutex_unlock(&c);
}
```

### The Corrected Solution: Total Ordering

```c
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

pthread_mutex_t a, b, c;
int work_counter = 0;

void *t1(void *arg) {
    printf("T1: Starting\n");
    
    // CORRECTED: Total order: a < b < c
    pthread_mutex_lock(&a);    // 1st
    printf("T1: Locked a\n");
    sleep(1);
    
    pthread_mutex_lock(&b);    // 2nd
    printf("T1: Locked b\n");
    
    // Critical section
    work_counter++;
    printf("T1: Work done. Counter = %d\n", work_counter);
    
    pthread_mutex_unlock(&b);
    pthread_mutex_unlock(&a);
    printf("T1: Completed\n\n");
    
    return NULL;
}

void *t2(void *arg) {
    printf("T2: Starting\n");
    
    // CORRECTED: Same total order: a < b < c
    pthread_mutex_lock(&a);    // 1st (CHANGED from b)
    printf("T2: Locked a\n");
    sleep(1);
    
    pthread_mutex_lock(&b);    // 2nd (CHANGED from c)
    printf("T2: Locked b\n");
    
    // Critical section
    work_counter++;
    printf("T2: Work done. Counter = %d\n", work_counter);
    
    pthread_mutex_unlock(&b);
    pthread_mutex_unlock(&a);
    printf("T2: Completed\n\n");
    
    return NULL;
}

void *t3(void *arg) {
    printf("T3: Starting\n");
    
    // CORRECTED: Same total order: a < b < c
    pthread_mutex_lock(&a);    // 1st (CHANGED from c)
    printf("T3: Locked a\n");
    sleep(1);
    
    pthread_mutex_lock(&b);    // 2nd (CHANGED from a)
    printf("T3: Locked b\n");
    
    // Critical section
    work_counter++;
    printf("T3: Work done. Counter = %d\n", work_counter);
    
    pthread_mutex_unlock(&b);
    pthread_mutex_unlock(&a);
    printf("T3: Completed\n\n");
    
    return NULL;
}

int main(void) {
    pthread_t tid1, tid2, tid3;
    
    pthread_mutex_init(&a, NULL);
    pthread_mutex_init(&b, NULL);
    pthread_mutex_init(&c, NULL);
    
    printf("Main: Creating 3 threads with total ordering (a < b < c)\n\n");
    
    pthread_create(&tid1, NULL, t1, NULL);
    pthread_create(&tid2, NULL, t2, NULL);
    pthread_create(&tid3, NULL, t3, NULL);
    
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    pthread_join(tid3, NULL);
    
    printf("Main: All threads completed\n");
    printf("Work counter: %d (should be 3)\n", work_counter);
    
    pthread_mutex_destroy(&a);
    pthread_mutex_destroy(&b);
    pthread_mutex_destroy(&c);
    
    return EXIT_SUCCESS;
}
```

### Why This Works

```
With total ordering a < b < c:

T1: lock(a) ✓ → sleep(1) → [may be preempted]
T2: lock(a) ✗ [T1 has it, waits]
T3: lock(a) ✗ [T1 has it, waits]

T1 wakes: lock(b) ✓ → work → unlock(b) → unlock(a)

T2: lock(a) ✓ → sleep(1) → [may be preempted]
T3: lock(a) ✗ [T2 has it, waits]

T2 wakes: lock(b) ✓ → work → unlock(b) → unlock(a)

T3: lock(a) ✓ → lock(b) ✓ → work → unlock(b) → unlock(a)

Result: All threads complete successfully
No partial cycles can form into complete cycle with total ordering
```

### Key Lesson

**Total (complete) ordering on all N resources prevents ANY possibility of circular wait, regardless of thread complexity or number of resources.**

---

## Summary: Cases 5-10

| Case | Mutexes | Complexity | Problem Type | Solution |
|------|---------|-----------|-------------|----------|
| 5 | 2 | Recursive + inter-thread | Double deadlock (intra + inter) | Recursive mutex + global order |
| 6 | 2 | Sequential phases | Hold-and-wait | Early release between phases |
| 7 | 2 | Architectural | Direct multi-access | Daemon pattern (separation) |
| 8 | 2 | Abstraction | Hidden circular chain | Explicit global ordering |
| 9 | 2 | Asymmetric locks | Starvation via trylock | Consistent global ordering |
| 10 | 3+ | High complexity | Multiple partial cycles | Total ordering covering all |

---

## Universal Pattern Across All 10 Cases

Despite surface differences, all 10 cases demonstrate that:

1. **Circular wait is the root cause** in most cases (8 out of 10)
2. **Global lock ordering is the universal solution** (effective for all 10)
3. **Other Coffman conditions matter** but breaking condition 4 is most practical
4. **Architectural patterns help** (spooler daemon) but ordering is fundamental

The final principle:

> **Define a total order on all resources. Have all threads acquire in this order. Deadlock becomes impossible.**

