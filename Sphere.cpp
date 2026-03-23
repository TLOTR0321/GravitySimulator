#include "Sphere.h"
#include <glm/gtc/constants.hpp>
#include <iostream>

Sphere::Sphere(btDiscreteDynamicsWorld* world, float r, float mass,
    const glm::vec3& pos, const glm::vec3& color)
    : radius(r), color(color) {

    // 生成顶点数据
    generateSphereData();

    // 创建碰撞形状 - 现在保存为成员变量
    collisionShape = std::make_unique<btSphereShape>(r);

    btTransform startTransform;
    startTransform.setIdentity();
    startTransform.setOrigin(btVector3(pos.x, pos.y, pos.z));

    // 创建运动状态
    motionState = std::make_unique<btDefaultMotionState>(startTransform);

    btVector3 localInertia(0, 0, 0);
    if (mass != 0.0f) {
        collisionShape->calculateLocalInertia(mass, localInertia);
    }

    // 使用成员变量中的碰撞形状
    btRigidBody::btRigidBodyConstructionInfo rbInfo(
        mass,
        motionState.get(),
        collisionShape.get(),  // 使用成员变量
        localInertia
    );
    rigidBody = std::make_unique<btRigidBody>(rbInfo);
    rigidBody->setActivationState(DISABLE_DEACTIVATION);

    // 设置用户指针，便于后续识别
    rigidBody->setUserPointer(this);

    world->addRigidBody(rigidBody.get());

    // 初始化OpenGL资源
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // 顶点缓冲区
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
        vertices.size() * sizeof(glm::vec3) + normals.size() * sizeof(glm::vec3),
        NULL, GL_STATIC_DRAW);

    // 复制顶点数据
    glBufferSubData(GL_ARRAY_BUFFER, 0,
        vertices.size() * sizeof(glm::vec3), vertices.data());

    // 复制法线数据
    glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3),
        normals.size() * sizeof(glm::vec3), normals.data());

    // 索引缓冲区
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // 设置顶点属性指针
    // 位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    // 法线属性（注意偏移量）
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3),
        (void*)(vertices.size() * sizeof(glm::vec3)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    isInitialized = true;
}

Sphere::~Sphere() {
    if (isInitialized) {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }
}

void Sphere::generateSphereData(int sectorCount, int stackCount) {
    constexpr float PI = glm::pi<float>();
    float sectorStep = 2.0f * PI / sectorCount;
    float stackStep = PI / stackCount;
    float sectorAngle, stackAngle;

    for (int i = 0; i <= stackCount; ++i) {
        stackAngle = PI / 2 - i * stackStep;
        float xy = radius * cosf(stackAngle);
        float z = radius * sinf(stackAngle);

        for (int j = 0; j <= sectorCount; ++j) {
            sectorAngle = j * sectorStep;

            float x = xy * cosf(sectorAngle);
            float y = xy * sinf(sectorAngle);

            vertices.push_back(glm::vec3(x, y, z));
            glm::vec3 normal = glm::normalize(glm::vec3(x, y, z));
            normals.push_back(normal);
        }
    }

    // 生成索引
    for (int i = 0; i < stackCount; ++i) {
        int k1 = i * (sectorCount + 1);
        int k2 = k1 + sectorCount + 1;

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != (stackCount - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }
}

void Sphere::render() const {
    if (!isInitialized) return;

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

glm::vec3 Sphere::getPosition() const {
    if (!rigidBody || !rigidBody->getMotionState()) {
        return glm::vec3(0.0f);
    }

    btTransform trans;
    rigidBody->getMotionState()->getWorldTransform(trans);
    btVector3 pos = trans.getOrigin();
    return glm::vec3(pos.x(), pos.y(), pos.z());
}