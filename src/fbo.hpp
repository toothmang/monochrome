
#include "glm/glm.hpp"
#include <vector>

class FBO
{
public:

    FBO(int texwidth, int texheight);
    ~FBO();

    void bind();
    void unbind();
    void blank();
    void load();

    unsigned int colTexID;

private:


    int width, height;

    unsigned int fbID;
    unsigned int depthTexID;
    unsigned int depthBuffer;
};

class FSQuad
{
    public:
    static FSQuad & get()
    {
        static FSQuad instance;
        return instance;
    }

    unsigned int vboHandle = 0;
    unsigned int elementsHandle = 0;
    int shaderHandle = 0, vertexHandle = 0, fragHandle = 0;
    int uniformColorLoc = 0, uniformTexLoc = 0;
    int attribPosLoc = 0, attribUVLoc = 0;

    bool initialized = false;

    std::vector<unsigned int> indices;
    std::vector<glm::vec4> xy_uv;

    virtual ~FSQuad();

    void shutdown();

    bool init();

    void render(glm::vec4 color);
};
