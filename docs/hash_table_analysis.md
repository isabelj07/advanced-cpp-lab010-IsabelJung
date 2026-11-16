# Hash Table Analysis Laboratory

## Core goals
Students must implement a hash table that stores key-value pairs using open addressing and quadratic probing.

## Required behaviors
Students must implement:

- insertion with collision handling;
- lookup and retrieval;
- update of existing entries;
- deletion while preserving probe correctness;
- capacity growth and rehashing;
- hash function design;
- complex-key scenarios with many collisions.

## Required analysis
Students should write a short comparison and complexity analysis of:

- average-case insertion, lookup, and removal;
- best-case and worst-case behavior;
- collision resolution strategies;
- load factor and rehashing costs;
- open addressing vs. chaining;
- how clustering can affect performance.

## Big-O expectations
Students should identify and explain the Big-Oh for:

- `insert`
- `remove`
- `get`
- `update`
- `rehash`
- `contains`

Expected average-case: O(1) for insert, lookup, and update under a reasonable load factor; worst case can degrade to O(n) when many collisions occur or the table becomes badly loaded.

## Analysis

My hash table uses open addressing with quadratic probing, an FNV-1a hash function, and tombstones for deletes. Each key hashes to a home slot, and if that slot is taken the table checks home + 1, home + 4, home + 9 and so on (mod the capacity) until it finds the key or an empty slot. Insert, get, contains, update and remove all run that same probe, so they all cost O(1) on average when the load factor stays low, because the expected number of probes depends on the load factor and not on n. The best case is O(1) when the key sits in its home slot. The worst case is O(n), which happens when lots of keys collide or the hash function is bad, since then every lookup walks a long probe chain. The load factor (size divided by capacity) is what controls performance. Probe counts stay small at low loads but grow quickly as the table fills, so I rehash whenever an insert would push the load factor over 0.5. With a prime capacity and a load factor at or below 0.5, quadratic probing is guaranteed to find a free slot. Rehashing moves every live entry into a new table that is about twice as big (the next prime after 2 * capacity + 1), so a single rehash costs O(n). Because the capacity doubles each time, rehashing only happens about log n times over n inserts, so insert is still O(1) amortized. Rehashing also throws away tombstones, so deleted slots don't build up forever. Deleting has to leave a tombstone instead of just emptying the slot, because an empty slot would cut the probe chain and make keys stored after it impossible to find. Probing skips tombstones, and when a deleted key gets inserted again, it reuses its own tombstone. Quadratic probing is better than linear probing (checking the next slot each time) because linear probing creates primary clustering, where long runs of filled slots keep getting longer. Quadratic probing jumps farther each step, which breaks those runs up, but keys with the same home slot still follow the exact same path (secondary clustering). Double hashing would fix that too, at the cost of a second hash function. Compared to chaining, where each slot holds a linked list, open addressing keeps everything in one array, so it has no per-node pointers and is friendlier to the CPU cache. The downsides are that it needs tombstones, has to keep the load factor low, and gets slow quickly once the table is close to full. Chaining handles high load factors and deletes more easily, but it pays for that with a memory allocation for every entry and with pointer chasing on every lookup. A poor hash function or a high load factor ruins either approach, because when keys pile up in the same few slots, every operation turns into a linear search and drops to O(n). That is why the hash should spread keys evenly and the table should grow before it gets crowded. Finally, keys() and values() scan the whole array, which takes O(capacity) time and O(n) memory for the result.
