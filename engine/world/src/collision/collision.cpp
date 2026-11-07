/**
 * @file collision.cpp
 * @brief Collision detection system implementation.
 *
 * Production Status: M1 World Foundation (Production)
 */

#include "poko/world/collision.h"
#include "poko/world/entity.h"
#include "poko/world/scene.h"
#include "poko/world/transform.h"
#include "poko/math/aabb.h"
#include "poko/math/sphere.h"
#include "poko/math/capsule.h"
#include "poko/math/obb.h"
#include "poko/math/ray.h"
#include "poko/math/vec3.h"
#include "poko/math/mat4.h"
#include "poko/math/constants.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace poko::world {

namespace {

constexpr float kRayEpsilon = 1e-6f;

// ---------------------------------------------------------------------------
// World-space shape representation
// ---------------------------------------------------------------------------

struct WorldShape {
    CollisionShape type = CollisionShape::AABB;
    math::AABB aabb;
    math::Sphere sphere;
    math::Capsule capsule;
    math::OBB obb;
};

math::Mat4 collider_world_matrix(const Collision* collider) {
    if (collider && collider->get_entity()) {
        if (const Transform* transform = collider->get_entity()->get_transform()) {
            return transform->get_world_matrix();
        }
    }
    return math::Mat4::identity();
}

// Largest basis-column length of a matrix; used to scale radii under
// (possibly non-uniform) transform scale.
float max_basis_scale(const math::Mat4& m) {
    float sx = math::Vec3(m.m[0], m.m[1], m.m[2]).length();
    float sy = math::Vec3(m.m[4], m.m[5], m.m[6]).length();
    float sz = math::Vec3(m.m[8], m.m[9], m.m[10]).length();
    return std::max(sx, std::max(sy, sz));
}

math::AABB transform_aabb(const math::AABB& box, const math::Mat4& m) {
    math::AABB result;
    const float inf = std::numeric_limits<float>::max();
    result.min = math::Vec3(inf, inf, inf);
    result.max = math::Vec3(-inf, -inf, -inf);
    for (int i = 0; i < 8; ++i) {
        math::Vec3 corner(
            (i & 1) ? box.max.x : box.min.x,
            (i & 2) ? box.max.y : box.min.y,
            (i & 4) ? box.max.z : box.min.z);
        math::Vec4 w = m.transform_point(corner);
        math::Vec3 p(w.x, w.y, w.z);
        result.expand(p);
    }
    return result;
}

WorldShape transform_shape_to_world(CollisionShape type, const math::AABB& aabb,
                                    const math::Sphere& sphere, const math::Capsule& capsule,
                                    const math::OBB& obb, const math::Mat4& m) {
    WorldShape out;
    out.type = type;
    switch (type) {
        case CollisionShape::AABB:
            out.aabb = transform_aabb(aabb, m);
            break;
        case CollisionShape::Sphere: {
            math::Vec4 c = m.transform_point(sphere.center);
            out.sphere.center = math::Vec3(c.x, c.y, c.z);
            out.sphere.radius = sphere.radius * max_basis_scale(m);
            break;
        }
        case CollisionShape::Capsule: {
            math::Vec4 s = m.transform_point(capsule.start);
            math::Vec4 e = m.transform_point(capsule.end);
            out.capsule.start = math::Vec3(s.x, s.y, s.z);
            out.capsule.end = math::Vec3(e.x, e.y, e.z);
            out.capsule.radius = capsule.radius * max_basis_scale(m);
            break;
        }
        case CollisionShape::OBB: {
            math::Vec4 c = m.transform_point(obb.center);
            out.obb.center = math::Vec3(c.x, c.y, c.z);
            // Basis columns give the world-space axes and per-axis scale.
            const float oe[3] = {obb.extents.x, obb.extents.y, obb.extents.z};
            for (int i = 0; i < 3; ++i) {
                math::Vec3 axis(m.m[i * 4 + 0], m.m[i * 4 + 1], m.m[i * 4 + 2]);
                float len = axis.length();
                out.obb.axes[i] = (len > math::EPSILON) ? (axis / len) : obb.axes[i];
                const float scaled = oe[i] * len;
                if (i == 0) out.obb.extents.x = scaled;
                else if (i == 1) out.obb.extents.y = scaled;
                else out.obb.extents.z = scaled;
            }
            break;
        }
    }
    return out;
}

WorldShape get_world_shape(const Collision* collider) {
    const math::Mat4 m = collider_world_matrix(collider);
    switch (collider->get_shape_type()) {
        case CollisionShape::AABB:
            return transform_shape_to_world(CollisionShape::AABB, collider->get_aabb(),
                                            math::Sphere(), math::Capsule(), math::OBB(), m);
        case CollisionShape::Sphere:
            return transform_shape_to_world(CollisionShape::Sphere, math::AABB(),
                                            collider->get_sphere(), math::Capsule(), math::OBB(), m);
        case CollisionShape::Capsule:
            return transform_shape_to_world(CollisionShape::Capsule, math::AABB(),
                                            math::Sphere(), collider->get_capsule(), math::OBB(), m);
        case CollisionShape::OBB:
            return transform_shape_to_world(CollisionShape::OBB, math::AABB(),
                                            math::Sphere(), math::Capsule(), collider->get_obb(), m);
    }
    return WorldShape{};
}

math::AABB shape_world_bounds(const WorldShape& shape) {
    switch (shape.type) {
        case CollisionShape::AABB:
            return shape.aabb;
        case CollisionShape::Sphere: {
            float r = shape.sphere.radius;
            return math::AABB::from_center_extents(shape.sphere.center, math::Vec3(r, r, r));
        }
        case CollisionShape::Capsule: {
            math::Vec3 lo = math::Vec3(std::min(shape.capsule.start.x, shape.capsule.end.x),
                                       std::min(shape.capsule.start.y, shape.capsule.end.y),
                                       std::min(shape.capsule.start.z, shape.capsule.end.z));
            math::Vec3 hi = math::Vec3(std::max(shape.capsule.start.x, shape.capsule.end.x),
                                       std::max(shape.capsule.start.y, shape.capsule.end.y),
                                       std::max(shape.capsule.start.z, shape.capsule.end.z));
            float cr = shape.capsule.radius;
            math::Vec3 r(cr, cr, cr);
            return math::AABB(lo - r, hi + r);
        }
        case CollisionShape::OBB: {
            // World AABB of an OBB: center +/- sum of |axis_i| * extent_i
            math::Vec3 e(std::abs(shape.obb.axes[0].x) * shape.obb.extents.x +
                         std::abs(shape.obb.axes[1].x) * shape.obb.extents.y +
                         std::abs(shape.obb.axes[2].x) * shape.obb.extents.z,
                         std::abs(shape.obb.axes[0].y) * shape.obb.extents.x +
                         std::abs(shape.obb.axes[1].y) * shape.obb.extents.y +
                         std::abs(shape.obb.axes[2].y) * shape.obb.extents.z,
                         std::abs(shape.obb.axes[0].z) * shape.obb.extents.x +
                         std::abs(shape.obb.axes[1].z) * shape.obb.extents.y +
                         std::abs(shape.obb.axes[2].z) * shape.obb.extents.z);
            return math::AABB::from_center_extents(shape.obb.center, e);
        }
    }
    return math::AABB();
}

// ---------------------------------------------------------------------------
// Primitive intersection tests (all in world space)
// ---------------------------------------------------------------------------

math::OBB obb_from_aabb(const math::AABB& box) {
    math::OBB obb(math::Vec3((box.min.x + box.max.x) * 0.5f,
                             (box.min.y + box.max.y) * 0.5f,
                             (box.min.z + box.max.z) * 0.5f),
                  math::Vec3((box.max.x - box.min.x) * 0.5f,
                             (box.max.y - box.min.y) * 0.5f,
                             (box.max.z - box.min.z) * 0.5f),
                  math::Mat4::identity());
    return obb;
}

bool obb_intersects_obb(const math::OBB& obb1, const math::OBB& obb2) {
    // SAT: 3 axes from each box + 9 cross-product axes.
    math::Vec3 t = obb2.center - obb1.center;
    const math::Vec3 axes1[3] = {obb1.axes[0], obb1.axes[1], obb1.axes[2]};
    const math::Vec3 axes2[3] = {obb2.axes[0], obb2.axes[1], obb2.axes[2]};

    auto overlaps_on_axis = [&](const math::Vec3& axis) {
        if (axis.length_squared() < kRayEpsilon) {
            return true;  // Degenerate axis cannot separate
        }
        math::Vec3 a = axis.normalized();
        float ra = obb1.extents.x * std::abs(a.dot(axes1[0])) +
                   obb1.extents.y * std::abs(a.dot(axes1[1])) +
                   obb1.extents.z * std::abs(a.dot(axes1[2]));
        float rb = obb2.extents.x * std::abs(a.dot(axes2[0])) +
                   obb2.extents.y * std::abs(a.dot(axes2[1])) +
                   obb2.extents.z * std::abs(a.dot(axes2[2]));
        return std::abs(t.dot(a)) <= ra + rb;
    };

    for (int i = 0; i < 3; ++i) {
        if (!overlaps_on_axis(axes1[i]) || !overlaps_on_axis(axes2[i])) {
            return false;
        }
    }
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            if (!overlaps_on_axis(axes1[i].cross(axes2[j]))) {
                return false;
            }
        }
    }
    return true;
}

