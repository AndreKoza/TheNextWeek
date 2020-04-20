#include <fstream>
#include <chrono>
#include <ppl.h>

#include "rtweekend.h"
#include "bvh.h"
#include "camera.h"
#include "hitable_list.h"
#include "material.h"
#include "sphere.h"
#include "moving_sphere.h"
#include "rtw_stb_image.h"


vec3 ray_color(const ray& r, hitable &world, int depth)
{
    hit_record rec;

    // If we've exceeded the ray bounce limit, no more light is gathered.
    if (depth <= 0)
        return vec3(0, 0, 0);

    // use 0.001 instead of 0 to avoid shadow acne (in this case leads to exception (don't know why))
    if (world.hit(r, 0.001, infinity, rec))
    {
        ray scattered;
        vec3 attenuation;
        if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
        {
            return attenuation * ray_color(scattered, world, depth-1);
        }
        else
        {
            return vec3(0,0,0);
        }
    }
    else
    {
        vec3 unit_direction = unit_vector(r.direction());
        double t = 0.5 * (unit_direction.y() + 1.0);
        // Linear interpolate between white and some other color
        return (1.0-t)*vec3(1.0, 1.0, 1.0) + t*vec3(0.5, 0.7, 1.0);
    }
}


hitable_list random_scene()
{
    hitable_list world;

    // Lambertian big sphere as "world"
    //world.add(make_shared<sphere>(vec3(0, -1000, 0), 1000, make_shared<lambertian>(make_shared<constant_texture>(vec3(0.5, 0.5, 0.5)))));

    auto checker = make_shared<checker_texture>(
        make_shared<constant_texture>(vec3(0.2, 0.3, 0.1)),
        make_shared<constant_texture>(vec3(0.9, 0.9, 0.9))
    );

    // Checker sphere as world
    world.add(make_shared<sphere>(vec3(0, -1000, 0), 1000, make_shared<lambertian>(checker)));

    int i = 1;
    for (int a = -12; a < 12; a++)
    {
        for (int b = -12; b < 12; b++)
        {
            double choose_mat = random_double();
            vec3 center(a + 0.9f * random_double(), 0.2, b + 0.9f * random_double());
            if ((center - vec3(4, 0.2, 0)).length() > 0.9)
            {
                // perlin marble
                if (choose_mat < 0.3)
                {
                    auto pertext = make_shared<noise_texture>(4);
                    world.add(make_shared<sphere>(center, 0.2, make_shared<lambertian>(pertext)));
                }
                // diffuse
                if (choose_mat < 0.8)
                {
                    auto albedo = vec3::random() * vec3::random();
                    auto rnd = random_double();
                    if (rnd < 0.5)
                        world.add(make_shared<sphere>(center, 0.2, make_shared<lambertian>(make_shared<constant_texture>(albedo))));
                    else
                        world.add(make_shared<moving_sphere>(center, center + vec3(0, random_double(0, 0.5), 0), 0.0, 1.0, 0.2, make_shared<lambertian>(make_shared<constant_texture>(albedo))));
                }
                // metal
                else if (choose_mat < 0.95)
                {
                    auto albedo = vec3::random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    world.add(make_shared<sphere>(center, 0.2, make_shared<metal>(albedo, fuzz)));
                }
                // glass
                else
                {
                    world.add(make_shared<sphere>(center, 0.2, make_shared<dielectric>(1.5)));
                }
            }
        }
    }

    int nx;
    int ny;
    int nn;
    unsigned char* texture_data = stbi_load("earthmap.jpg", &nx, &ny, &nn, 0);

    auto earth_surface =
        make_shared<lambertian>(make_shared<image_texture>(texture_data, nx, ny));
    world.add(make_shared<sphere>(vec3(3, 0.5, -1), 0.5, earth_surface));

    auto pertext = make_shared<noise_texture>(4);
    world.add(make_shared<sphere>(vec3(0, 1, 2), 1.0, make_shared<lambertian>(pertext)));

    world.add(make_shared<sphere>(vec3(0, 1, 0), 1.0, make_shared<dielectric>(1.5)));
    
    world.add(make_shared<sphere>(vec3(0, 1, -2), 1.0, make_shared<metal>(vec3(0.7, 0.6, 0.5), 0.0)));
    world.add(make_shared<sphere>(vec3(0, 1, -4), 1.0, make_shared<lambertian>(make_shared<constant_texture>(vec3(0.1, 0.2, 0.5)))));


    //return world;
    return hitable_list(make_shared<bvh_node>(world, 0.0, 1.0));
}

