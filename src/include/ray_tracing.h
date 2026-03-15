#pragma once


const char* csSource = R"(
    #version 460
    
    layout(local_size_x = 16, local_size_y = 16) in;
    layout(rgba32f, binding = 0) uniform image2D imgOutput;
    
    
    uniform int frame;
    
    #define MAX_SPHERES 64
    uniform int sphere_count = 0;
    uniform vec3 sphere_pos[MAX_SPHERES];
    uniform float sphere_rad[MAX_SPHERES];
    uniform int sphere_mat[MAX_SPHERES];

    #define MAX_MATERIALS 16

    uniform int material_count;
    uniform int material_type[MAX_MATERIALS];
    uniform vec3 material_albedo[MAX_MATERIALS];
    uniform float material_fuzz[MAX_MATERIALS];





    
    struct Ray {
        vec3 orig;
        vec3 dir;
    };


    const float pi = 3.1415926535897932385;
    const float infinity = 1.0/0.0;

    vec3 at(Ray r, float t) {
        return r.orig + t*r.dir;
    }

    float deg_to_rad(float degrees) {
        return degrees * pi/180.0;
    }

    struct interval {
        float min;
        float max;    
    };

    float size(interval i) {
        return i.max - i.min;
    }
    
    bool contains(interval i, float x) {
        return i.min <= x && x <= i.max;
    }

    bool surrounds(interval i, float x) {
        return i.min < x && x < i.max;
    }

    float clamp(interval i, float x) {
        if (x<i.min) { return i.min; }
        if (x>i.max) { return i.max; }
        return x;
    }

    float rand(vec3 seed)
    {
        return fract(sin(dot(seed, vec3(12.9898, 78.233, 37.719))) * 43758.5453123);
    }

    float rand(vec3 seed, float min, float max) {
        return min + (max-min)*rand(seed);
    }

    vec3 random(vec3 seed1, vec3 seed2, vec3 seed3) {
        return vec3(
            rand(vec3(seed1)),
            rand(vec3(seed2)),
            rand(vec3(seed3))
        );
    }

    vec3 random(vec3 seed1, vec3 seed2, vec3 seed3, float min, float max) {
        return vec3(
            rand(vec3(seed1), min, max),
            rand(vec3(seed2), min, max),
            rand(vec3(seed3), min, max)
        );
    }
    
    vec3 random_unit_vector(vec3 seed1, vec3 seed2, vec3 seed3) {
        for (int i = 0; i < 64; i++) {
            vec3 p = random(seed1, seed2, seed3, -1, 1);
            float lensq = length(p)*length(p);
            if (1e-160 < lensq && lensq <= 1) {
                return p/sqrt(lensq);
            }
            seed1 += vec3(1.0);
            seed2 += vec3(1.0);
            seed3 += vec3(1.0);
        }
        return vec3(0,1,0);
    }
    
    float linear_to_gamma(float linear_component) {
        if (linear_component > 0) {
            return sqrt(linear_component);
        }
        return 0; 
    }


















    







    struct hit_record {
        vec3 p;
        vec3 normal;
        float t;
        bool front_face;
        int material_id;
    };

    void set_face_normal(Ray r, inout hit_record rec, vec3 outward_normal) {
        rec.front_face = dot(r.dir, outward_normal) < 0;
        if (rec.front_face) {
            rec.normal = outward_normal;
        } else {
            rec.normal = -outward_normal;
        }
    }




    struct Material {
        int type;
        vec3 albedo;
        float fuzz;
    };

    bool scatter(int mat_id, Ray r_in, hit_record rec, inout vec3 attenuation, inout Ray scattered, vec3 seed1, vec3 seed2, vec3 seed3)
    {
        if (material_type[mat_id] == 0)
        {
            vec3 scatter_direction = rec.normal + random_unit_vector(seed1, seed2, seed3);

            if (length(scatter_direction) < 1e-8)
            {
                scatter_direction = rec.normal;
            }

            scattered.orig = rec.p;
            scattered.dir = normalize(scatter_direction);

            attenuation = material_albedo[mat_id];
            return true;
        }
        
        if (material_type[mat_id] == 1)
        {
            vec3 reflected = reflect(normalize(r_in.dir), rec.normal);
            reflected += material_fuzz[mat_id] * random_unit_vector(seed1, seed2, seed3);

            scattered.orig = rec.p;
            scattered.dir = reflected;

            attenuation = material_albedo[mat_id];
            return dot(scattered.dir, rec.normal) > 0.0;
        }
    }






    struct Sphere {
        vec3 center;
        float radius;
        int material_id;
    };


    Sphere spheres[MAX_SPHERES];

    bool hit_sphere(Ray r, Sphere s, interval ray_t, inout hit_record rec)
    {
        vec3 oc = s.center - r.orig;
        float a = dot(r.dir, r.dir);
        float h = dot(r.dir, oc);
        float c = dot(oc, oc) - s.radius*s.radius;
        float discriminant = h*h - a*c;
        
        if (discriminant < 0.0) {
            return false;
        }

        float sqrtd = sqrt(discriminant);

        float root = (h - sqrtd) / a;
        if (!surrounds(ray_t, root)) {
            root = (h+sqrtd) / a;
            if (!surrounds(ray_t, root)) {
                return false;
            }
        }
        
        rec.t = root;
        rec.p = at(r, rec.t);
        vec3 outward_normal = (rec.p - s.center) / s.radius;
        set_face_normal(r, rec, outward_normal);
        rec.material_id = s.material_id;

        return true;
    }



    bool hit_world(Ray r, interval ray_t, inout hit_record rec)
    {
        hit_record temp_rec;
        bool hit_anything = false;
        float closest_so_far = ray_t.max;

        for (int i = 0; i < sphere_count; i++) {
            interval inter;
            inter.min = ray_t.min;
            inter.max = closest_so_far;

            if (hit_sphere(r, spheres[i], inter, temp_rec)) {
                hit_anything = true;
                closest_so_far = temp_rec.t;
                rec = temp_rec;
            }
        }

        return hit_anything;
    }















    




    vec3 ray_color(Ray r, int max_depth,  vec3 seed1, vec3 seed2, vec3 seed3)
    {
        vec3 curr_attenuation = vec3(1.0); // accumulates color scaling
        vec3 color = vec3(0.0);

        for (int depth = 0; depth < max_depth; depth++) {
            hit_record rec;

            interval inter;
            inter.min = 0.001;
            inter.max = infinity;

            if (hit_world(r, inter, rec)) {
                Ray scattered;
                vec3 attenuation;
                if (scatter(rec.material_id, r, rec, attenuation, scattered, seed1, seed2, seed3)) 
                {
                    curr_attenuation *= attenuation;
                    r = scattered;
                } else {
                    return vec3(0.0); 
                }
            } else {
                vec3 unit_direction = normalize(r.dir);
                float t = 0.5 * (unit_direction.y + 1.0);
                color = curr_attenuation * ((1.0-t)*vec3(1.0) + t*vec3(0.5, 0.7, 1.0));
                break;
            }
        }

        return color;
    }
    

    void main()
    {
        ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
        ivec2 size = imageSize(imgOutput);

        if(pixel.x >= size.x || pixel.y >= size.y) return;



        float aspect_ratio = float(size.x) / float(size.y);
        int max_depth = 10;
        
        float focal_length = 1.0;
        float viewport_height = 2.0;
        float viewport_width = viewport_height * aspect_ratio;
        vec3 camera_center = vec3(0.0, 0.0, 0.0);

        vec3 viewport_u = vec3(viewport_width, 0.0, 0.0);
        vec3 viewport_v = vec3(0.0, viewport_height, 0.0);

        vec3 pixel_delta_u = viewport_u / size.x;
        vec3 pixel_delta_v = viewport_v / size.y;

        vec3 viewport_upper_left = camera_center - vec3(0.0, 0.0, focal_length) - viewport_u*0.5 - viewport_v*0.5;
        vec3 pixel00_loc = viewport_upper_left + 0.5*( pixel_delta_u + pixel_delta_v );

        vec3 pixel_center = pixel00_loc + (pixel.x * pixel_delta_u) + (pixel.y * pixel_delta_v);
        vec3 ray_direction = pixel_center - camera_center;





        for (int i=0; i<sphere_count; i++) {
            Sphere sphere;
            sphere.center = sphere_pos[i];
            sphere.radius = sphere_rad[i];
            sphere.material_id = sphere_mat[i];
            spheres[i] = sphere;
        }



        vec3 offset = vec3(
            rand(vec3(gl_GlobalInvocationID.xy, 0) + vec3(pixel.x + frame, pixel.y + frame, 0)) - 0.5,
            rand(vec3(gl_GlobalInvocationID.xy, 0) + vec3(pixel.x* 5.665412365412364 + frame, pixel.y * 0.246546314563145 + frame, 0)) - 0.5,
            0
        );
        vec3 pixel_sample = pixel00_loc + ((float(pixel.x)+offset.x) * pixel_delta_u) + ((float(pixel.y)+offset.y) * pixel_delta_v);
        Ray r;
        r.orig = camera_center;
        r.dir = pixel_sample - r.orig;
        


        vec3 seed1 = vec3(frame+1);
        vec3 seed2 = vec3(frame+7);
        vec3 seed3 = vec3(frame+13);
        
        vec4 _prev = imageLoad(imgOutput, pixel);
        vec4 prev = _prev * _prev;

        
        vec3 new_sample = ray_color(r, max_depth, seed1,seed2,seed3);
        vec3 accumulated = (prev.rgb*(float(frame-1)) + new_sample) / float(frame);


        accumulated.r = linear_to_gamma(accumulated.r);
        accumulated.g = linear_to_gamma(accumulated.g);
        accumulated.b = linear_to_gamma(accumulated.b);
        
    
        imageStore(imgOutput, pixel, vec4(accumulated ,1));
    }

)";