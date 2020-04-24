#include <fstream>
#include <chrono>
#include <ppl.h>

#include "rtweekend.h"
#include "bvh.h"
#include "camera.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"
#include "aarect.h"
#include "moving_sphere.h"
#include "rtw_stb_image.h"
#include "box.h"


vec3 ray_color(const ray& r, const vec3& background, const hittable &world, int depth)
{
    hit_record rec;

    // If we've exceeded the ray bounce limit, no more light is gathered.
    if (depth <= 0)
        return Color::black;

    // use 0.001 (epsilon) instead of 0 to avoid shadow acne (in this case leads to exception (don't know why))
    if (!world.hit(r, epsilon, infinity, rec))
        return background;

    ray scattered;
    vec3 attenuation;
    vec3 emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);

    if (!rec.mat_ptr->scatter(r, rec, attenuation, scattered))
        return emitted;

    return emitted + attenuation * ray_color(scattered, background, world, depth - 1);

    //else
    //{
    //    vec3 unit_direction = unit_vector(r.direction());
    //    double t = 0.5 * (unit_direction.y() + 1.0);
    //    // Linear interpolate between white and some other color
    //    return (1.0-t)*vec3(1.0, 1.0, 1.0) + t*vec3(0.5, 0.7, 1.0);
    //}
}


hittable_list random_scene()
{
    hittable_list world;

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
    return hittable_list(make_shared<bvh_node>(world, 0.0, 1.0));
}

hittable_list two_spheres()
{
    hittable_list objects;

    auto checker = make_shared<checker_texture>(
        make_shared<constant_texture>(vec3(0.2, 0.3, 0.1)),
        make_shared<constant_texture>(vec3(0.9, 0.9, 0.9))
    );

    objects.add(make_shared<sphere>(vec3(0, -10, 0), 10, make_shared<lambertian>(checker)));
    objects.add(make_shared<sphere>(vec3(0, 10, 0), 10, make_shared<lambertian>(checker)));

    return objects;
}

hittable_list two_perlin_spheres()
{
    hittable_list objects;

    auto pertext = make_shared<noise_texture>(4);
    objects.add(make_shared<sphere>(vec3(0, -1000, 0), 1000, make_shared<lambertian>(pertext)));
    objects.add(make_shared<sphere>(vec3(0, 2, 0), 2, make_shared<lambertian>(pertext)));

    return objects;
}

hittable_list earth()
{
    int nx;
    int ny;
    int nn;
    unsigned char* texture_data = stbi_load("earthmap.jpg", &nx, &ny, &nn, 0);

    auto earth_surface =
        make_shared<lambertian>(make_shared<image_texture>(texture_data, nx, ny));
    auto globe = make_shared<sphere>(vec3(0, 0, 0), 2, earth_surface);

    return hittable_list(globe);
}

hittable_list simple_light()
{
    hittable_list objects;

    auto pertext = make_shared<noise_texture>(4);
    objects.add(make_shared<sphere>(vec3(0, -1000, 0), 1000, make_shared<lambertian>(pertext)));
    objects.add(make_shared<sphere>(vec3(0, 2, 0), 2, make_shared<lambertian>(pertext)));

    auto difflight = make_shared<diffuse_light>(make_shared<constant_texture>(vec3(4, 4, 4)));
    objects.add(make_shared<sphere>(vec3(0, 7, 0), 2, difflight));
    objects.add(make_shared<xy_rect>(3, 5, 1, 3, -2, difflight));

    return objects;
}

