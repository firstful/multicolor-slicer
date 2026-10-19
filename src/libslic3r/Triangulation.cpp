// Triangulation: dependency-free polygon triangulation.
//
// The upstream implementation used CGAL's Constrained_Delaunay_triangulation_2.
// CGAL is a heavy dependency (it pulls in Boost.Geometry, GMP and MPFR) that is
// only needed for the wipe-tower rib/brim preview mesh, so it was replaced with
// the Mapbox earcut triangulator that is already vendored in this tree
// (src/earcut, MIT licensed).
//
// The public contract is unchanged: triangulate() returns triangle indices into
// the input point array, wound the same way as the input polygon (a CCW
// polygon yields CCW triangles, exactly like the constrained Delaunay
// triangulation did).

#include "Triangulation.hpp"

#include <algorithm>

#include <earcut/earcut.hpp>

#include "libslic3r/Point.hpp"
#include "libslic3r/Polygon.hpp"

namespace Slic3r {

namespace {

// Twice the signed area of triangle (a, b, c). Exact for integer coordinates.
inline int64_t cross2(const Point &a, const Point &b, const Point &c)
{
    return int64_t(b.x() - a.x()) * int64_t(c.y() - a.y())
         - int64_t(b.y() - a.y()) * int64_t(c.x() - a.x());
}

} // namespace

Triangulation::Indices Triangulation::triangulate(const Polygon &polygon)
{
    Indices indices;

    const Points &src = polygon.points;
    if (src.size() < 3)
        return indices;

    // earcut works on rings of doubles and returns indices into the flattened
    // vertex list. A single ring is used here: Polygon is a simple polygon
    // without holes.
    std::vector<std::vector<double>> ring;
    ring.reserve(1);
    ring.emplace_back();
    ring.back().reserve(src.size() * 2);
    for (const Point &p : src) {
        ring.back().emplace_back(double(p.x()));
        ring.back().emplace_back(double(p.y()));
    }

    const std::vector<uint32_t> tri = mapbox::earcut<uint32_t>(ring);
    if (tri.size() < 3)
        return indices;

    indices.reserve(tri.size() / 3);
    for (size_t i = 0; i + 2 < tri.size(); i += 3)
        indices.emplace_back(int32_t(tri[i]), int32_t(tri[i + 1]), int32_t(tri[i + 2]));

    // earcut normalizes the winding of the ring it is given, so the triangle
    // winding may not match the input polygon. Restore the input's winding so
    // that callers relying on the orientation (e.g. the wipe tower rib) keep
    // working: compare the signed area of the first triangle against the
    // signed area of the polygon and flip all triangles if they disagree.
    if (! indices.empty()) {
        const Vec3i32 &t = indices.front();
        const int64_t tri_area2 = cross2(src[size_t(t[0])], src[size_t(t[1])], src[size_t(t[2])]);
        int64_t poly_area2 = 0;
        for (size_t i = 0, n = src.size(); i < n; ++i) {
            const Point &p = src[i];
            const Point &q = src[(i + 1) % n];
            poly_area2 += int64_t(p.x()) * int64_t(q.y()) - int64_t(q.x()) * int64_t(p.y());
        }
        if (tri_area2 != 0 && poly_area2 != 0 && ((tri_area2 > 0) != (poly_area2 > 0)))
            for (Vec3i32 &t2 : indices)
                std::swap(t2[0], t2[1]);
    }

    return indices;
}

} // namespace Slic3r