// Exact squared distance between a segment and an AABB. The distance is a
// piecewise quadratic in the segment parameter; breakpoints are the values of
// t where the segment crosses any box face. On each interval the set of
// "outside" axes is constant, so the minimum is found analytically.
float segment_aabb_distance_sq(const math::Vec3& s, const math::Vec3& e, const math::AABB& box) {
    math::Vec3 d = e - s;
    const float sv[3] = {s.x, s.y, s.z};
    const float dv[3] = {d.x, d.y, d.z};
    const float mn[3] = {box.min.x, box.min.y, box.min.z};
    const float mx[3] = {box.max.x, box.max.y, box.max.z};

    // Collect all breakpoints in (0, 1).
    float ts[16];
    int n = 0;
    ts[n++] = 0.0f;
    ts[n++] = 1.0f;
    for (int i = 0; i < 3; ++i) {
        if (std::abs(dv[i]) < kRayEpsilon) {
            continue;
        }
        float inv = 1.0f / dv[i];
        float t0 = (mn[i] - sv[i]) * inv;
        float t1 = (mx[i] - sv[i]) * inv;
        for (float t : {t0, t1}) {
            if (t > 0.0f && t < 1.0f) {
                ts[n++] = t;
            }
        }
    }
    std::sort(ts, ts + n);
    n = static_cast<int>(std::unique(ts, ts + n) - ts);

    float best = std::numeric_limits<float>::max();
    for (int k = 0; k + 1 < n; ++k) {
        float t0 = ts[k];
        float t1 = ts[k + 1];
        if (t1 - t0 < math::EPSILON) {
            continue;
        }
        float tm = (t0 + t1) * 0.5f;

        // Determine which axes are outside the box within this interval and
        // build the quadratic f(t) = A t^2 + B t + C for the squared gap.
        double A = 0.0, B = 0.0, C = 0.0;
        for (int i = 0; i < 3; ++i) {
            float val = sv[i] + dv[i] * tm;
            float bound;
            if (val < mn[i]) {
                bound = mn[i];
            } else if (val > mx[i]) {
                bound = mx[i];
            } else {
                continue;  // Inside this axis's slab
            }
            double a = static_cast<double>(dv[i]);
            double b = static_cast<double>(sv[i]) - bound;
            A += a * a;
            B += 2.0 * a * b;
            C += b * b;
        }

        auto eval = [&](float t) {
            return static_cast<float>(A * t * t + B * t + C);
        };

        best = std::min(best, eval(t0));
        best = std::min(best, eval(t1));
        if (A > 1e-12) {
            double tv = -B / (2.0 * A);
            if (tv > t0 && tv < t1) {
                best = std::min(best, eval(static_cast<float>(tv)));
            }
        }
    }
    return best;
}

