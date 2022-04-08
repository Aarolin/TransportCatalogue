#pragma once

#include <cmath>

namespace geo {

    struct Coordinates {
        Coordinates() = default;
        Coordinates(double latitude, double longitude) : lat(latitude), lng(longitude) {

        }
        bool operator<(const Coordinates& rhs) const;
        double lat;
        double lng;
    };

    double ComputeDistance(Coordinates from, Coordinates to);

}

