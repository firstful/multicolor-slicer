#ifndef libslic3r_Triangulation_hpp_
#define libslic3r_Triangulation_hpp_

#include <vector>
#include <libslic3r/Point.hpp>
#include <libslic3r/Polygon.hpp>

namespace Slic3r {

/// Triangulation of a simple (hole-free) polygon.
///
/// The upstream implementation was based on CGAL's
/// Constrained_Delaunay_triangulation_2. CGAL is a heavy dependency that was
/// only needed for the wipe-tower rib/brim preview mesh, so it was replaced
/// with a self-contained ear-clipping triangulator. The contract is unchanged:
/// the returned triangle indices address the input polygon's points and are
/// wound the same way as the input polygon.
class Triangulation
{
public:
    Triangulation() = delete;

    using Indices = std::vector<Vec3i32>;

    /// <summary>
    /// Connect the points of a simple polygon by triangulation to create a
    /// filled surface of triangles.
    /// </summary>
    /// <param name="polygon">Simple polygon (no holes).</param>
    /// <returns>Triangles, as indices into polygon.points.</returns>
    static Indices triangulate(const Polygon &polygon);
};

} // namespace Slic3r
#endif // libslic3r_Triangulation_hpp_
