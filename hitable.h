#pragma once

#include "rtweekend.h"
#include "aabb.h"


class material;

struct hit_record
{
    vec3 p; // hit point
    vec3 normal; // hit point normal
    shared_ptr<material> mat_ptr; // stored material pointer
    double t; // the t from the ray equation
    double u; // u texture coordinate
    double v; // v texture coordinate
    bool front_face; // front face or back face?

    inline void set_face_normal(const ray& r, const vec3& outward_normal)
    {
        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

class hitable
{
    public:
        // Only hits in the interval [t_min, t_max] are considered. t being the t from ray equation p(t) = orig + t*direction
        virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const = 0;

        // Compute bounding box of object. Object may move in interval time0 und time1, so aabb is calculated to bound all possible locations.
        virtual bool bounding_box(double time0, double time1, aabb& output_box) const = 0;
};
