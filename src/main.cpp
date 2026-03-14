#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>


const char* csSource = R"(
    #version 460

    layout(local_size_x = 16, local_size_y = 16) in;
    layout(rgba32f, binding = 0) uniform image2D imgOutput;

    void main()
    {
        ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
        ivec2 size = imageSize(imgOutput);

        if(pixel.x >= size.x || pixel.y >= size.y) return;

        imageStore(imgOutput, pixel, vec4(float(pixel.y)/float(size.y),0,0,1));
    }
)";


const unsigned int SCREEN_WIDTH = 640;
const unsigned int SCREEN_HEIGHT = 480;

int main(void)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window;
    window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "RAYFORGE 3D", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        return -1;
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);


//  COMPUTE SHADER CODE ()

    unsigned int tex;
    glGenTextures(1, &tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, SCREEN_WIDTH, SCREEN_HEIGHT);
    glBindImageTexture(0, tex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    unsigned int compute_shader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(compute_shader, 1, &csSource, nullptr);
    glCompileShader(compute_shader);

    int _success;
    glGetShaderiv(compute_shader, GL_COMPILE_STATUS, &_success);
    if (!_success) {
        char log[512];
        glGetShaderInfoLog(compute_shader, 512, NULL, log);
        std::cout << "COMPUTE SHADER COMPILE ERROR:\n" << log << std::endl;
    }

    unsigned int shader_program = glCreateProgram();
    glAttachShader(shader_program, compute_shader);
    glLinkProgram(shader_program);
    glGetProgramiv(shader_program, GL_LINK_STATUS, &_success);
    if (!_success) {
        char log[512];
        glGetProgramInfoLog(shader_program, 512, NULL, log);
        std::cout << "COMPUTE SHADER LINK ERROR:\n" << log << std::endl;
    }

    unsigned int groupsX = (unsigned int)std::ceil(SCREEN_WIDTH/16.0);
    unsigned int groupsY = (unsigned int)std::ceil(SCREEN_HEIGHT/16.0);
    




    const char *vertexShaderSource = R"(
        #version 460 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec2 aUV;
        out vec2 TexCoords;
        void main() {
            TexCoords = aUV;
            gl_Position = vec4(aPos, 1.0);
        }
    )";
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    int  success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    const char *fragmentShaderSource = R"(
        #version 460 core
        in vec2 TexCoords;
        out vec4 FragColor;
        uniform sampler2D screenTex;
        void main() {
            FragColor = texture(screenTex, TexCoords);
        }
    )";
    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    float vertices[] = {
        -1.0, -1.0,  0.0,       0, 0,
        -1.0,  1.0,  0.0,       0, 1,
         1.0,  1.0,  0.0,       1, 1,

         1.0,  1.0,  0.0,       1, 1,
         1.0, -1.0,  0.0,       1, 0,
        -1.0, -1.0,  0.0,       0, 0,
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3* sizeof(float)));
    glEnableVertexAttribArray(1);

    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "screenTex"), 0);

    while (!glfwWindowShouldClose(window))
    {
        glUseProgram(shader_program);
        glDispatchCompute(groupsX, groupsY, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}