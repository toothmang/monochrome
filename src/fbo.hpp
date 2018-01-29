
class FBO
{
public:

    FBO(int texwidth, int texheight);
    ~FBO();

    void bind();
    void unbind();
    void blank();
    void load();

private:


    int width, height;

    unsigned int fbID;
    unsigned int colTexID, depthTexID;
    unsigned int depthBuffer;
};
