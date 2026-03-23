#ifndef SPHERE_H
#define SPHERE_H

#include <btBulletDynamicsCommon.h>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <vector>
#include <memory>

class Sphere {
public:
    Sphere(btDiscreteDynamicsWorld* world, float r, float mass,
        const glm::vec3& pos, const glm::vec3& color);
    ~Sphere();

    // 生成球体顶点数据
    void generateSphereData(int sectorCount = 32, int stackCount = 16);

    // 渲染球体
    void render() const;

    // 获取球体位置
    glm::vec3 getPosition() const;

    // 公有成员
    std::unique_ptr<btRigidBody> rigidBody;
    float radius;
    glm::vec3 color;

private:
    // 物理引擎相关
    std::unique_ptr<btDefaultMotionState> motionState;
    std::unique_ptr<btSphereShape> collisionShape;  // 新增：保存碰撞形状

    // OpenGL相关
    GLuint VAO, VBO, EBO;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;
    bool isInitialized = false;
};

#endif