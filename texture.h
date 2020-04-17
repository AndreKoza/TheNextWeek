#pragma once

#include "rtweekend.h"
#include "perlin.h"

class texture
{
	public:
		virtual vec3 value(double u, double v, const vec3& p) const = 0;
};

class noise_texture : public texture
{
	public:
		noise_texture() {}
		noise_texture(double sc) : scale(sc) {}

		virtual vec3 value(double u, double v, const vec3& p) const
		{
			return vec3(1, 1, 1) * noise.noise(scale * p);
		}

	private:
		perlin noise;
		double scale;
};

class constant_texture : public texture
{
	public:
		constant_texture(vec3 c) : color(c) {}

		// value for constant_texture just retuns the color and ignores u, v and p
		virtual vec3 value(double u, double v, const vec3& p) const
		{
			return color;
		}


	private:
		vec3 color;
};

class checker_texture : public texture
{
	public:
		checker_texture(shared_ptr<texture> t0, shared_ptr<texture> t1)
			: even(t0), odd(t1)
		{}

		/*	We can create a checker texture by noting that the sign of sine and cosine just alternates 
			in a regular way and if we multiply trig functions in all three dimensions, the sign of that 
			product forms a 3D checker pattern. */
		virtual vec3 value(double u, double v, const vec3& p) const
		{
			auto sines = sin(10 * p.x())* sin(10 * p.y())* sin(10 * p.z());
			if (sines < 0)
				return odd->value(u, v, p);
			else
				return even->value(u, v, p);
		}


	private:
		shared_ptr<texture> even;
		shared_ptr<texture> odd;
};
