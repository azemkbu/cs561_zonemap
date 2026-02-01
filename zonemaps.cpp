#include "zonemaps.h"

#include <algorithm>

template<typename T>
zonemap<T>::zonemap(std::vector<T> _elements, uint _num_elements_per_zone)
    : elements(std::move(_elements)),
      zones(),
      num_zones(0),
      num_elements_per_zone(_num_elements_per_zone)
{
    // Build zonemap metadata (min/max per zone) once at construction.
    build();
}

template<typename T>
void zonemap<T>::build()
{

    // Partition the input into fixed-size zones and compute per-zone [min, max].
    // These bounds allow us to skip entire zones that cannot match a query.

    zones.clear();
    num_zones = 0;

    const size_t n = elements.size();
    if (n == 0) return;

    // Create zones of at most num_elements_per_zone elements.
    for (size_t start = 0; start < n; start += static_cast<size_t>(num_elements_per_zone))
    {
        const size_t end = std::min(start + static_cast<size_t>(num_elements_per_zone), n);

        zone<T> z;

        // Store the zone's raw elements
        z.elements.assign(elements.begin() + start, elements.begin() + end);
        z.size = static_cast<uint>(z.elements.size());

        // Compute zone min/max metadata
        T min = z.elements[0];
        T max = z.elements[0];

        for (const T& element : z.elements) {
            if (element < min) min = element;
            if (max < element) max = element;
        }

        z.min = min;
        z.max = max;

        zones.push_back(std::move(z));
    }

    num_zones = static_cast<uint>(zones.size());
}

template<typename T>
void zonemap<T>::sort_elements()
{
    std::sort(elements.begin(), elements.end());
    build();
}

template<typename T>
size_t zonemap<T>::query(T key)
{
    size_t count = 0;
    // Point query use min and max to prune zones
    // scan only candidate zones
    // Returns count to support duplicates in the data

    for (const auto& z : zones){
        if (key < z.min || key > z.max) continue;

        for (const auto& v : z.elements) {
            if (v == key) ++count;
        }
    }

    return count;
}

template<typename T>
std::vector<T> zonemap<T>::query(T low, T high)
{
    // Range query returning values
    // prune zones whose ranges do not overlap with low and high
    std::vector<T> result;

    for (const auto& z : zones)
    {
        if (z.max < low || z.min > high) continue;

        for (const auto& v : z.elements) {
            if (v >= low && v <= high) {
                result.push_back(v);
            }
        }
    }

    return result;
}

template<typename T>
size_t zonemap<T>::range_query_count(T low, T high) {
    // Range query count variant
    // same pruning as above but avoids materializing output
    size_t count = 0;

    for (const auto& z : zones) {
        if (z.max < low || z.min > high) continue;

        for (const auto& v : z.elements) {
            if (v >= low && v <= high) ++count;
        }
    }
    return count;
}