// Closest points between two segments; returns squared distance.
// s/t are the segment parameters in [0,1].
float segment_segment_distance_sq(const math::Vec3& p1, const math::Vec3& q1,
                                  const math::Vec3& p2, const math::Vec3& q2) {
    math::Vec3 d1 = q1 - p1;
    math::Vec3 d2 = q2 - p2;
    math::Vec3 r = p1 - p2;

    float a = d1.dot(d1);
    float e = d2.dot(d2);
    float f = d2.dot(r);
    float c = d1.dot(r);
    float b = d1.dot(d2);
    float denom = a * e - b * b;

    float s, t;
    if (denom > math::EPSILON_SQ) {
        s = std::clamp((b * f - c * e) / denom, 0.0f, 1.0f);
    } else {
        s = 0.0f;
    }
    if (e > math::EPSILON_SQ) {
        t = std::clamp((b * s + f) / e, 0.0f, 1.0f);
    } else {
        t = 0.0f;
    }
    if (a > math::EPSILON_SQ) {
        s = std::clamp((b * t - c) / a, 0.0f, 1.0f);
    }

    math::Vec3 c1 = p1 + d1 * s;
    math::Vec3 c2 = p2 + d2 * t;
    return (c1 - c2).length_squared();
}

bool sphere_intersects_obb(const math::Sphere& sphere, const math::OBB& obb) {
    math::Vec3 local(sphere.center.x - obb.center.x,
                     sphere.center.y - obb.center.y,
                     sphere.center.z - obb.center.z);
    math::Vec3 l(local.dot(obb.axes[0]), local.dot(obb.axes[1]), local.dot(obb.axes[2]));
    math::Vec3 closest(std::clamp(l.x, -obb.extents.x, obb.extents.x),
                       std::clamp(l.y, -obb.extents.y, obb.extents.y),
                       std::clamp(l.z, -obb.extents.z, obb.extents.z));
    math::Vec3 diff = l - closest;
    return diff.length_squared() <= sphere.radius * sphere.radius;
}

