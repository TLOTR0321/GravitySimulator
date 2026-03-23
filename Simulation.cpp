#include "Simulation.h"
#include <iostream>

// 构造函数
Simulation::Simulation() = default;

// 析构函数 - 现在自动管理内存
Simulation::~Simulation() {
    // 清理物理世界
    cleanupPhysics();

    glDeleteVertexArrays(1, &axesVAO);
    glDeleteBuffers(1, &axesVBO);

    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}

// 初始化投影矩阵（只调用一次）
void Simulation::initProjectionMatrix() {
    projectionMatrix = glm::perspective(
        glm::radians(45.0f),
        (float)SCR_WIDTH / (float)SCR_HEIGHT,
        0.1f, 100.0f
    );
}

// 运行模拟
void Simulation::run() {
    // 初始化GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return;
    }

    // 设置OpenGL版本和核心模式
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Gravity Simulation - Modern OpenGL", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window);

    // 设置用户指针和回调函数
    glfwSetWindowUserPointer(window, this);
    glfwSetCursorPosCallback(window, Simulation::mouseCallbackWrapper);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // 初始化GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return;
    }

    // 设置视口
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    // 初始化OpenGL设置和着色器
    initOpenGL();
    initProjectionMatrix();
    initAxes();

    // 初始化物理引擎
    initPhysics();

    // 主循环
    lastFrameTime = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;

        processInput(deltaTime);
        update(deltaTime);
        render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

// 初始化OpenGL设置和着色器
void Simulation::initOpenGL() {
    // 启用深度测试
    glEnable(GL_DEPTH_TEST);
    // 创建着色器（使用make_unique）
    simpleShader = std::make_unique<Shader>("Shader.vert", "Shader.frag");
    axesShader = std::make_unique<Shader>("Axes.vert", "Axes.frag");

    // 预取常用uniform位置
    simpleShader->prefetchCommonUniforms();
    axesShader->prefetchCommonUniforms();
    // 设置清除颜色
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
}

// 初始化坐标轴
void Simulation::initAxes() {
    // 坐标轴顶点数据：位置(3) + 颜色(3)
    float axesVertices[] = {
        // X轴（红色）
        0.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f,
        5.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f,

        // Y轴（绿色）
        0.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f,
        0.0f, 5.0f, 0.0f,   0.0f, 1.0f, 0.0f,

        // Z轴（蓝色）
        0.0f, 0.0f, 0.0f,   0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 5.0f,   0.0f, 0.0f, 1.0f
    };

    glGenVertexArrays(1, &axesVAO);
    glGenBuffers(1, &axesVBO);

    glBindVertexArray(axesVAO);
    glBindBuffer(GL_ARRAY_BUFFER, axesVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axesVertices), axesVertices, GL_STATIC_DRAW);

    // 位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 颜色属性
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

// 初始化物理引擎
void Simulation::initPhysics() {
    // 碰撞配置
    collisionConfiguration = std::make_unique<btDefaultCollisionConfiguration>();
    dispatcher = std::make_unique<btCollisionDispatcher>(collisionConfiguration.get());
    broadphase = std::make_unique<btDbvtBroadphase>();
    solver = std::make_unique<btSequentialImpulseConstraintSolver>();
    // 动态世界
    dynamicsWorld = std::make_unique<btDiscreteDynamicsWorld>(
        dispatcher.get(),
        broadphase.get(),
        solver.get(),
        collisionConfiguration.get()
    );
    dynamicsWorld->setGravity(btVector3(0, 0, 0));
    // 随机数初始化
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posDist(-5.0f, 5.0f); // 位置范围
    std::uniform_real_distribution<float> massDist(1.0f, 100.0f);   // 质量范围
    std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);   // 颜色范围
    // 生成100个随机球体
    int numSpheres = 50;
    float radius = 0.2f;  // 所有球体大小相同

    for (int i = 0; i < numSpheres; ++i) {
        // 生成随机位置
        glm::vec3 randomPos(
            posDist(gen),
            posDist(gen),
            posDist(gen)
        );
        // 生成随机质量
        float randomMass = massDist(gen);
        // 生成随机颜色
        glm::vec3 randomColor(
            colorDist(gen),
            colorDist(gen),
            colorDist(gen)
        );
        // 创建球体
        spheres.emplace_back(std::make_unique<Sphere>(
            dynamicsWorld.get(),
            radius,              // 固定半径
            randomMass,          // 随机质量
            randomPos,           // 随机位置
            randomColor          // 随机颜色
        ));
        // 设置初始速度为0
        spheres.back()->rigidBody->setLinearVelocity(btVector3(0, 0, 0));
    }
    std::cout << "Generated " << numSpheres << " random spheres" << std::endl;
}

// 清理物理引擎（简化版，智能指针自动管理）
void Simulation::cleanupPhysics() {
    // 先停止模拟
    for (auto& sphere : spheres) {
        if (dynamicsWorld && sphere->rigidBody) {
            dynamicsWorld->removeRigidBody(sphere->rigidBody.get());
        }
    }

    // 清空 spheres 容器（这会触发 Sphere 析构函数）
    spheres.clear();

    // 最后销毁 dynamicsWorld 和其他 Bullet 对象
    // 这些 unique_ptr 会自动销毁，但顺序很重要
    // 确保先销毁 dynamicsWorld，再销毁其他组件
    dynamicsWorld.reset();
    solver.reset();
    broadphase.reset();
    dispatcher.reset();
    collisionConfiguration.reset();
}

