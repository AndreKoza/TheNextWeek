#include "pch.h"

#include <fstream>
#include <chrono>
#include <limits>

#include "material.h"
#include "camera.h"
#include "sphere.h"
#include "hitable_list.h"


vec3 color(const ray& r, hitable *world, int depth)
{
    hit_record rec;
    // use 0.001 instead of 0 to avoid shadow acne (in this case leads to exception (don't know why))
    if (world->hit(r, 0.001f, std::numeric_limits<float>::max(), rec))
    {
        ray scattered;
        vec3 attenuation;
        if (depth < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scattered))
        {
            return attenuation*color(scattered, world, depth+1);
        }
        else
        {
            return vec3(0,0,0);
        }
    }
    else
    {
        vec3 unit_direction = unit_vector(r.direction());
        float t = 0.5f * (unit_direction.y() + 1.0f);
        // Linear interpolate between white and some other color
        return (1.0f-t)*vec3(1.0f, 1.0f, 1.0f) + t*vec3(0.5f, 0.7f, 1.0f);
    }
}


hitable* random_scene()
{
    int n = 500;
    hitable** list = new hitable*[n+1];
    list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(vec3(0.4f, 0.4f, 0.4f)));
    int i = 1;
    for (int a = -11; a < 11; a++)
    {
        for (int b = -11; b < 11; b++)
        {
            float choose_mat = (float)dis(gen);
            vec3 center(a+0.9f*(float)dis(gen), 0.2f, b+0.9f*(float)dis(gen));
            if ((center-vec3(4.f,0.2f,0.f)).length() > 0.9f)
            {
                // diffuse
                if (choose_mat < 0.8)
                {
                    list[i++] = new sphere(center, 0.2f, new lambertian(vec3((float)dis(gen)*(float)dis(gen), (float)dis(gen)*(float)dis(gen), (float)dis(gen)*(float)dis(gen))));
                }
                // metal
                else if (choose_mat < 0.95)
                {
                    list[i++] = new sphere(center, 0.2f, new metal(vec3(0.5f*(1+ (float)dis(gen)), 0.5f*(1+ (float)dis(gen)), 0.5f*(1+ (float)dis(gen))), 0.5f*(float)dis(gen)));
                }
                // glass
                else
                {
                    // vec3 center_new(vec3(center.x(), 0.16, center.z()));
                    list[i++] = new sphere(center, 0.2f, new dielectric(1.5f));
                }
            }
        }
    }

    list[i++] = new sphere(vec3(0, 1, 0), 1.0f, new dielectric(1.5f));
    list[i++] = new sphere(vec3(-4, 1, 0), 1.0f, new lambertian(vec3(0.1f, 0.2f, 0.5f)));
    list[i++] = new sphere(vec3(4, 1, 0), 1.0f, new metal(vec3(0.7f, 0.6f, 0.5f), 0.0f));

    return new hitable_list(list, i);
}


int main()
{
    auto start = std::chrono::system_clock::now();
    std::ofstream output;
    output.open("picture.ppm");

    int nx = 400;
    int ny = 200;
    int ns = 5;
    output << "P3\n" << nx << " " << ny << "\n255\n";


    // hitable *list[5];
    // list[0] = new sphere(vec3(0,0,-1), 0.5, new lambertian(vec3(0.1, 0.2, 0.5))); //(0.8, 0.3, 0.3
    // list[1] = new sphere(vec3(0,-100.5,-1), 100, new lambertian(vec3(0.8, 0.8, 0.0)));
    // list[2] = new sphere(vec3(1,0,-1), 0.5, new metal(vec3(0.8, 0.6, 0.2)));
    // list[3] = new sphere(vec3(-1, 0, -1), 0.5, new dielectric(1.5));
    // list[4] = new sphere(vec3(-1, 0, -1), -0.45, new dielectric(1.5));
    // hitable *world = new hitable_list(list, 5);
    hitable *world = random_scene();

    vec3 lookfrom(13,2,3);
    vec3 lookat(0,0,0);
    float dist_to_focus = 10;//(lookfrom-lookat).length();
    float aperture = 0.1f;
    camera cam(lookfrom, lookat, vec3(0,1,0), 20, float(nx)/float(ny), aperture, dist_to_focus);
    
    int percent = ny / 100;
    
    for (int j = ny - 1; j >= 0; j--)
    {
        if (j % percent == 0)
        {
            int print_percent = std::abs((j - ny) / percent);
            std::cout << print_percent << "% done. \n";
        }
        for (int i = 0; i < nx; i++)
        {
            vec3 col(0, 0, 0);
            // Number of rays per pixel
            for(int s = 0; s < ns; s++)
            {
                float u = float(i + dis(gen)) / float(nx);
				float v = float(j + dis(gen)) / float(ny);
				ray r = cam.get_ray(u, v);
                // vec3 p = r.point_at_parameter(2.0); <- don't know why this line is in the tutorial
                col += color(r, world, 0);
            }

            col /= float(ns);

            // Gamma correction
            col = vec3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));

            int ir = int(255.99 * col[0]);
            int ig = int(255.99 * col[1]);
            int ib = int(255.99 * col[2]);

            output << ir << " " << ig << " " << ib << "\n";
        }
    }

    output.close();
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = end-start;
    std::cout << "Total time: " << diff.count() << " s\n";
    std::cin.ignore();
    return 0;
}
    