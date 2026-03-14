#pragma once


const char* csSource = R"(
    #version 460

    layout(local_size_x = 16, local_size_y = 16) in;
    layout(rgba32f, binding = 0) uniform image2D imgOutput;


    vec3 ray_color(vec3 orig, vec3 dir)
    {
        vec3 unit_direction = normalize(dir);
        float a = 0.5*(unit_direction.y + 1.0);
        return (1.0-a)*vec3(1.0,1.0,1.0) + a*vec3(0.5, 0.7, 1.0);
    }

    void main()
    {
        ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
        ivec2 size = imageSize(imgOutput);

        if(pixel.x >= size.x || pixel.y >= size.y) return;



        float aspect_ratio = float(size.x) / float(size.y);

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



        vec3 pixel_color = ray_color(camera_center, ray_direction);

        imageStore(imgOutput, pixel, vec4( ray_color(camera_center, ray_direction) ,1));
    }
)";