// Capsule vs OBB: transform the capsule segment into the OBB's local frame,
// where the box becomes an AABB, then measure segment-to-box distance.
bool capsule_intersects_obb(const math::Capsule& capsule, const math::OBB& obb) {
    auto to_local = [&](const math::Vec3& p) {
        math::Vec3 d = p - obb.center;
        return math::Vec3(d.dot(obb.axes[0]), d.dot(obb.axes[1]), d.dot(obb.axes[2]));
    };
    math::Vec3 ls = to_local(capsule.start);
    math::Vec3 le = to_local(capsule.end);
    math::AABB local_box(math::Vec3(-obb.extents.x, -obb.extents.y, -obb.extents.z),
                         math::Vec3(obb.extents.x, obb.extents.y, obb.extents.z));
    return segment_aabb_distance_sq(ls, le, local_box) <=
           capsule.radius * capsule.radius;
}

bool capsule_intersects_aabb(const math::Capsule& capsule, const math::AABB& box) {
    return segment_aabb_distance_sq(capsule.start, capsule.end, box) <=
           capsule.radius * capsule.radius;
}

bool capsule_intersects_capsule(const math::Capsule& c1, const math::Capsule& c2) {
    float rsum = c1.radius + c2.radius;
    return segment_segment_distance_sq(c1.start, c1.end, c2.start, c2.end) <= rsum * rsum;
}

bool shapes_intersect(const WorldShape& a, const WorldShape& b) {
    // Normalize order for symmetric pairs via type rank.
    auto rank = [](CollisionShape t) {
        switch (t) {
            case CollisionShape::AABB: return 0;
            case CollisionShape::Sphere: return 1;
            case CollisionShape::Capsule: return 2;
            case CollisionShape::OBB: return 3;
        }
        return 0;
    };
    const WorldShape* x = &a;
    const WorldShape* y = &b;
    if (rank(b.type) < rank(a.type)) {
        std::swap(x, y);
    }

    switch (x->type) {
        case CollisionShape::AABB:
            switch (y->type) {
                case CollisionShape::AABB: return x->aabb.intersects(y->aabb);
                case CollisionShape::Sphere: return y->sphere.intersects_aabb(x->aabb);
                case CollisionShape::Capsule: return capsule_intersects_aabb(y->capsule, x->aabb);
                case CollisionShape::OBB: return obb_intersects_obb(obb_from_aabb(x->aabb), y->obb);
            }
            break;
        case CollisionShape::Sphere:
            switch (y->type) {
                case CollisionShape::Sphere: return x->sphere.intersects(y->sphere);
                case CollisionShape::Capsule: return y->capsule.intersects_sphere(x->sphere);
                case CollisionShape::OBB: return sphere_intersects_obb(x->sphere, y->obb);
                default: return false;
            }
            break;
        case CollisionShape::Capsule:
            switch (y->type) {
                case CollisionShape::Capsule: return capsule_intersects_capsule(x->capsule, y->capsule);
                case CollisionShape::OBB: return capsule_intersects_obb(x->capsule, y->obb);
                default: return false;
            }
            break;
        case CollisionShape::OBB:
            return obb_intersects_obb(x->obb, y->obb);
    }
    return false;
}