hittable_list cornell_box()
{
    hittable_list objects;

    auto red = make_shared<lambertian>(make_shared<constant_texture>(vec3(0.65, 0.05, 0.05)));
    auto white = make_shared<lambertian>(make_shared<constant_texture>(vec3(0.73, 0.73, 0.73)));
    auto green = make_shared<lambertian>(make_shared<constant_texture>(vec3(0.12, 0.45, 0.15)));
    auto light = make_shared<diffuse_light>(make_shared<constant_texture>(vec3(15, 15, 15)));

    objects.add(make_shared<flip_face>(make_shared<yz_rect>(0, 555, 0, 555, 555, green))); // left
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red)); // right
    objects.add(make_shared<xz_rect>(213, 343, 227, 332, 554, light)); // 213, 343, 227, 332, 554  // 120, 420, 120, 420, 554
    objects.add(make_shared<flip_face>(make_shared<xz_rect>(0, 555, 0, 555, 555, white))); // top
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white)); // bottom
    objects.add(make_shared<flip_face>(make_shared<xy_rect>(0, 555, 0, 555, 555, white))); // back

    shared_ptr<hittable> box1 = make_shared<box>(vec3(0, 0, 0), vec3(165, 330, 165), white);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, vec3(265, 0, 295));
    objects.add(box1);

    shared_ptr<hittable> box2 = make_shared<box>(vec3(0, 0, 0), vec3(165, 165, 165), white);
    box2 = make_shared<rotate_y>(box2, -18);
    box2 = make_shared<translate>(box2, vec3(130, 0, 65));
    objects.add(box2);

    //objects.add(make_shared<sphere>(vec3(130, 50, 65), 50.0, make_shared<dielectric>(1.5)));
    
    return objects;
}

int main()
{
    auto start = std::chrono::system_clock::now();
    std::ofstream output;
    output.open("picture.ppm");

    const int image_width = 600;
    const int image_height = 600;
    const int samples_per_pixel = 1000;
    const int max_depth = 50;

    const auto aspect_ratio = double(image_width) / double(image_height);  



    // hittable *list[5];
    // list[0] = new sphere(vec3(0,0,-1), 0.5, new lambertian(vec3(0.1, 0.2, 0.5))); //(0.8, 0.3, 0.3
    // list[1] = new sphere(vec3(0,-100.5,-1), 100, new lambertian(vec3(0.8, 0.8, 0.0)));
    // list[2] = new sphere(vec3(1,0,-1), 0.5, new metal(vec3(0.8, 0.6, 0.2)));
    // list[3] = new sphere(vec3(-1, 0, -1), 0.5, new dielectric(1.5));
    // list[4] = new sphere(vec3(-1, 0, -1), -0.45, new dielectric(1.5));
    // hittable *world = new hittable_list(list, 5);

    

    vec3 lookfrom(13, 2, 3);
    vec3 lookat(0, 0, 0);
    vec3 vup(0, 1, 0);
    auto dist_to_focus = 10; //(lookfrom-lookat).length();
    auto aperture = 0.0;
    auto vfov = 20.0;
    vec3 background(Color::black);
    auto world = random_scene();

    switch (6)
    {
    case 1:
        world = random_scene();
        lookfrom = vec3(13, 2, 3);
        lookat = vec3(0, 0, 0);
        vfov = 20.0;
        background = vec3(0.7, 0.8, 1.0);
        break;

    case 2:
        world = two_spheres();
        lookfrom = vec3(13, 2, 3);
        lookat = vec3(0, 0, 0);
        vfov = 20.0;
        background = vec3(0.70, 0.80, 1.00);
        break;

    case 3:
        world = two_perlin_spheres();
        lookfrom = vec3(13, 2, 3);
        lookat = vec3(0, 0, 0);
        vfov = 20.0;
        background = vec3(0.70, 0.80, 1.00);
        break;

    case 4:
        world = earth();
        lookfrom = vec3(0, 0, 12);
        lookat = vec3(0, 0, 0);
        vfov = 20.0;
        background = vec3(0.70, 0.80, 1.00);
        break;

    case 5:
        world = simple_light();
        lookfrom = vec3(26, 3, 6);
        lookat = vec3(0, 2, 0);
        vfov = 20.0;
        break;

    default:
    case 6:
        world = cornell_box();
        lookfrom = vec3(278, 278, -800);
        lookat = vec3(278, 278, 0);
        vfov = 40.0;
        break;
    }



    output << "P3\n" << image_width << " " << image_height << "\n255\n";

    camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);
    
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
                color += ray_color(r, background, world, max_depth);
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
    