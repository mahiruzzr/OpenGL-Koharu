#define GLEW_NO_GLU
#define GLFW_INCLUDE_NONE
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>
#include <X11/Xlib.h>
#include <iostream>
#define GLM_FORCE_XYZW_ONLY
#include <X11/Xcursor/Xcursor.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xfixes.h>

#include "Mesh.h"
#include "Model.h"
#include <vector>
#include <vector>
#include "tool/gui.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include<fstream>
#include<string>
#include <sstream>
#include <cmath>
#define STB_IMAGE_IMPLEMENTATION
#include "tool/stb_image.h"
using namespace std;

struct ShaderProgramSource{
    string VertexSource;
    string FragmentSource;
};

static ShaderProgramSource ParseShader(const string& filepath){
    ifstream stream(filepath);

    enum class ShaderType{
        NONE=-1, VERTEX =0 ,FRAGMENT =1
    };

    string line;
    stringstream ss[2];
    ShaderType type =  ShaderType ::NONE;
    while(getline(stream, line)){
        if(line.find("shader")!= string :: npos){
            if(line.find("vertex")!= string :: npos){
                type = ShaderType::VERTEX;
            }else if(line.find("fragment")!= string :: npos){
                type = ShaderType::FRAGMENT;
            }
        }else{
                ss[(int)type] << line << "\n";
        }
    }


    return { ss[0].str(), ss[1].str()};
}

static unsigned int CompileShader(unsigned int type,const string& source){
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS,&result);
    if(result == GL_FALSE){
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        cout << "Failed to compile" <<
        (type == GL_VERTEX_SHADER ? "vertex" : "fragment")<< " shader!" << endl;
        cout << message << endl;
        glDeleteShader(id);
        return 0;
    }

    return id;
}
static  unsigned int CreateShader(const string & vertexShader, const string & fragmentShader){
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}
float deltaTime = 0.0f;
float lastFrame = 0.0f;
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
void processInput(GLFWwindow *window){
    float cameraSpeed = 2.5f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}
float lastX = 400, lastY = 300;
bool firstMouse = true;
float yaw = 90.0f;
float pitch = 0.0f;
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if(firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.05;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset;
    pitch += yoffset;

    if(pitch > 89.0f)
        pitch = 89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}
/*void change_size(GLFWwindow*window, int width, int height){
    if(height==0){
        height =1;
    }
    glViewport(0,0,width,height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = (float)height / (float)width;
    if(height >= width){
        glOrtho(-400.0f,400.0f,-300.0f*aspect,300.0f*aspect,1000.0f,-1000.0f);
    }else{
        glOrtho(-400.0f/aspect,400.0f/aspect,-300.0f,300.0f,1000.0f,-1000.0f);
    }
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}*/
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
class PhysicsObject{
public:
    float gravity = 9.8f;
    glm::vec3 velocity;
    float groundLevel = 1.5f;
    glm::vec3 position;
    glm::vec3 currentMax;
    glm::vec3 currentMin;

    PhysicsObject(const glm::mat4& model,glm::vec3 max,glm::vec3 min){
        position = glm::vec3(model[3]);
        velocity = glm::vec3(0.0f,0.0f,0.0f);
        currentMax = max;
        currentMin = min;
    }
    void UpdatePhysics(float deltatime ,glm::mat4& model){
        if (position.y > groundLevel)
        {
          velocity.y -= gravity * deltatime;
          position += velocity * deltatime;
        }else{
          position.y = groundLevel;
          velocity.y = 0.0f;
        }
        model[3][0] = position.x;
        model[3][1] = position.y;
        model[3][2] = position.z;
    }
    void AABB(glm::vec3 othermax,glm::vec3 othermin){
        if(currentMax.x > othermin.x && othermax.x > currentMin.x &&
           currentMax.y > othermin.y && othermax.y > currentMin.y &&
           currentMax.z > othermin.z && othermax.z > currentMin.z){
            cout << "Collided" << endl;
        }
    }
};
class cursor_enter_callback{
public:
    GLFWcursor* hoverCursor = nullptr;
    GLFWcursor* grabCursor = nullptr;
    bool isMouseInWindow = false;