// ---------------------------------------------------------------------------
// Raycasts (world space)
// ---------------------------------------------------------------------------

// Slab-method ray vs AABB. Returns hit distance or -1.
float ray_aabb_distance(const math::Ray& ray, const math::AABB& box, float max_distance) {
    float t_min = 0.0f;
    float t_max = max_distance;
    const float o[3] = {ray.origin.x, ray.origin.y, ray.origin.z};
    const float d[3] = {ray.direction.x, ray.direction.y, ray.direction.z};
    const float mn[3] = {box.min.x, box.min.y, box.min.z};
    const float mx[3] = {box.max.x, box.max.y, box.max.z};

    for (int i = 0; i < 3; ++i) {
        if (std::abs(d[i]) < kRayEpsilon) {
            if (o[i] < mn[i] || o[i] > mx[i]) {
                return -1.0f;
            }
            continue;
        }
        float inv = 1.0f / d[i];
        float t0 = (mn[i] - o[i]) * inv;
        float t1 = (mx[i] - o[i]) * inv;
        if (inv < 0.0f) {
            std::swap(t0, t1);
        }
        t_min = std::max(t_min, t0);
        t_max = std::min(t_max, t1);
        if (t_max < t_min) {
            return -1.0f;
        }
    }
    return (t_min >= 0.0f && t_min <= max_distance) ? t_min : -1.0f;
}

float ray_sphere_distance(const math::Ray& ray, const math::Sphere& sphere, float max_distance) {
    math::Vec3 oc = ray.origin - sphere.center;
    float b = oc.dot(ray.direction);
    float c = oc.length_squared() - sphere.radius * sphere.radius;
    float disc = b * b - c;  // direction is normalized: a = 1
    if (disc < 0.0f) {
        return -1.0f;
    }
    float root = std::sqrt(disc);
    float t = -b - root;
    if (t < 0.0f) {
        t = -b + root;  // Origin inside sphere: exit point
    }
    return (t >= 0.0f && t <= max_distance) ? t : -1.0f;
}

// Ray vs finite cylinder around a segment, plus the two end-cap spheres.
// Returns the earliest hit distance or -1.
float ray_capsule_distance(const math::Ray& ray, const math::Capsule& capsule, float max_distance) {
    math::Vec3 axis = capsule.end - capsule.start;
    float axis_len = axis.length();
    if (axis_len < math::EPSILON) {
        // Degenerate capsule: just a sphere
        return ray_sphere_distance(ray, math::Sphere(capsule.start, capsule.radius), max_distance);
    }
    math::Vec3 an = axis / axis_len;
    math::Vec3 mid = (capsule.start + capsule.end) * 0.5f;
    float half_len = axis_len * 0.5f;

    // Build an orthonormal basis (an, u, v) for the capsule's local frame.
    math::Vec3 ref = std::abs(an.y) < 0.9f ? math::Vec3::up() : math::Vec3::right();
    math::Vec3 u = an.cross(ref).normalized();
    math::Vec3 v = an.cross(u);

    // Transform the ray into the capsule frame.
    math::Vec3 lo = ray.origin - mid;
    math::Vec3 ld(ray.direction.dot(an), ray.direction.dot(u), ray.direction.dot(v));
    math::Vec3 oo(lo.dot(an), lo.dot(u), lo.dot(v));

    float best = max_distance + 1.0f;
    bool found = false;

    // 1) Infinite cylinder around the segment axis, clipped to |an| <= half_len.
    //    Solve |oo_perp + t * ld_perp|^2 = r^2 in the (u,v) plane.
    {
        float ox = oo.y, oy = oo.z;
        float dx = ld.y, dy = ld.z;
        float A = dx * dx + dy * dy;
        if (A > math::EPSILON_SQ) {
            float B = 2.0f * (ox * dx + oy * dy);
            float C = ox * ox + oy * oy - capsule.radius * capsule.radius;
            float disc = B * B - 4.0f * A * C;
            if (disc >= 0.0f) {
                float root = std::sqrt(disc);
                for (float t : {(-B - root) / (2.0f * A), (-B + root) / (2.0f * A)}) {
                    if (t >= 0.0f && t <= max_distance && t < best) {
                        float axial = oo.x + ld.x * t;
                        if (std::abs(axial) <= half_len) {
                            best = t;
                            found = true;
                        }
                    }
                }
            }
        }
    }

    // 2) End-cap spheres at the segment endpoints.
    for (const math::Vec3& cap : {capsule.start, capsule.end}) {
        float t = ray_sphere_distance(ray, math::Sphere(cap, capsule.radius), max_distance);
        if (t >= 0.0f && t < best) {
            best = t;
            found = true;
        }
    }

    return found ? best : -1.0f;
}

