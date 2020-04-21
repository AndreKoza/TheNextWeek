#pragma once

#include "rtweekend.h"
#include "aabb.h"


class material;

/* utility function to calculate uv coordinates for a sphere
   Expects things on the unit sphere (divided by radius) centered at the origin (minus center)
   Spherical coordinates phi and theta can be calculated by spherical equations (see tutorial for derivations)
*/
void get_sphere_uv(const vec3& p, double& u, double& v)
{
    auto phi = atan2(p.z(), p.x());
    auto theta = asin(p.y());

    // atan2 returns in the range -pi to pi but we want it in the range [0,1]
    u = 1 - (phi + pi) / (2 * pi);

    // asin returns in the range -pi/2 to pi/2 but we want it in the range [0,1]
    v = (theta + pi / 2) / pi;
}

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

class flip_face : public hitable
{
    public:
        flip_face(shared_ptr<hitable> p)
            : ptr(p) {}

        virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const
        {
            if (!ptr->hit(r, t_min, t_max, rec))
                return false;

            rec.front_face = !rec.front_face;
            return true;
        }

        virtual bool bounding_box(double t0, double t1, aabb& output_box) const
        {
            return ptr->bounding_box(t0, t1, output_box);
        }
        
    private:
        shared_ptr<hitable> ptr;
};
