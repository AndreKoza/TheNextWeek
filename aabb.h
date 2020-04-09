#pragma once

#include "rtweekend.h"

class aabb
{
	public:
		aabb() {}
		aabb(const vec3& a, const vec3& b) { _min = a; _max = b; }

		vec3 min() const { return _min; }
		vec3 max() const { return _max; }

		bool hit(const ray& r, double t_min, double t_max) const
		{
			// iterate over the three axes
			for (int a = 0; a < 3; ++a)
			{
				auto invD = 1.0f / r.direction()[a];
				auto t0 = (_min[a] - r.origin()[a]) * invD;
				auto t1 = (_max[a] - r.origin()[a]) * invD;
				if (invD < 0.0f)
					std::swap(t0, t1);
				t_min = t0 > t_min ? t0 : t_min;
				t_max = t1 < t_max ? t1 : t_max;

				if (t_max <= t_min)
					return false;
			}
			return true;
		}

		vec3 _min;
		vec3 _max;
};

aabb surrounding_box(aabb box0, aabb box1)
{
	vec3 small 
		(min(box0.min().x(), box1.min().x()),
		 min(box0.min().y(), box1.min().y()),
		 min(box0.min().z(), box1.min().z()));
	vec3 big   
		(max(box0.max().x(), box1.max().x()),
		 max(box0.max().y(), box1.max().y()),
		 max(box0.max().z(), box1.max().z()));
	return aabb(small, big);
}