    void load_cursors(){
        hoverCursor = glfwCreateStandardCursor(GLFW_POINTING_HAND_CURSOR);

        int width,height,channels;
        unsigned char*data = stbi_load("/home/kiyotaka/atcoder/QtOpengl/images/swipe.png",&width,&height,&channels,4);
        if(data){
            GLFWimage image;
            image.width = width;
            image.height = height;
            image.pixels = data;
            grabCursor = glfwCreateCursor(&image, width/2, height/2);
            stbi_image_free(data);
        }else{
            cout << "Failed to load cursor image!" << endl;
        }
    }
    void on_cursor_enter(GLFWwindow* window, int entered){
        if(entered){
            isMouseInWindow = true;
            glfwSetCursor(window, hoverCursor);
        }else{
            isMouseInWindow = false;
            glfwSetCursor(window, nullptr);
        }     
    }
    void mouse_buuton_callback(GLFWwindow*window, int button, int action, int mods){
        if(isMouseInWindow&& button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
            glfwSetCursor(window, grabCursor);
    }else if(isMouseInWindow&& action == GLFW_RELEASE){
            glfwSetCursor(window, hoverCursor);
        }
    } 
};
cursor_enter_callback*global_cursor_callback = nullptr;
void c_on_cursor_enter(GLFWwindow*window, int entered){
    if(global_cursor_callback){
        global_cursor_callback->on_cursor_enter(window, entered);
    }
}
void c_mouse_button_callback(GLFWwindow*window, int button, int action, int mods){
    if(global_cursor_callback){
        global_cursor_callback->mouse_buuton_callback(window, button, action, mods);
    }
}
int main() {
    setenv("XCURSOR_THEME", "Adwaita", 1);
    setenv("XCURSOR_SIZE", "24", 1);
    setenv("XCURSOR_PATH", "/usr/share/icons", 1);
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    GLFWwindow* window = glfwCreateWindow(800, 600, " kiyotaka OpenGL", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);


    if (!gladLoadGL(glfwGetProcAddress)) {
    cout << "GLAD 初始化失敗！" << endl;
    return -1;
    }

    Display* display = glfwGetX11Display();
    Window xwindow = glfwGetX11Window(window);

    glfwWindowHint(GLFW_SAMPLES, 4);
    glEnable(GL_MULTISAMPLE);

    cout << "OpenGL 版本: " << glGetString(GL_VERSION) << endl;
    Model koharu("/home/kiyotaka/atcoder/QtOpengl/model/koharu/blue_archive_-koharu-__cb_default_emotion (1)/scene.gltf");

    cursor_enter_callback cursorCallback;
    cursorCallback.load_cursors();
    global_cursor_callback = &cursorCallback;

    //glfwSetCursorEnterCallback(window, c_on_cursor_enter);
    //glfwSetMouseButtonCallback(window, c_mouse_button_callback);
    glfwSetCursor(window, cursorCallback.hoverCursor); 
    cursorCallback.isMouseInWindow = true;

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);

    glEnable(GL_STENCIL_TEST);
    glCullFace(GL_BACK);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //初始化ImGui
    //------------------------------
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    io.MouseDrawCursor = false;
    (void)io;
    //設置ImGui樣式
    ImGui::StyleColorsDark();
    //設置ImGui平台/渲染器綁定
    ImGui_ImplGlfw_InitForOpenGL(window, false);
    glfwSetKeyCallback(window, ImGui_ImplGlfw_KeyCallback);
    glfwSetCharCallback(window, ImGui_ImplGlfw_CharCallback);
    glfwSetScrollCallback(window, ImGui_ImplGlfw_ScrollCallback);
    glfwSetMouseButtonCallback(window, ImGui_ImplGlfw_MouseButtonCallback);
    const char* glsl_version = "#version 330";
    ImGui_ImplOpenGL3_Init(glsl_version);
    //------------------------------

    float lightCoords[1296] = {
    0.0000f, 0.0500f, 0.0000f,
    0.0000f, 0.0500f, 0.0000f,
    0.0250f, 0.0433f, 0.0000f,
    0.0000f, 0.0500f, 0.0000f,
    0.0217f, 0.0433f, 0.0125f,
    0.0250f, 0.0433f, 0.0000f,
    0.0000f, 0.0500f, 0.0000f,
    0.0000f, 0.0500f, 0.0000f,
    0.0217f, 0.0433f, 0.0125f,
    0.0000f, 0.0500f, 0.0000f,
    0.0125f, 0.0433f, 0.0217f,
    0.0217f, 0.0433f, 0.0125f,

    0.0000f, 0.0500f, 0.0000f,
    0.0000f, 0.0500f, 0.0000f,
    0.0125f, 0.0433f, 0.0217f,
    0.0000f, 0.0500f, 0.0000f,
    0.0000f, 0.0433f, 0.0250f,
    0.0125f, 0.0433f, 0.0217f,
    0.0000f, 0.0500f, 0.0000f,
    0.0000f, 0.0500f, 0.0000f,
    0.0000f, 0.0433f, 0.0250f,
    0.0000f, 0.0500f, 0.0000f,
    -0.0125f, 0.0433f, 0.0217f,
    0.0000f, 0.0433f, 0.0250f,

    0.0000f, 0.0500f, 0.0000f,
    0.0000f, 0.0500f, 0.0000f,
    -0.0125f, 0.0433f, 0.0217f,
    0.0000f, 0.0500f, 0.0000f,
    -0.0217f, 0.0433f, 0.0125f,
    -0.0125f, 0.0433f, 0.0217f,
    0.0000f, 0.0500f, 0.0000f,
    0.0000f, 0.0500f, 0.0000f,
    -0.0217f, 0.0433f, 0.0125f,
    0.0000f, 0.0500f, 0.0000f,
    -0.0250f, 0.0433f, 0.0000f,
    -0.0217f, 0.0433f, 0.0125f,

    0.0000f, 0.0500f, 0.0000f,
    0.0000f, 0.0500f, 0.0000f,
    -0.0250f, 0.0433f, 0.0000f,
    0.0000f, 0.0500f, 0.0000f,
    -0.0217f, 0.0433f, -0.0125f,
    -0.0250f, 0.0433f, 0.0000f,
    0.0000f, 0.0500f, 0.0000f,
    0.0000f, 0.0500f, 0.0000f,
    -0.0217f, 0.0433f, -0.0125f,
    0.0000f, 0.0500f, 0.0000f,
    -0.0125f, 0.0433f, -0.0217f,
    -0.0217f, 0.0433f, -0.0125f,

    0.0000f, 0.0500f, 0.0000f,
    0.0000f, 0.0500f, 0.0000f,
    -0.0125f, 0.0433f, -0.0217f,
    0.0000f, 0.0500f, 0.0000f,
    -0.0000f, 0.0433f, -0.0250f,
    -0.0125f, 0.0433f, -0.0217f,
    0.0000f, 0.0500f, 0.0000f,
    0.0000f, 0.0500f, 0.0000f,
    -0.0000f, 0.0433f, -0.0250f,
    0.0000f, 0.0500f, 0.0000f,
    0.0125f, 0.0433f, -0.0217f,
    -0.0000f, 0.0433f, -0.0250f,

    0.0000f, 0.0500f, 0.0000f,
    0.0000f, 0.0500f, 0.0000f,
    0.0125f, 0.0433f, -0.0217f,
    0.0000f, 0.0500f, 0.0000f,
    0.0217f, 0.0433f, -0.0125f,
    0.0125f, 0.0433f, -0.0217f,
    0.0000f, 0.0500f, 0.0000f,
    0.0000f, 0.0500f, 0.0000f,
    0.0217f, 0.0433f, -0.0125f,
    0.0000f, 0.0500f, 0.0000f,
    0.0250f, 0.0433f, -0.0000f,
    0.0217f, 0.0433f, -0.0125f,

    0.0250f, 0.0433f, 0.0000f,
    0.0217f, 0.0433f, 0.0125f,
    0.0433f, 0.0250f, 0.0000f,
    0.0217f, 0.0433f, 0.0125f,
    0.0375f, 0.0250f, 0.0217f,
    0.0433f, 0.0250f, 0.0000f,
    0.0217f, 0.0433f, 0.0125f,
    0.0125f, 0.0433f, 0.0217f,
    0.0375f, 0.0250f, 0.0217f,
    0.0125f, 0.0433f, 0.0217f,
    0.0217f, 0.0250f, 0.0375f,
    0.0375f, 0.0250f, 0.0217f,

    0.0125f, 0.0433f, 0.0217f,
    0.0000f, 0.0433f, 0.0250f,
    0.0217f, 0.0250f, 0.0375f,
    0.0000f, 0.0433f, 0.0250f,
    0.0000f, 0.0250f, 0.0433f,
    0.0217f, 0.0250f, 0.0375f,
    0.0000f, 0.0433f, 0.0250f,
    -0.0125f, 0.0433f, 0.0217f,
    0.0000f, 0.0250f, 0.0433f,
    -0.0125f, 0.0433f, 0.0217f,
    -0.0217f, 0.0250f, 0.0375f,
    0.0000f, 0.0250f, 0.0433f,

    -0.0125f, 0.0433f, 0.0217f,
    -0.0217f, 0.0433f, 0.0125f,
    -0.0217f, 0.0250f, 0.0375f,
    -0.0217f, 0.0433f, 0.0125f,
    -0.0375f, 0.0250f, 0.0217f,
    -0.0217f, 0.0250f, 0.0375f,
    -0.0217f, 0.0433f, 0.0125f,
    -0.0250f, 0.0433f, 0.0000f,
    -0.0375f, 0.0250f, 0.0217f,
    -0.0250f, 0.0433f, 0.0000f,
    -0.0433f, 0.0250f, 0.0000f,
    -0.0375f, 0.0250f, 0.0217f,

    -0.0250f, 0.0433f, 0.0000f,
    -0.0217f, 0.0433f, -0.0125f,
    -0.0433f, 0.0250f, 0.0000f,
    -0.0217f, 0.0433f, -0.0125f,
    -0.0375f, 0.0250f, -0.0217f,
    -0.0433f, 0.0250f, 0.0000f,
    -0.0217f, 0.0433f, -0.0125f,
    -0.0125f, 0.0433f, -0.0217f,
    -0.0375f, 0.0250f, -0.0217f,
    -0.0125f, 0.0433f, -0.0217f,
    -0.0217f, 0.0250f, -0.0375f,
    -0.0375f, 0.0250f, -0.0217f,

    -0.0125f, 0.0433f, -0.0217f,
    -0.0000f, 0.0433f, -0.0250f,
    -0.0217f, 0.0250f, -0.0375f,
    -0.0000f, 0.0433f, -0.0250f,
    -0.0000f, 0.0250f, -0.0433f,
    -0.0217f, 0.0250f, -0.0375f,
    -0.0000f, 0.0433f, -0.0250f,
    0.0125f, 0.0433f, -0.0217f,
    -0.0000f, 0.0250f, -0.0433f,
    0.0125f, 0.0433f, -0.0217f,
    0.0217f, 0.0250f, -0.0375f,
    -0.0000f, 0.0250f, -0.0433f,

    0.0125f, 0.0433f, -0.0217f,
    0.0217f, 0.0433f, -0.0125f,
    0.0217f, 0.0250f, -0.0375f,
    0.0217f, 0.0433f, -0.0125f,
    0.0375f, 0.0250f, -0.0217f,
    0.0217f, 0.0250f, -0.0375f,
    0.0217f, 0.0433f, -0.0125f,
    0.0250f, 0.0433f, -0.0000f,
    0.0375f, 0.0250f, -0.0217f,
    0.0250f, 0.0433f, -0.0000f,
    0.0433f, 0.0250f, -0.0000f,
    0.0375f, 0.0250f, -0.0217f,

    0.0433f, 0.0250f, 0.0000f,
    0.0375f, 0.0250f, 0.0217f,
    0.0500f, 0.0000f, 0.0000f,
    0.0375f, 0.0250f, 0.0217f,
    0.0433f, 0.0000f, 0.0250f,
    0.0500f, 0.0000f, 0.0000f,
    0.0375f, 0.0250f, 0.0217f,
    0.0217f, 0.0250f, 0.0375f,
    0.0433f, 0.0000f, 0.0250f,
    0.0217f, 0.0250f, 0.0375f,
    0.0250f, 0.0000f, 0.0433f,
    0.0433f, 0.0000f, 0.0250f,

    0.0217f, 0.0250f, 0.0375f,
    0.0000f, 0.0250f, 0.0433f,
    0.0250f, 0.0000f, 0.0433f,
    0.0000f, 0.0250f, 0.0433f,
    0.0000f, 0.0000f, 0.0500f,
    0.0250f, 0.0000f, 0.0433f,
    0.0000f, 0.0250f, 0.0433f,
    -0.0217f, 0.0250f, 0.0375f,
    0.0000f, 0.0000f, 0.0500f,
    -0.0217f, 0.0250f, 0.0375f,
    -0.0250f, 0.0000f, 0.0433f,
    0.0000f, 0.0000f, 0.0500f,

    -0.0217f, 0.0250f, 0.0375f,
    -0.0375f, 0.0250f, 0.0217f,
    -0.0250f, 0.0000f, 0.0433f,
    -0.0375f, 0.0250f, 0.0217f,
    -0.0433f, 0.0000f, 0.0250f,
    -0.0250f, 0.0000f, 0.0433f,
    -0.0375f, 0.0250f, 0.0217f,
    -0.0433f, 0.0250f, 0.0000f,
    -0.0433f, 0.0000f, 0.0250f,
    -0.0433f, 0.0250f, 0.0000f,
    -0.0500f, 0.0000f, 0.0000f,
    -0.0433f, 0.0000f, 0.0250f,

    -0.0433f, 0.0250f, 0.0000f,
    -0.0375f, 0.0250f, -0.0217f,
    -0.0500f, 0.0000f, 0.0000f,
    -0.0375f, 0.0250f, -0.0217f,
    -0.0433f, 0.0000f, -0.0250f,
    -0.0500f, 0.0000f, 0.0000f,
    -0.0375f, 0.0250f, -0.0217f,
    -0.0217f, 0.0250f, -0.0375f,
    -0.0433f, 0.0000f, -0.0250f,
    -0.0217f, 0.0250f, -0.0375f,
    -0.0250f, 0.0000f, -0.0433f,
    -0.0433f, 0.0000f, -0.0250f,

    -0.0217f, 0.0250f, -0.0375f,
    -0.0000f, 0.0250f, -0.0433f,
    -0.0250f, 0.0000f, -0.0433f,
    -0.0000f, 0.0250f, -0.0433f,
    -0.0000f, 0.0000f, -0.0500f,
    -0.0250f, 0.0000f, -0.0433f,
    -0.0000f, 0.0250f, -0.0433f,
    0.0217f, 0.0250f, -0.0375f,
    -0.0000f, 0.0000f, -0.0500f,
    0.0217f, 0.0250f, -0.0375f,
    0.0250f, 0.0000f, -0.0433f,
    -0.0000f, 0.0000f, -0.0500f,

    0.0217f, 0.0250f, -0.0375f,
    0.0375f, 0.0250f, -0.0217f,
    0.0250f, 0.0000f, -0.0433f,
    0.0375f, 0.0250f, -0.0217f,
    0.0433f, 0.0000f, -0.0250f,
    0.0250f, 0.0000f, -0.0433f,
    0.0375f, 0.0250f, -0.0217f,
    0.0433f, 0.0250f, -0.0000f,
    0.0433f, 0.0000f, -0.0250f,
    0.0433f, 0.0250f, -0.0000f,
    0.0500f, 0.0000f, -0.0000f,
    0.0433f, 0.0000f, -0.0250f,

    0.0500f, 0.0000f, 0.0000f,
    0.0433f, 0.0000f, 0.0250f,
    0.0433f, -0.0250f, 0.0000f,
    0.0433f, 0.0000f, 0.0250f,
    0.0375f, -0.0250f, 0.0217f,
    0.0433f, -0.0250f, 0.0000f,
    0.0433f, 0.0000f, 0.0250f,
    0.0250f, 0.0000f, 0.0433f,
    0.0375f, -0.0250f, 0.0217f,
    0.0250f, 0.0000f, 0.0433f,
    0.0217f, -0.0250f, 0.0375f,
    0.0375f, -0.0250f, 0.0217f,

    0.0250f, 0.0000f, 0.0433f,
    0.0000f, 0.0000f, 0.0500f,
    0.0217f, -0.0250f, 0.0375f,
    0.0000f, 0.0000f, 0.0500f,
    0.0000f, -0.0250f, 0.0433f,
    0.0217f, -0.0250f, 0.0375f,
    0.0000f, 0.0000f, 0.0500f,
    -0.0250f, 0.0000f, 0.0433f,
    0.0000f, -0.0250f, 0.0433f,
    -0.0250f, 0.0000f, 0.0433f,
    -0.0217f, -0.0250f, 0.0375f,
    0.0000f, -0.0250f, 0.0433f,

    -0.0250f, 0.0000f, 0.0433f,
    -0.0433f, 0.0000f, 0.0250f,
    -0.0217f, -0.0250f, 0.0375f,
    -0.0433f, 0.0000f, 0.0250f,
    -0.0375f, -0.0250f, 0.0217f,
    -0.0217f, -0.0250f, 0.0375f,
    -0.0433f, 0.0000f, 0.0250f,
    -0.0500f, 0.0000f, 0.0000f,
    -0.0375f, -0.0250f, 0.0217f,
    -0.0500f, 0.0000f, 0.0000f,
    -0.0433f, -0.0250f, 0.0000f,
    -0.0375f, -0.0250f, 0.0217f,

    -0.0500f, 0.0000f, 0.0000f,
    -0.0433f, 0.0000f, -0.0250f,
    -0.0433f, -0.0250f, 0.0000f,
    -0.0433f, 0.0000f, -0.0250f,
    -0.0375f, -0.0250f, -0.0217f,
    -0.0433f, -0.0250f, 0.0000f,
    -0.0433f, 0.0000f, -0.0250f,
    -0.0250f, 0.0000f, -0.0433f,
    -0.0375f, -0.0250f, -0.0217f,
    -0.0250f, 0.0000f, -0.0433f,
    -0.0217f, -0.0250f, -0.0375f,
    -0.0375f, -0.0250f, -0.0217f,

    -0.0250f, 0.0000f, -0.0433f,
    -0.0000f, 0.0000f, -0.0500f,
    -0.0217f, -0.0250f, -0.0375f,
    -0.0000f, 0.0000f, -0.0500f,
    -0.0000f, -0.0250f, -0.0433f,
    -0.0217f, -0.0250f, -0.0375f,
    -0.0000f, 0.0000f, -0.0500f,
    0.0250f, 0.0000f, -0.0433f,
    -0.0000f, -0.0250f, -0.0433f,
    0.0250f, 0.0000f, -0.0433f,
    0.0217f, -0.0250f, -0.0375f,
    -0.0000f, -0.0250f, -0.0433f,

    0.0250f, 0.0000f, -0.0433f,
    0.0433f, 0.0000f, -0.0250f,
    0.0217f, -0.0250f, -0.0375f,
    0.0433f, 0.0000f, -0.0250f,
    0.0375f, -0.0250f, -0.0217f,
    0.0217f, -0.0250f, -0.0375f,
    0.0433f, 0.0000f, -0.0250f,
    0.0500f, 0.0000f, -0.0000f,
    0.0375f, -0.0250f, -0.0217f,
    0.0500f, 0.0000f, -0.0000f,
    0.0433f, -0.0250f, -0.0000f,
    0.0375f, -0.0250f, -0.0217f,

    0.0433f, -0.0250f, 0.0000f,
    0.0375f, -0.0250f, 0.0217f,
    0.0250f, -0.0433f, 0.0000f,
    0.0375f, -0.0250f, 0.0217f,
    0.0217f, -0.0433f, 0.0125f,
    0.0250f, -0.0433f, 0.0000f,
    0.0375f, -0.0250f, 0.0217f,
    0.0217f, -0.0250f, 0.0375f,
    0.0217f, -0.0433f, 0.0125f,
    0.0217f, -0.0250f, 0.0375f,
    0.0125f, -0.0433f, 0.0217f,
    0.0217f, -0.0433f, 0.0125f,

    0.0217f, -0.0250f, 0.0375f,
    0.0000f, -0.0250f, 0.0433f,
    0.0125f, -0.0433f, 0.0217f,
    0.0000f, -0.0250f, 0.0433f,
    0.0000f, -0.0433f, 0.0250f,
    0.0125f, -0.0433f, 0.0217f,
    0.0000f, -0.0250f, 0.0433f,
    -0.0217f, -0.0250f, 0.0375f,
    0.0000f, -0.0433f, 0.0250f,
    -0.0217f, -0.0250f, 0.0375f,
    -0.0125f, -0.0433f, 0.0217f,
    0.0000f, -0.0433f, 0.0250f,

    -0.0217f, -0.0250f, 0.0375f,
    -0.0375f, -0.0250f, 0.0217f,
    -0.0125f, -0.0433f, 0.0217f,
    -0.0375f, -0.0250f, 0.0217f,
    -0.0217f, -0.0433f, 0.0125f,
    -0.0125f, -0.0433f, 0.0217f,
    -0.0375f, -0.0250f, 0.0217f,
    -0.0433f, -0.0250f, 0.0000f,
    -0.0217f, -0.0433f, 0.0125f,
    -0.0433f, -0.0250f, 0.0000f,
    -0.0250f, -0.0433f, 0.0000f,
    -0.0217f, -0.0433f, 0.0125f,

    -0.0433f, -0.0250f, 0.0000f,
    -0.0375f, -0.0250f, -0.0217f,
    -0.0250f, -0.0433f, 0.0000f,
    -0.0375f, -0.0250f, -0.0217f,
    -0.0217f, -0.0433f, -0.0125f,
    -0.0250f, -0.0433f, 0.0000f,
    -0.0375f, -0.0250f, -0.0217f,
    -0.0217f, -0.0250f, -0.0375f,
    -0.0217f, -0.0433f, -0.0125f,
    -0.0217f, -0.0250f, -0.0375f,
    -0.0125f, -0.0433f, -0.0217f,
    -0.0217f, -0.0433f, -0.0125f,

    -0.0217f, -0.0250f, -0.0375f,
    -0.0000f, -0.0250f, -0.0433f,
    -0.0125f, -0.0433f, -0.0217f,
    -0.0000f, -0.0250f, -0.0433f,
    -0.0000f, -0.0433f, -0.0250f,
    -0.0125f, -0.0433f, -0.0217f,
    -0.0000f, -0.0250f, -0.0433f,
    0.0217f, -0.0250f, -0.0375f,
    -0.0000f, -0.0433f, -0.0250f,
    0.0217f, -0.0250f, -0.0375f,
    0.0125f, -0.0433f, -0.0217f,
    -0.0000f, -0.0433f, -0.0250f,

    0.0217f, -0.0250f, -0.0375f,
    0.0375f, -0.0250f, -0.0217f,
    0.0125f, -0.0433f, -0.0217f,
    0.0375f, -0.0250f, -0.0217f,
    0.0217f, -0.0433f, -0.0125f,
    0.0125f, -0.0433f, -0.0217f,
    0.0375f, -0.0250f, -0.0217f,
    0.0433f, -0.0250f, -0.0000f,
    0.0217f, -0.0433f, -0.0125f,
    0.0433f, -0.0250f, -0.0000f,
    0.0250f, -0.0433f, -0.0000f,
    0.0217f, -0.0433f, -0.0125f,

    0.0250f, -0.0433f, 0.0000f,
    0.0217f, -0.0433f, 0.0125f,
    0.0000f, -0.0500f, 0.0000f,
    0.0217f, -0.0433f, 0.0125f,
    0.0000f, -0.0500f, 0.0000f,
    0.0000f, -0.0500f, 0.0000f,
    0.0217f, -0.0433f, 0.0125f,
    0.0125f, -0.0433f, 0.0217f,
    0.0000f, -0.0500f, 0.0000f,
    0.0125f, -0.0433f, 0.0217f,
    0.0000f, -0.0500f, 0.0000f,
    0.0000f, -0.0500f, 0.0000f,

    0.0125f, -0.0433f, 0.0217f,
    0.0000f, -0.0433f, 0.0250f,
    0.0000f, -0.0500f, 0.0000f,
    0.0000f, -0.0433f, 0.0250f,
    0.0000f, -0.0500f, 0.0000f,
    0.0000f, -0.0500f, 0.0000f,
    0.0000f, -0.0433f, 0.0250f,
    -0.0125f, -0.0433f, 0.0217f,
    0.0000f, -0.0500f, 0.0000f,
    -0.0125f, -0.0433f, 0.0217f,
    -0.0000f, -0.0500f, 0.0000f,
    0.0000f, -0.0500f, 0.0000f,

    -0.0125f, -0.0433f, 0.0217f,
    -0.0217f, -0.0433f, 0.0125f,
    -0.0000f, -0.0500f, 0.0000f,
    -0.0217f, -0.0433f, 0.0125f,
    -0.0000f, -0.0500f, 0.0000f,
    -0.0000f, -0.0500f, 0.0000f,
    -0.0217f, -0.0433f, 0.0125f,
    -0.0250f, -0.0433f, 0.0000f,
    -0.0000f, -0.0500f, 0.0000f,
    -0.0250f, -0.0433f, 0.0000f,
    -0.0000f, -0.0500f, 0.0000f,
    -0.0000f, -0.0500f, 0.0000f,

    -0.0250f, -0.0433f, 0.0000f,
    -0.0217f, -0.0433f, -0.0125f,
    -0.0000f, -0.0500f, 0.0000f,
    -0.0217f, -0.0433f, -0.0125f,
    -0.0000f, -0.0500f, -0.0000f,
    -0.0000f, -0.0500f, 0.0000f,
    -0.0217f, -0.0433f, -0.0125f,
    -0.0125f, -0.0433f, -0.0217f,
    -0.0000f, -0.0500f, -0.0000f,
    -0.0125f, -0.0433f, -0.0217f,
    -0.0000f, -0.0500f, -0.0000f,
    -0.0000f, -0.0500f, -0.0000f,

    -0.0125f, -0.0433f, -0.0217f,
    -0.0000f, -0.0433f, -0.0250f,
    -0.0000f, -0.0500f, -0.0000f,
    -0.0000f, -0.0433f, -0.0250f,
    -0.0000f, -0.0500f, -0.0000f,
    -0.0000f, -0.0500f, -0.0000f,
    -0.0000f, -0.0433f, -0.0250f,
    0.0125f, -0.0433f, -0.0217f,
    -0.0000f, -0.0500f, -0.0000f,
    0.0125f, -0.0433f, -0.0217f,
    0.0000f, -0.0500f, -0.0000f,
    -0.0000f, -0.0500f, -0.0000f,

    0.0125f, -0.0433f, -0.0217f,
    0.0217f, -0.0433f, -0.0125f,
    0.0000f, -0.0500f, -0.0000f,
    0.0217f, -0.0433f, -0.0125f,
    0.0000f, -0.0500f, -0.0000f,
    0.0000f, -0.0500f, -0.0000f,
    0.0217f, -0.0433f, -0.0125f,
    0.0250f, -0.0433f, -0.0000f,
    0.0000f, -0.0500f, -0.0000f,
    0.0250f, -0.0433f, -0.0000f,
    0.0000f, -0.0500f, -0.0000f,
    0.0000f, -0.0500f, -0.0000f,
    };
    unsigned int fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    unsigned int framebuffer;
    glGenTextures(1, &framebuffer);
    glBindTexture(GL_TEXTURE_2D, framebuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 800, 600, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer, 0);

    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    unsigned int lightVAO, lightVBO;
    glGenBuffers(1, &lightVBO);
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lightCoords), lightCoords, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    ShaderProgramSource source = ParseShader("/home/kiyotaka/atcoder/QtOpengl/res/shaders/Basic.shader");
    unsigned int shader = CreateShader(source.VertexSource,source.FragmentSource);
    glUseProgram(shader);
    ShaderProgramSource lightSource = ParseShader("/home/kiyotaka/atcoder/QtOpengl/res/shaders/Light.shader");
    unsigned int lightShader = CreateShader(lightSource.VertexSource, lightSource.FragmentSource);
    glUseProgram(lightShader);
    ShaderProgramSource Outline = ParseShader("/home/kiyotaka/atcoder/QtOpengl/res/shaders/Outline.shader");
    unsigned int outlineShader = CreateShader(Outline.VertexSource, Outline.FragmentSource);
    glUseProgram(outlineShader);
    ShaderProgramSource fbosource = ParseShader("/home/kiyotaka/atcoder/QtOpengl/res/shaders/Fbo.shader");
    unsigned int Fbo = CreateShader(fbosource.VertexSource, fbosource.FragmentSource);
    glUseProgram(Fbo);
    glUniform1i(glGetUniformLocation(Fbo, "screenTexture"), 0);
    //生成紋理
    unsigned int texture1,texture2;
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //圖像y軸翻轉
    stbi_set_flip_vertically_on_load(true);
    
    //加載圖片
    int width,height,nrChannels;
    unsigned char*data = stbi_load("/home/kiyotaka/atcoder/QtOpengl/images/container.png", &width, &height, &nrChannels, 0);
    if(data){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }else{
        cout << "Failed to load texture" << endl;
    }
    stbi_image_free(data);

    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    data = stbi_load("/home/kiyotaka/atcoder/QtOpengl/images/awesomeface.png", &width, &height, &nrChannels, 0);
    if(data){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }else{
        cout << "Failed to load texture" << endl;
    }
    stbi_image_free(data);
    
    glUseProgram(shader);
    glUniform1i(glGetUniformLocation(shader, "texture1"), 0);
    glUniform1i(glGetUniformLocation(shader, "texture2"), 1);

    glm::mat4 model = glm :: mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);
    
    glUseProgram(shader);
    int modelLoc = glGetUniformLocation(shader, "model");
    int viewLoc = glGetUniformLocation(shader, "view");
    int projectionLoc = glGetUniformLocation(shader, "projection");
    //int lightPosLoc = glGetUniformLocation(shader, "lightPos");
    glUniform1f(glGetUniformLocation(shader, "ambientStrength"), 0.4f);
    int viewPosLoc = glGetUniformLocation(shader, "viewPos");
    glUniform3f(viewPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);
    int lightDirLoc = glGetUniformLocation(shader, "lightDir");

    glUseProgram(lightShader);
    glUniform3f(glGetUniformLocation(lightShader, "lightColor"), 1.0f, 1.0f, 1.0f);
    int lightModelLoc = glGetUniformLocation(lightShader, "model");


    model = glm::scale(model, glm::vec3(0.00005f));
    model = glm::rotate(model,glm::radians(180.0f),glm::vec3(1.0f,0.0f,0.0f));
    model = glm::translate(model, glm::vec3(-18000.0f, -30000.0f, -3000.0f));
    PhysicsObject koharu1(model,koharu.localmax,koharu.localmin);

    //glfwSetFramebufferSizeCallback(window, change_size);
    //glfwSetCursorPosCallback(window, mouse_callback);

    float radius = 3.0f;
    glm :: vec3 clear_color = glm::vec3(1.0, 0.7, 0.97);
    while (!glfwWindowShouldClose(window)) {
        
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glEnable(GL_DEPTH_TEST);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if(!io.WantCaptureMouse){
            double xpos, ypos;

            glfwGetCursorPos(window, &xpos, &ypos);
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
            {
                glfwSetCursor(window, cursorCallback.grabCursor);
                if(firstMouse){
                    lastX = xpos;
                    lastY = ypos;
                    firstMouse = false;
                }
                float xoffest = xpos - lastX;
                float yoffest = lastY - ypos;
                lastX = xpos;
                lastY = ypos;

                float sensitivity = 0.2f;
                yaw += xoffest * sensitivity;
                pitch += yoffest * sensitivity;

                if(pitch > 89.0f)
                    pitch = 89.0f;
                if(pitch < -89.0f)
                    pitch = -89.0f;
            }else{
                glfwSetCursor(window, cursorCallback.hoverCursor);
                firstMouse = true;
            }    
        }

        float camx = radius * glm::cos(glm::radians(pitch))*glm::cos(glm::radians(yaw));
        float camy = radius * glm::sin(glm::radians(pitch));
        float camz = radius * glm::cos(glm::radians(pitch))*glm::sin(glm::radians(yaw));
        glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
        cameraPos = target + glm::vec3(camx, camy, camz);
        view = glm::lookAt(cameraPos, target, cameraUp);

        float f  = 0.0f;
        ImGui::Begin("Hello, ImGui!");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::ColorEdit3("Background Color", glm::value_ptr(clear_color));
        ImGui::End();

        glClearColor(clear_color.x, clear_color.y, clear_color.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, 1.0f);

        glUseProgram(shader);
        //processInput(window);
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        koharu1.UpdatePhysics(deltaTime,model);

        glm::mat4 projection = glm::mat4(1.0f);
        //glm::mat4 view;

        //view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        //glm::vec3 lightPos = glm::vec3(1.6f*glm::sin(glfwGetTime()), 2.0f, 0.0f);
        glm::vec3 lightDir = glm::vec3(2.0f, -3.0f, 0.0f);

        glUniform3f(viewPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        //glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));
        glUniform3f(lightDirLoc, lightDir.x, lightDir.y, lightDir.z);

        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

        glCullFace(GL_BACK);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        koharu.Draw(shader);
        glEnable(GL_CULL_FACE);
        glDisable(GL_BLEND);

        glUseProgram(outlineShader);
        glUniformMatrix4fv(glGetUniformLocation(outlineShader, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(outlineShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(outlineShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        
        glDisable(GL_CULL_FACE);
        koharu.DrawOutlinePass(outlineShader);
        glEnable(GL_CULL_FACE);

        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glEnable(GL_CULL_FACE);

        glUseProgram(lightShader);
        glBindVertexArray(lightVAO);
        glm::mat4 modelLight = glm::mat4(1.0f);
        //modelLight = glm::translate(modelLight, lightPos);
        glUniformMatrix4fv(lightModelLoc, 1, GL_FALSE, glm::value_ptr(modelLight));
        glUniformMatrix4fv(glGetUniformLocation(lightShader, "view"),1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(lightShader, "projection"),1, GL_FALSE, glm::value_ptr(projection));
        //glDrawArrays(GL_TRIANGLE_FAN, 0, 432);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(Fbo);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, framebuffer);
        renderQuad();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteProgram(shader);
    glDeleteProgram(outlineShader);
    glDeleteProgram(Fbo);
    glDeleteProgram(lightShader);
    glfwDestroyCursor(cursorCallback.grabCursor);
    glfwDestroyCursor(cursorCallback.hoverCursor);
    glfwTerminate();
    return 0;
}


// 舊（錯誤）：在 X11 作業系統層級強制隱藏游標，GLFW 的 glfwSetCursor() 完全失效
//XFixesHideCursor(display, xwindow);
//XFlush(display);

// 修正：移除這兩行，改加說明注釋