hitable_list two_spheres()
{
    hitable_list objects;

    auto checker = make_shared<checker_texture>(
        make_shared<constant_texture>(vec3(0.2, 0.3, 0.1)),
        make_shared<constant_texture>(vec3(0.9, 0.9, 0.9))
    );

    objects.add(make_shared<sphere>(vec3(0, -10, 0), 10, make_shared<lambertian>(checker)));
    objects.add(make_shared<sphere>(vec3(0, 10, 0), 10, make_shared<lambertian>(checker)));

    return objects;
}

hitable_list two_perlin_spheres()
{
    hitable_list objects;

    auto pertext = make_shared<noise_texture>(4);
    objects.add(make_shared<sphere>(vec3(0, -1000, 0), 1000, make_shared<lambertian>(pertext)));
    objects.add(make_shared<sphere>(vec3(0, 2, 0), 2, make_shared<lambertian>(pertext)));

    return objects;
}

hitable_list earth()
{
    int nx;
    int ny;
    int nn;
    unsigned char* texture_data = stbi_load("earthmap.jpg", &nx, &ny, &nn, 0);

    auto earth_surface =
        make_shared<lambertian>(make_shared<image_texture>(texture_data, nx, ny));
    auto globe = make_shared<sphere>(vec3(0, 0, 0), 2, earth_surface);

    return hitable_list(globe);
}

int main()
{
    auto start = std::chrono::system_clock::now();
    std::ofstream output;
    output.open("picture.ppm");

    const int image_width = 1200;
    const int image_height = 600;
    const int samples_per_pixel = 50;
    const int max_depth = 50;
    const auto aspect_ratio = double(image_width) / double(image_height);

    output << "P3\n" << image_width << " " << image_height << "\n255\n";


    // hitable *list[5];
    // list[0] = new sphere(vec3(0,0,-1), 0.5, new lambertian(vec3(0.1, 0.2, 0.5))); //(0.8, 0.3, 0.3
    // list[1] = new sphere(vec3(0,-100.5,-1), 100, new lambertian(vec3(0.8, 0.8, 0.0)));
    // list[2] = new sphere(vec3(1,0,-1), 0.5, new metal(vec3(0.8, 0.6, 0.2)));
    // list[3] = new sphere(vec3(-1, 0, -1), 0.5, new dielectric(1.5));
    // list[4] = new sphere(vec3(-1, 0, -1), -0.45, new dielectric(1.5));
    // hitable *world = new hitable_list(list, 5);

    auto world = random_scene();

    vec3 lookfrom(13, 2, 3);
    vec3 lookat(0, 0, 0);
    vec3 vup(0, 1, 0);
    auto dist_to_focus = 10; //(lookfrom-lookat).length();
    auto aperture = 0.0;

    camera cam(lookfrom, lookat, vup, 20, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);
    
    int percent = image_height / 100;
    
    for (int j = image_height - 1; j >= 0; --j)
    {
        if (j % percent == 0)
        {
            int print_percent = std::abs((j - image_height) / percent);
            std::cout << print_percent << "% done. \n";
        }
        for (int i = 0; i < image_width; ++i)
        {
            vec3 color;

            // Number of rays per pixel
            concurrency::parallel_for(int(0), samples_per_pixel, [&](int s)
            //for (int s = 0; s < samples_per_pixel; ++s)
            {
                auto u = (i + random_double()) / image_width;
                auto v = (j + random_double()) / image_height;
                ray r = cam.get_ray(u, v);
                color += ray_color(r, world, max_depth);
            }); 

            color.write_color(output, samples_per_pixel);
        }
    }

    output.close();
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = end-start;
    std::cout << "Total time: " << diff.count() << " s\n";
    std::cin.ignore();
    return 0;
}
    