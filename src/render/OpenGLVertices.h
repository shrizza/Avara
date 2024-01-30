#pragma once

#include "CBSPPart.h"
#include "ModernOpenGLRenderer.h"
#include "VertexData.h"

#include <vector>

struct GLData {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 0.0f;
    float nx = 0.0f;
    float ny = 0.0f;
    float nz = 0.0f;

    GLData() {};
};

struct GLVertexCollection {
public:
    GLuint vertexBuffer = 0;
    GLuint vertexArray = 0;
    uint16_t pointCount = 0;
    std::vector<GLData> glData = {};
    GLsizeiptr glDataSize = 0;

    void SortFromCamera(float pos[3]);
};

class OpenGLVertices: public VertexData {
public:
    friend class ModernOpenGLRenderer;

    OpenGLVertices();
    virtual ~OpenGLVertices();

    virtual void Append(const CBSPPart &part);
    virtual void Replace(const CBSPPart &part);
private:
    GLVertexCollection opaque;
    GLVertexCollection alpha;
};