float ray_obb_distance(const math::Ray& ray, const math::OBB& obb, float max_distance) {
    // Transform the ray into the OBB's local frame where it is an AABB.
    auto to_local = [&](const math::Vec3& p) {
        math::Vec3 d = p - obb.center;
        return math::Vec3(d.dot(obb.axes[0]), d.dot(obb.axes[1]), d.dot(obb.axes[2]));
    };
    math::Vec3 lo = to_local(ray.origin);
    math::Vec3 ld(ray.direction.dot(obb.axes[0]),
                  ray.direction.dot(obb.axes[1]),
                  ray.direction.dot(obb.axes[2]));
    math::Ray local_ray(lo, ld);
    return ray_aabb_distance(local_ray,
                             math::AABB(math::Vec3(-obb.extents.x, -obb.extents.y, -obb.extents.z),
                                        math::Vec3(obb.extents.x, obb.extents.y, obb.extents.z)),
                             max_distance);
}

} // namespace

// ---------------------------------------------------------------------------
// Collision component
// ---------------------------------------------------------------------------

Collision::Collision() : shape_(CollisionShape::AABB) {
    initialize();
}

Collision::~Collision() = default;

void Collision::initialize() {
    type_ = CollisionType::Solid;
    shape_ = CollisionShape::AABB;
    aabb_ = math::AABB();
    sphere_ = math::Sphere();
    capsule_ = math::Capsule();
    obb_ = math::OBB();
    collision_layers_ = static_cast<uint32_t>(CollisionLayer::Default);
    collision_mask_ = static_cast<uint32_t>(CollisionLayer::All);
    entity_ = nullptr;
}

void Collision::set_type(CollisionType type) {
    type_ = type;
}

CollisionType Collision::get_type() const {
    return type_;
}

void Collision::set_aabb(const math::AABB& aabb) {
    aabb_ = aabb;
    shape_ = CollisionShape::AABB;
}

const math::AABB& Collision::get_aabb() const {
    return aabb_;
}

void Collision::set_sphere(const math::Sphere& sphere) {
    sphere_ = sphere;
    shape_ = CollisionShape::Sphere;
}

const math::Sphere& Collision::get_sphere() const {
    return sphere_;
}

void Collision::set_capsule(const math::Capsule& capsule) {
    capsule_ = capsule;
    shape_ = CollisionShape::Capsule;
}

const math::Capsule& Collision::get_capsule() const {
    return capsule_;
}

void Collision::set_obb(const math::OBB& obb) {
    obb_ = obb;
    shape_ = CollisionShape::OBB;
}

const math::OBB& Collision::get_obb() const {
    return obb_;
}

CollisionShape Collision::get_shape_type() const {
    return shape_;
}

void Collision::set_collision_layers(uint32_t layers) {
    collision_layers_ = layers;
}

uint32_t Collision::get_collision_layers() const {
    return collision_layers_;
}

void Collision::set_collision_mask(uint32_t mask) {
    collision_mask_ = mask;
}

uint32_t Collision::get_collision_mask() const {
    return collision_mask_;
}

Entity* Collision::get_entity() const {
    return entity_;
}

void Collision::on_add(Entity* entity) {
    entity_ = entity;
}

