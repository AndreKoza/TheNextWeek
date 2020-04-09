#pragma once

#include "rtweekend.h"
#include "hitable.h"

#include <memory>
#include <vector>


class hitable_list: public hitable
{
    public:
        hitable_list() {}
        hitable_list(shared_ptr<hitable> object) { add(object); }

        void clear() { objects.clear(); }
        void add(shared_ptr<hitable> object) { objects.push_back(object); }

        virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const;
        virtual bool bounding_box(double t0, double t1, aabb& output_box) const;
        

        std::vector<shared_ptr<hitable>> objects;

};

bool hitable_list::hit(const ray& r, double t_min, double t_max, hit_record& rec) const
{
    hit_record temp_rec;
    bool hit_anything = false;
    double closest_so_far = t_max;

    for (const auto& object : objects)
    {
        if (object->hit(r, t_min, closest_so_far, temp_rec))
        {
            hit_anything = true;
            closest_so_far = temp_rec.t;
            rec = temp_rec;
        }
    }
    return hit_anything;
}

bool hitable_list::bounding_box(double t0, double t1, aabb& output_box) const
{
    if (objects.empty())
        return false;

    aabb temp_box;
    bool first_true = objects[0]->bounding_box(t0, t1, temp_box);

    if (!first_true)
        return false;

    output_box = temp_box;

    for (const auto& object : objects)
    {
        if (!object->bounding_box(t0, t1, temp_box))
            return false;
        output_box = surrounding_box(output_box, temp_box);
    }

    return true;
}
