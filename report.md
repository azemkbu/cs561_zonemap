## Zone Maps Benchmarking results


<p align="center">
  <img src="/benchmark/tests.png" width="500">
</p>


## Research Questions

### (i) Expected Memory Footprint of the Zone Map

Let:
- **N** be the total number of elements in the dataset
- **M** be the number of elements per zone
- **Z = N / M** be the number of zones
- each zone contains **d** entries
- each entry occupies **b** bytes

Then, the expected memory footprint of the zone map is:

Memory footprint = Z * d * b

---

### (ii) Limited Memory Budget for the Zone Map

If the available memory budget is smaller than the expected memory footprint of the zone map, several strategies can be applied. Each approach involves trade-offs between memory usage and pruning effectiveness.

#### 1) Increase zone size and reduce the number of zones
Increase the number of elements per zone so that the total number of zones decreases.  
This reduces the amount of metadata stored but weakens pruning accuracy, potentially increasing disk I/O during query execution.

#### 2) Reduce the number of entries per zone
If the zone map tracks multiple attributes, keep summaries only for the most frequently queried or most selective columns to reduce memory usage.

#### 3) Use a hierarchical or coarse-grained zone map
Maintain larger zones in memory and store finer-grained summaries on disk or compute them on demand.  
This balances memory constraints with query performance.

Overall, when memory is limited, the system must trade reduced zone-map precision for lower memory usage, potentially incurring additional disk I/O during query processing.