// 鼠标回调包装器
void Simulation::mouseCallbackWrapper(GLFWwindow* window, double xpos, double ypos) {
    Simulation* sim = static_cast<Simulation*>(glfwGetWindowUserPointer(window));
    if (sim) {
        sim->mouseCallback(xpos, ypos);
    }
}

// 鼠标回调
void Simulation::mouseCallback(double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // 限制俯仰角
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    // 计算新的前向向量
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

// 处理输入
void Simulation::processInput(float deltaTime) {
    if (!window) return;

    // ESC退出
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // 相机移动速度
    float cameraSpeed = 5.0f * deltaTime;

    // 相机移动控制
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraUp;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraUp;

    // R键重置球体位置
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        resetSpheres();
    }
}

// 应用万有引力
void Simulation::applyGravity(Sphere* sphere1, Sphere* sphere2) {
    if (!sphere1 || !sphere2) return;

    glm::vec3 pos1 = sphere1->getPosition();
    glm::vec3 pos2 = sphere2->getPosition();

    glm::vec3 direction = pos2 - pos1;
    float distance = glm::length(direction);

    if (distance < 0.001f) return;

    direction = glm::normalize(direction);

    float mass1 = 1.0f / sphere1->rigidBody->getInvMass();
    float mass2 = 1.0f / sphere2->rigidBody->getInvMass();
    float forceMagnitude = G * mass1 * mass2 / (distance * distance);

    glm::vec3 force = forceMagnitude * direction;
    sphere1->rigidBody->applyCentralForce(btVector3(force.x, force.y, force.z));
    sphere2->rigidBody->applyCentralForce(btVector3(-force.x, -force.y, -force.z));
}

// 应用万有引力到所有球体
void Simulation::applyGravityToAll() {
    for (size_t i = 0; i < spheres.size(); ++i) {
        for (size_t j = i + 1; j < spheres.size(); ++j) {
            applyGravity(spheres[i].get(), spheres[j].get());
        }
    }
}

// 重置球体位置
void Simulation::resetSpheres() {
    if (spheres.size() >= 2) {
        // 重置速度
        spheres[0]->rigidBody->setLinearVelocity(btVector3(0, 0, 0));
        spheres[1]->rigidBody->setLinearVelocity(btVector3(0, 0, 0));

        // 重置位置
        spheres[0]->rigidBody->setWorldTransform(
            btTransform(btQuaternion(0, 0, 0, 1), btVector3(-2, 0, 0)));
        spheres[1]->rigidBody->setWorldTransform(
            btTransform(btQuaternion(0, 0, 0, 1), btVector3(2, 0, 0)));

        // 重置累计时间
        accumulatedTime = 0.0f;
    }
}

// 更新模拟状态
void Simulation::update(float deltaTime) {
    accumulatedTime += deltaTime;

    // 改为使用通用方法，支持任意数量的球体
    applyGravityToAll();

    if (dynamicsWorld) {
        dynamicsWorld->stepSimulation(deltaTime, 10);
    }
}

// 渲染场景
void Simulation::render() {
    // 清除缓冲区
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 创建投影矩阵（透视投影）
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        (float)SCR_WIDTH / (float)SCR_HEIGHT,
        0.1f, 100.0f
    );

    // 创建视图矩阵
    glm::mat4 view = glm::lookAt(
        cameraPos,
        cameraPos + cameraFront,
        cameraUp
    );

    // 使用着色器
    simpleShader->use();
    simpleShader->setMat4("projection", projectionMatrix);
    simpleShader->setMat4("view", view);

    // 设置光照属性
    simpleShader->setVec3("lightPos", 5.0f, 5.0f, 5.0f);
    simpleShader->setVec3("lightColor", 1.0f, 1.0f, 1.0f);
    simpleShader->setVec3("viewPos", cameraPos);

    // 渲染所有球体
    for (const auto& sphere : spheres) {
        renderSphere(*sphere);
    }

    // 渲染坐标轴
    renderAxes();
}

// 渲染单个球体
void Simulation::renderSphere(const Sphere& sphere) {
    // 获取球体位置
    glm::vec3 position = sphere.getPosition();

    // 创建模型矩阵
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);

    // 设置着色器uniform
    simpleShader->setMat4("model", model);
    simpleShader->setVec3("color", sphere.color);

    // 渲染球体
    sphere.render();
}

// 渲染坐标轴
void Simulation::renderAxes() {
    // 临时禁用深度测试，确保坐标轴始终可见
    glDisable(GL_DEPTH_TEST);

    // 使用坐标轴着色器
    axesShader->use();

    // 创建变换矩阵
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        (float)SCR_WIDTH / (float)SCR_HEIGHT,
        0.1f, 100.0f
    );

    glm::mat4 view = glm::lookAt(
        cameraPos,
        cameraPos + cameraFront,
        cameraUp
    );

    glm::mat4 model = glm::mat4(1.0f);

    axesShader->setMat4("projection", projectionMatrix);
    axesShader->setMat4("view", view);
    axesShader->setMat4("model", model);

    // 绑定并渲染坐标轴
    glBindVertexArray(axesVAO);
    glDrawArrays(GL_LINES, 0, 6);
    glBindVertexArray(0);

    // 重新启用深度测试
    glEnable(GL_DEPTH_TEST);
}