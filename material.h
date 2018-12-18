#ifndef MATERIALH
#define MATERIALH

struct hit_record;

#include "ray.h"
#include "hitable.h"
#include <random>

std::random_device rd;  //Will be used to obtain a seed for the random number engine
std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
std::uniform_real_distribution<> dis(0.0, 1.0);

// Returns a random point within unit sphere. First a point within the unit cube is calculated and then rejected if outside
vec3 random_in_unit_sphere()
{
    vec3 p;
    do
    {
        // Range of random numbers: [0, 1). Start is center of sphere: function 2*x-1 reaches values between [-1, 1).
        p = 2.0f * vec3((float)dis(gen), (float)dis(gen), (float)dis(gen)) - vec3(1.f,1.f,1.f);
    } while (p.squared_length() >= 1.0);
    return p;
}

// Standard reflection equation (incident angle = reflected angle etc.)
vec3 reflect(const vec3& v, const vec3& n)
{
    return v - 2 * dot(v,n) * n;
}

// Standard refraction equation, a bit hard to derive and strange variable names but for now I stick to tutorial names
bool refract(const vec3& v, const vec3& n, float ni_over_ti, vec3& refracted)
{
    vec3 uv = unit_vector(v);
    float dt = dot(uv, n);
    float discriminant = 1.0f - ni_over_ti*ni_over_ti*(1-dt*dt);
    if (discriminant > 0)
    {
        refracted = ni_over_ti*(uv - n*dt) - n*sqrt(discriminant);
        return true;
    }
    else
    {
        return false;
    }
}

// Reflectivity varies with angle -> this is an approximation for that (haven't looked into that)
float schlick(float cosine, float ref_idx)
{
    float r0 = (1-ref_idx) / (1+ref_idx);
    r0 = r0*r0;
    return r0 + (1-r0)*pow((1-cosine),5);
}

class material
{
    public:
        virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const = 0;
};


class lambertian : public material
{
    public:
        lambertian(const vec3& a) : 
            albedo(a)
        {
        }

        virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const
        {
            vec3 target = rec.p + rec.normal + random_in_unit_sphere();
            scattered = ray(rec.p, target-rec.p);
            attenuation = albedo;
            return true;
        }

        vec3 albedo;
};


class metal : public material
{
    public:
        metal(const vec3& a, float f = 0) :
            albedo(a)
        {
            if (f < 1)
            {
                fuzz = f;
            }
            else
            {
                fuzz = 1;
            }
        }

        virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const
        {
            vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
            scattered = ray(rec.p, reflected + fuzz*random_in_unit_sphere());
            attenuation = albedo;
            // See definition of dot product: If dot product > 0 -> angle is sharp (spitzer Winkel)
            return (dot(scattered.direction(), rec.normal) > 0);
        }

        vec3 albedo;
        float fuzz;
};


class dielectric : public material
{
    public:
        dielectric(float ri) :
            ref_idx(ri)
        {}

        virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const
        {
            vec3 outward_normal;
            vec3 reflected = reflect(r_in.direction(), rec.normal);
            float ni_over_nt;
            attenuation = vec3(1.0, 1.0, 1.0);
            vec3 refracted;
            float reflect_prob;
            float cosine;
            if (dot(r_in.direction(), rec.normal) > 0)
            {
                outward_normal = -rec.normal;
                ni_over_nt = ref_idx;
                cosine = ref_idx * dot(r_in.direction(), rec.normal) / r_in.direction().length();
            }
            else
            {
                outward_normal = rec.normal;
                ni_over_nt = 1.0f / ref_idx;
                cosine = -dot(r_in.direction(), rec.normal) / r_in.direction().length();
            }
            if (refract(r_in.direction(), outward_normal, ni_over_nt, refracted))
            {
                reflect_prob = schlick(cosine, ref_idx);
            }
            else
            {
                reflect_prob = 1.0;
            }
            if (dis(gen) < reflect_prob)
            {
                scattered = ray(rec.p, reflected);
            }
            else
            {
                scattered = ray(rec.p, refracted);
            }
            return true;
        }

        float ref_idx;
};

#endif