void Collision::on_remove(Entity* entity) {
    if (entity_ == entity) {
        entity_ = nullptr;
    }
}

bool Collision::check_collision(const Entity* other) const {
    if (!other) {
        return false;
    }

    const Collision* other_collision = other->get_component<Collision>();
    if (!other_collision) {
        return false;
    }

    // Broad phase on world-space bounds, then exact narrow-phase test.
    if (!get_world_bounds().intersects(other_collision->get_world_bounds())) {
        return false;
    }

    WorldShape a = get_world_shape(this);
    WorldShape b = get_world_shape(other_collision);
    return shapes_intersect(a, b);
}

bool Collision::check_collision_filtered(const Entity* other) const {
    if (!other) {
        return false;
    }

    const Collision* other_collision = other->get_component<Collision>();
    if (!other_collision) {
        return false;
    }

    // Check collision layers/masks (symmetric test)
    if ((collision_layers_ & other_collision->collision_mask_) == 0 ||
        (other_collision->collision_layers_ & collision_mask_) == 0) {
        return false;
    }

    return check_collision(other);
}

float Collision::raycast_distance(const math::Ray& ray, float max_distance) const {
    WorldShape shape = get_world_shape(this);
    switch (shape.type) {
        case CollisionShape::AABB:
            return ray_aabb_distance(ray, shape.aabb, max_distance);
        case CollisionShape::Sphere:
            return ray_sphere_distance(ray, shape.sphere, max_distance);
        case CollisionShape::Capsule:
            return ray_capsule_distance(ray, shape.capsule, max_distance);
        case CollisionShape::OBB:
            return ray_obb_distance(ray, shape.obb, max_distance);
    }
    return -1.0f;
}

bool Collision::raycast(const math::Ray& ray, float max_distance) const {
    return raycast_distance(ray, max_distance) >= 0.0f;
}

math::AABB Collision::get_world_bounds() const {
    return shape_world_bounds(get_world_shape(this));
}

CollisionResult check_entity_collision(const Entity* entity1, const Entity* entity2) {
    CollisionResult result;

    if (!entity1 || !entity2) {
        return result;
    }

    const Collision* collision1 = entity1->get_component<Collision>();
    const Collision* collision2 = entity2->get_component<Collision>();

    if (!collision1 || !collision2) {
        return result;
    }

    WorldShape shape1 = get_world_shape(collision1);
    WorldShape shape2 = get_world_shape(collision2);

    math::AABB bounds1 = shape_world_bounds(shape1);
    math::AABB bounds2 = shape_world_bounds(shape2);

    // Broad phase: world bounds must overlap
    if (!bounds1.intersects(bounds2)) {
        return result;
    }

    result.collided = shapes_intersect(shape1, shape2);
    if (!result.collided) {
        return result;
    }

    // Contact info. Sphere-sphere gets an exact result; other pairs use the
    // overlap of the world bounds, which is a conservative estimate.
    if (shape1.type == CollisionShape::Sphere && shape2.type == CollisionShape::Sphere) {
        math::Vec3 delta = shape2.sphere.center - shape1.sphere.center;
        float dist = delta.length();
        float rsum = shape1.sphere.radius + shape2.sphere.radius;
        if (dist > math::EPSILON) {
            result.normal = delta / dist;
        } else {
            result.normal = math::Vec3::up();
        }
        result.penetration_depth = rsum - dist;
        result.contact_point = shape1.sphere.center + result.normal *
                               (shape1.sphere.radius - result.penetration_depth * 0.5f);
        return result;
    }

    // Overlap interval per axis between the two world bounds
    float overlaps[3];
    int min_axis = 0;
    for (int i = 0; i < 3; ++i) {
        const float b1min[3] = {bounds1.min.x, bounds1.min.y, bounds1.min.z};
        const float b1max[3] = {bounds1.max.x, bounds1.max.y, bounds1.max.z};
        const float b2min[3] = {bounds2.min.x, bounds2.min.y, bounds2.min.z};
        const float b2max[3] = {bounds2.max.x, bounds2.max.y, bounds2.max.z};
        overlaps[i] = std::min(b1max[i], b2max[i]) - std::max(b1min[i], b2min[i]);
        if (overlaps[i] < overlaps[min_axis]) {
            min_axis = i;
        }
    }

    result.penetration_depth = std::max(0.0f, overlaps[min_axis]);

    // Contact point: midpoint of the overlapping region on each axis
    const float b1min[3] = {bounds1.min.x, bounds1.min.y, bounds1.min.z};
    const float b1max[3] = {bounds1.max.x, bounds1.max.y, bounds1.max.z};
    const float b2min[3] = {bounds2.min.x, bounds2.min.y, bounds2.min.z};
    const float b2max[3] = {bounds2.max.x, bounds2.max.y, bounds2.max.z};
    float cx = (std::max(b1min[0], b2min[0]) + std::min(b1max[0], b2max[0])) * 0.5f;
    float cy = (std::max(b1min[1], b2min[1]) + std::min(b1max[1], b2max[1])) * 0.5f;
    float cz = (std::max(b1min[2], b2min[2]) + std::min(b1max[2], b2max[2])) * 0.5f;
    result.contact_point = math::Vec3(cx, cy, cz);

    // Normal: direction from entity1's bounds center to entity2's, snapped to
    // the axis of least penetration.
    math::Vec3 c1 = bounds1.center();
    math::Vec3 c2 = bounds2.center();
    math::Vec3 dir = c2 - c1;
    if (dir.length_squared() < math::EPSILON_SQ) {
        dir = math::Vec3::up();
    }
    math::Vec3 axis_normal(0.0f, 0.0f, 0.0f);
    switch (min_axis) {
        case 0: axis_normal = math::Vec3(dir.x >= 0.0f ? 1.0f : -1.0f, 0.0f, 0.0f); break;
        case 1: axis_normal = math::Vec3(0.0f, dir.y >= 0.0f ? 1.0f : -1.0f, 0.0f); break;
        default: axis_normal = math::Vec3(0.0f, 0.0f, dir.z >= 0.0f ? 1.0f : -1.0f); break;
    }
    result.normal = axis_normal;

    return result;
}

