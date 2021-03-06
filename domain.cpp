#include "domain.h"

bool Stop::operator<(const Stop& rhs) const {
    return coordinates < rhs.coordinates;
}

size_t StopsHasher::operator()(std::pair<Stop*, Stop*> stops) const {
    size_t stop_from_hash = ptr_hasher_(stops.first);
    size_t stop_to_hash = ptr_hasher_(stops.second);
    size_t result_hash = stop_from_hash + stop_to_hash * 74;
    return result_hash;
}

bool WayInfo::operator<(const WayInfo& rhs) const {
    return weight < rhs.weight;
}

bool WayInfo::operator>(const WayInfo& rhs) const {
    return !(weight < rhs.weight);
}

WayInfo WayInfo::operator+(const WayInfo& rhs) const {
    return { bus_name, stop_count, weight + rhs.weight };
}