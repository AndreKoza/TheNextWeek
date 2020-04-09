#pragma once

#include "rtweekend.h"
#include "aabb.h"


class material;

struct hit_record
{
    vec3 p; 
    vec3 normal;
    shared_ptr<material> mat_ptr;
    double t; // the t from the ray equation
    bool front_face;

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

        // interval t0 and t1 (the "borders" of the bb)
        virtual bool bounding_box(double t0, double t1, aabb& output_box) const = 0;
};