RaycastResult raycast_scene(Scene* scene, const math::Ray& ray, float max_distance,
                            uint32_t layer_mask) {
    RaycastResult best;
    if (!scene) {
        return best;
    }

    for (Entity* entity : scene->get_entities()) {
        if (!entity) {
            continue;
        }
        Collision* collision = entity->get_component<Collision>();
        if (!collision) {
            continue;
        }
        // Layer mask filters which colliders are tested
        if ((collision->get_collision_layers() & layer_mask) == 0) {
            continue;
        }
        // Trigger volumes are for overlap events, not ray blocking
        if (collision->get_type() == CollisionType::Trigger) {
            continue;
        }

        float t = collision->raycast_distance(ray, max_distance);
        if (t >= 0.0f && (!best.hit || t < best.distance)) {
            best.hit = true;
            best.distance = t;
            best.point = ray.point_at(t);
            best.entity = entity;

            // Contact normal: exact for spheres, bounds-axis approximation
            // for the remaining shapes.
            WorldShape shape = get_world_shape(collision);
            if (shape.type == CollisionShape::Sphere) {
                math::Vec3 n = best.point - shape.sphere.center;
                best.normal = (n.length_squared() > math::EPSILON_SQ)
                                  ? n.normalized()
                                  : math::Vec3::up();
            } else {
                math::AABB bounds = shape_world_bounds(shape);
                math::Vec3 local = best.point - bounds.center();
                math::Vec3 half = bounds.extents();
                // Pick the axis with the smallest relative distance to a face
                float ax = std::abs(local.x) / std::max(half.x, math::EPSILON);
                float ay = std::abs(local.y) / std::max(half.y, math::EPSILON);
                float az = std::abs(local.z) / std::max(half.z, math::EPSILON);
                if (ax >= ay && ax >= az) {
                    best.normal = math::Vec3(local.x >= 0.0f ? 1.0f : -1.0f, 0.0f, 0.0f);
                } else if (ay >= az) {
                    best.normal = math::Vec3(0.0f, local.y >= 0.0f ? 1.0f : -1.0f, 0.0f);
                } else {
                    best.normal = math::Vec3(0.0f, 0.0f, local.z >= 0.0f ? 1.0f : -1.0f);
                }
            }
        }
    }

    return best;
}

} // namespace poko::world
