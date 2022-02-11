#pragma once

#include <cmath>

struct Coordinates {
    Coordinates() = default;
    Coordinates(double latitude, double longitude) : lat(latitude), lng(longitude) {

    }
    bool operator<(const Coordinates& rhs) const {
        return lat < rhs.lat&& lng < rhs.lng;
    }
    double lat;
    double lng;
};

inline double ComputeDistance(Coordinates from, Coordinates to) {
    using namespace std;
    static const double dr = 3.1415926535 / 180.;
    return acos(sin(from.lat * dr) * sin(to.lat * dr)
        + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr))
        * 6371000;
}
