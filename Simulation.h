#ifndef SIMULATION_H
#define SIMULATION_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <random>
#include <btBulletDynamicsCommon.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <memory>
#include "Sphere.h"
#include "Shader.h"

class Simulation {
public:
    Simulation();
    ~Simulation();

    void run();

private:
    // 窗口尺寸
    static const unsigned int SCR_WIDTH = 2560;
    static const unsigned int SCR_HEIGHT = 1440;

    // 窗口和OpenGL对象
    GLFWwindow* window = nullptr;

    // 着色器（使用智能指针）
    std::unique_ptr<Shader> simpleShader;
    std::unique_ptr<Shader> axesShader;

    // 物理引擎组件（使用智能指针）
    std::unique_ptr<btDefaultCollisionConfiguration> collisionConfiguration;
    std::unique_ptr<btCollisionDispatcher> dispatcher;
    std::unique_ptr<btDbvtBroadphase> broadphase;
    std::unique_ptr<btSequentialImpulseConstraintSolver> solver;
    std::unique_ptr<btDiscreteDynamicsWorld> dynamicsWorld;
    std::vector<std::unique_ptr<Sphere>> spheres;

    // 万有引力常数
    const float G = 6.67430e-5f;

    // 时间管理
    float accumulatedTime = 0.0f;
    float lastFrameTime = 0.0f;

    // 相机
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 20.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    float yaw = -90.0f;
    float pitch = 0.0f;
    float lastX = SCR_WIDTH / 2.0f;
    float lastY = SCR_HEIGHT / 2.0f;
    bool firstMouse = true;

    glm::mat4 projectionMatrix;

    // 坐标轴
    GLuint axesVAO, axesVBO;

    // 初始化函数
    void initPhysics();
    void initOpenGL();
    void initAxes();
    void initProjectionMatrix();

    // 回调函数
    static void mouseCallbackWrapper(GLFWwindow* window, double xpos, double ypos);
    void mouseCallback(double xpos, double ypos);

    // 渲染函数
    void renderAxes();
    void renderSphere(const Sphere& sphere);

    // 物理计算
    void applyGravity(Sphere* sphere1, Sphere* sphere2);
    void applyGravityToAll();
    void resetSpheres();
    void cleanupPhysics();

    // 输入处理
    void processInput(float deltaTime);

    // 更新和渲染
    void update(float deltaTime);
    void render();
};

#endif