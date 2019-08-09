#include "openglwidget.h"

OpenGLWidget::OpenGLWidget(QWidget *parent) : QOpenGLWidget(parent)
{
    // screen geometry
    int w = geometry().width();
    int h = geometry().height();
    float AspectRatio = static_cast<float>(w) / static_cast<float>(h);

    // camera
    camera.pitch = 0.0f;
    camera.yaw = 90.0f;
    camera.radius = 55.0f;
    camera.eye = {0.0f, 0.0f, camera.radius};
    camera.center = {0.0f, 0.0f, 0.0f};
    camera.up = {0.0f, 1.0f, 0.0f};
    camera.front = (camera.center - camera.eye).normalized();
    camera.right = QVector3D::crossProduct(camera.front, camera.up).normalized();
    camera.FieldOfView = 45.0f;
    camera.AspectRatio = AspectRatio;
    camera.NearPlane = 1.0f;
    camera.FarPlane = 1000.0f;

    // mouse events
    setMouseTracking(true);
    mouse.lastX = 0.0f;
    mouse.lastY = 0.0f;
    mouse.MouseLeftButton = false;
    mouse.MouseMiddleButton = false;
    mouse.MouseRightButton = false;
    mouse.MouseWheel = false;

    // background color
    BackgroundColor = Qt::GlobalColor::gray;
    // background = background.darker();

    // trajectory
    trajectory.LoadCookedData();
    PointsCount = trajectory.atoms.size();

    // playback
    playback.active = false;
    playback.step = 0;
    playback.size = trajectory.models.size();
    playback.speed = 30.0f;

    // lighting
    light.position = {0.0f, 0.0f, 0.0f};
    light.direction = {0.0f, 0.0f, 0.0f};

    light.free = false;
    light.model.setToIdentity();

    light.ambient = FromQColorToQVector3D(Qt::GlobalColor::darkGray);
    light.diffuse = FromQColorToQVector3D(Qt::GlobalColor::gray);
    light.specular = FromQColorToQVector3D(Qt::GlobalColor::lightGray);
    light.attenuation = false;
    light.constant = 1.0f;
    light.linear = 0.09f;
    light.quadratic = 0.032f;

    // SSAO
    SSAO.active = true;
    SSAO.blur = true;
    SSAO.KernelSize = 8;
    SSAO.NoiseSize = 4;
    SSAO.radius = 10.0f;
    SSAO.bias = 0.025f;
    SSAO.SetKernel();
    SSAO.SetNoise(w, h);

    // outline
    outline.active = false;
    outline.grayscale = true;
    outline.mode = OutlineMode::RESIDUE_RMSF;
    outline.palette = ColorPalettes.first();
    outline.size = 5;
    outline.thickness = 3;
    outline.boundary = BoundaryValues::absolute;
    SetOutlineColor();

    // frame rate
    FrameRate.frames = 0;
    FrameRate.fps = 0.0f;

    // shift key status
    ShiftDown = false;
}

// gl

void OpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();

    /*
    int w = geometry().width();
    int h = geometry().height();
    */

    // clear color
    SetClearColor(BackgroundColor);

    // VAOs
    {
        auto VAOsData = trajectory.GetVAOsData();
        VAOs.resize(VAOsData.size());

        int index = 0;
        for (auto data : VAOsData)
        {
            glGenVertexArrays(1, &(VAOs[index]));

            GLuint vbo;
            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            GLsizei stride = sizeof(VertexData);
            GLsizeiptr size = data.size() * static_cast<GLsizeiptr>(stride);
            glBufferData(GL_ARRAY_BUFFER, size, &(data[0]), GL_STATIC_DRAW);

            unsigned int bytes;
            const void* offset;

            glBindVertexArray(VAOs[index]);
            {
                // center
                bytes = 0;
                offset = nullptr;
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, offset);
                glEnableVertexAttribArray(0);
                // radius
                bytes += sizeof(VertexData::center);
                offset = reinterpret_cast<const void*>(bytes);
                glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, stride, offset);
                glEnableVertexAttribArray(1);
                // albedo
                bytes += sizeof(VertexData::radius);
                offset = reinterpret_cast<const void*>(bytes);
                glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, offset);
                glEnableVertexAttribArray(2);
                // atom and residue number
                bytes += sizeof(VertexData::albedo);
                offset = reinterpret_cast<const void*>(bytes);
                // glVertexAttribPointer(3, 1, GL_INT, GL_FALSE, stride, offset);
                // glVertexAttribPointer(3, 1, GL_UNSIGNED_INT, GL_FALSE, stride, offset);
                // glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, stride, offset);
                glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, stride, offset);
                glEnableVertexAttribArray(3);
            }
            glBindVertexArray(0);

            glDeleteBuffers(1, &vbo);

            index += 1;
        }
    }

    // framebuffers
    {
        int index;

        // geometry
        index = FBOIndex::GEOMETRY;
        glBindFramebuffer(GL_FRAMEBUFFER, addFBO(index));
        {
            // textures
            addTexture(TextureIndex::CENTER);
            addTexture(TextureIndex::NORMAL);
            addTexture(TextureIndex::ALBEDO);
            addTexture(TextureIndex::COLOUR);

            // render buffer object
            addRBO(RBOIndex::GEOMETRY);

            // check framebuffer status
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            {
                qDebug() << QString("framebuffer %1 :: NOT complete").arg(index);
            }

            // attachments
            QVector<GLenum> attachments;
            while (attachments.size() < 4)
            {
                attachments += GL_COLOR_ATTACHMENT0 + static_cast<unsigned int>(attachments.size());
            }
            glDrawBuffers(attachments.size(), &(attachments[0]));
        }
        glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());

        // SSAO : occlusion
        index = FBOIndex::SSAO_OCCLUSION;
        glBindFramebuffer(GL_FRAMEBUFFER, addFBO(index));
        {
            // textures
            addTexture(TextureIndex::SSAO_KERNEL);
            addTexture(TextureIndex::SSAO_NOISE);
            addTexture(TextureIndex::SSAO_OCCLUSION);

            // check framebuffer status
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            {
                qDebug() << QString("framebuffer %1 :: NOT complete").arg(index);
            }
        }
        glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());

        // SSAO : blur
        index = FBOIndex::SSAO_BLUR;
        glBindFramebuffer(GL_FRAMEBUFFER, addFBO(index));
        {
            // textures
            addTexture(TextureIndex::SSAO_BLUR);

            // check framebuffer status
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            {
                qDebug() << QString("framebuffer %1 :: NOT complete").arg(index);
            }
        }
        glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());

        // silhouette
        index = FBOIndex::SILHOUETTE;
        glBindFramebuffer(GL_FRAMEBUFFER, addFBO(index));
        {
            // textures
            addTexture(TextureIndex::SILHOUETTE);

            // render buffer object
            addRBO(RBOIndex::SILHOUETTE);

            // check framebuffer status
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            {
                qDebug() << QString("framebuffer %1 :: NOT complete").arg(index);
            }
        }
        glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
    }

    // textures code here
    addTexture(TextureIndex::OUTLINE);

    // frame rate
    FrameRate.time.start();
}

void OpenGLWidget::resizeGL(int w, int h)
{
    // viewport
    glViewport(0, 0, w, h);

    // projection
    camera.AspectRatio = static_cast<float>(w) / static_cast<float>(h);
    projection.setToIdentity();
    projection.perspective(camera.FieldOfView, camera.AspectRatio, camera.NearPlane, camera.FarPlane);

    // SSAO : noise scale
    SSAO.SetNoiseScale(w, h);

    // framebuffers
    {
        int index;

        // geometry
        index = FBOIndex::GEOMETRY;
        glBindFramebuffer(GL_FRAMEBUFFER, addFBO(index));
        {
            // textures
            addTexture(TextureIndex::CENTER);
            addTexture(TextureIndex::NORMAL);
            addTexture(TextureIndex::ALBEDO);
            addTexture(TextureIndex::COLOUR);

            // render buffer object
            addRBO(RBOIndex::GEOMETRY);

            // check framebuffer status
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            {
                qDebug() << QString("framebuffer %1 :: NOT complete").arg(index);
            }
        }
        glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());

        // SSAO : occlusion
        index = FBOIndex::SSAO_OCCLUSION;
        glBindFramebuffer(GL_FRAMEBUFFER, addFBO(index));
        {
            // textures
            // addTexture(TextureIndex::SSAO_KERNEL);
            // addTexture(TextureIndex::SSAO_NOISE);
            addTexture(TextureIndex::SSAO_OCCLUSION);

            // check framebuffer status
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            {
                qDebug() << QString("framebuffer %1 :: NOT complete").arg(index);
            }
        }
        glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());

        // SSAO : blur
        index = FBOIndex::SSAO_BLUR;
        glBindFramebuffer(GL_FRAMEBUFFER, addFBO(index));
        {
            // textures
            addTexture(TextureIndex::SSAO_BLUR);

            // check framebuffer status
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            {
                qDebug() << QString("framebuffer %1 :: NOT complete").arg(index);
            }
        }
        glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());

        // silhouette
        index = FBOIndex::SILHOUETTE;
        glBindFramebuffer(GL_FRAMEBUFFER, addFBO(index));
        {
            // textures
            addTexture(TextureIndex::SILHOUETTE);

            // render buffer object
            addRBO(RBOIndex::SILHOUETTE);

            // check framebuffer status
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            {
                qDebug() << QString("framebuffer %1 :: NOT complete").arg(index);
            }
        }
        glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
    }
}

void OpenGLWidget::paintGL()
{
    int w = geometry().width();
    int h = geometry().height();

    // update view matrix
    view.setToIdentity();
    view.lookAt(camera.eye, camera.center, camera.up);

    /* #################### geometry #################### */

    glBindFramebuffer(GL_FRAMEBUFFER, addFBO(FBOIndex::GEOMETRY));
    {
        // set clear color
        SetClearColor(TestDrawingMode ? BackgroundColor : Qt::GlobalColor::black);

        // enable depth test
        glEnable(GL_DEPTH_TEST);
        // change depth test function
        glDepthFunc(GL_LESS);

        // clear color and depth buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // draw geometry
        DrawImpostors();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());

    /* #################### SSAO #################### */

    if (SSAO.active || TestDrawingMode)
    {
        if (SSAO.KernelTextureFlag)
        {
            addTexture(TextureIndex::SSAO_KERNEL);
            SSAO.KernelTextureFlag = false;
        }

        if (SSAO.NoiseTextureFlag)
        {
            addTexture(TextureIndex::SSAO_NOISE);
            SSAO.NoiseTextureFlag = false;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, addFBO(FBOIndex::SSAO_OCCLUSION));
        {
            // set clear color
            SetClearColor(Qt::GlobalColor::white); // useless

            // disable depth test
            glDisable(GL_DEPTH_TEST);

            // clear color buffer
            glClear(GL_COLOR_BUFFER_BIT);

            // draw SSAO : occlusion
            DrawOcclusion();
        }
        glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());

        if (SSAO.blur || TestDrawingMode)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, addFBO(FBOIndex::SSAO_BLUR));
            {
                // disable depth test
                glDisable(GL_DEPTH_TEST);

                // clear color buffer
                glClear(GL_COLOR_BUFFER_BIT);

                // draw SSAO : blur
                DrawBlur();
            }
            glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
        }
    }

    /* #################### silhouette #################### */

    if (outline.active || TestDrawingMode)
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, addFBO(FBOIndex::GEOMETRY));
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, addFBO(FBOIndex::SILHOUETTE));
        {
            // set clear color
            SetClearColor(Qt::GlobalColor::white);

            // copy depth buffer
            glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
            // enable depth test
            glEnable(GL_DEPTH_TEST);
            // change depth test function
            glDepthFunc(GL_LEQUAL);

            // clear color buffer
            glClear(GL_COLOR_BUFFER_BIT);

            // draw silhouette
            DrawSilhouette();
        }
        glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
    }

    /* #################### lighting #################### */

    // set clear color
    SetClearColor(BackgroundColor);

    // disable depth test
    glDisable(GL_DEPTH_TEST);

    // clear color buffer
    glClear(GL_COLOR_BUFFER_BIT);

    // draw lighting
    TestDrawingMode ? DrawQuad() : DrawLighting();

    /* #################### outline #################### */

    if (outline.active)
    {
        if (outline.OutlineTextureFlag)
        {
            addTexture(TextureIndex::OUTLINE);
            outline.OutlineTextureFlag = false;
        }

        DrawOutline();
    }

    /* #################### light marker #################### */

    if (light.free)
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, addFBO(FBOIndex::GEOMETRY));
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, defaultFramebufferObject());
        {
            // copy depth buffer
            glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

            // enable depth test
            glEnable(GL_DEPTH_TEST);

            // draw light marker
            DrawLightMarker();
        }
        glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
    }

    // playback
    if (playback.time.elapsed() >= (1000 / playback.speed))
    {
        if (playback.active)
        {
            playback.step = (playback.step + 1) % playback.size;
            emit NextStepSignal();
        }
        playback.time.restart();
    }

    // frame rate
    FrameRate.frames++;
    if (FrameRate.time.elapsed() >= 1000)
    {
        FrameRate.fps = FrameRate.frames / (FrameRate.time.elapsed() * 0.001f);
        FrameRate.frames = 0;
        // emit FrameRateSignal(FrameRate.fps);
        emit FrameRateSignal();
        FrameRate.time.restart();
    }

    update();
}

void OpenGLWidget::SetClearColor(QColor color)
{
    GLfloat r = static_cast<GLfloat>(color.redF());
    GLfloat g = static_cast<GLfloat>(color.greenF());
    GLfloat b = static_cast<GLfloat>(color.blueF());
    GLfloat a = static_cast<GLfloat>(color.alphaF());
    glClearColor(r, g, b, a);
}

// shaders

GLuint OpenGLWidget::createProgram(std::string vertPath, std::string geomPath, std::string fragPath)
{
    // retrieve the vertex and fragment source code from file path
    std::string vertSource;
    std::string fragSource;
    std::ifstream vertShaderFile;
    std::ifstream fragShaderFile;
    // ensure ifstream objects can throw exceptions
    vertShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fragShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        // open files
        vertShaderFile.open(vertPath.c_str());
        fragShaderFile.open(fragPath.c_str());
        std::stringstream vertShaderStream;
        std::stringstream fragShaderStream;
        // read file's buffer contents into streams
        vertShaderStream << vertShaderFile.rdbuf();
        fragShaderStream << fragShaderFile.rdbuf();
        // close file handlers
        vertShaderFile.close();
        fragShaderFile.close();
        // convert stream into string
        vertSource = vertShaderStream.str();
        fragSource = fragShaderStream.str();
    }
    catch (std::ifstream::failure exception)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
        return 0;
    }

    // compile the vertex shader
    GLuint vertShader = createShader(vertSource, GL_VERTEX_SHADER);
    if (vertShader == 0)
    {
        std::cout << "ERROR::" << vertPath << std::endl;
        return 0;
    }

    // compile the fragment shader
    GLuint fragShader = createShader(fragSource, GL_FRAGMENT_SHADER);
    if (fragShader == 0)
    {
        std::cout << "ERROR::" << fragPath << std::endl;
        return 0;
    }

    // vertex and fragment shaders are now successfully compiled

    GLuint geomShader = 0;
    if (!geomPath.empty())
    {
        // retrieve the geometry source code from file path
        std::string geomSource;
        std::ifstream geomShaderFile;
        // ensure ifstream objects can throw exceptions
        geomShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            // open files
            geomShaderFile.open(geomPath.c_str());
            std::stringstream geomShaderStream;
            // read file's buffer contents into streams
            geomShaderStream << geomShaderFile.rdbuf();
            // close file handlers
            geomShaderFile.close();
            // convert stream into string
            geomSource = geomShaderStream.str();
        }
        catch (std::ifstream::failure exception)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
            return 0;
        }

        // compile the geometry shader
        geomShader = createShader(geomSource, GL_GEOMETRY_SHADER);
        if (geomShader == 0)
        {
            std::cout << "ERROR::" << geomPath << std::endl;
            return 0;
        }

        // geometry shader is now successfully compiled
    }

    // get a program object
    // glCreateProgram returns 0 if an error occurs creating the program object
    GLuint program = glCreateProgram();

    // attach vertex and fragment shaders to the program
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    // attach geometry shaders to the program
    if (geomShader != 0)
    {
        glAttachShader(program, geomShader);
    }

    // link the program
    glLinkProgram(program);

    GLint isLinked = 0;
    // glGetProgramiv(program, GL_LINK_STATUS, (int *)&isLinked);
    glGetProgramiv(program, GL_LINK_STATUS, static_cast<int *>(&isLinked));
    if (isLinked == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

        // the maxLength includes the NULL character
        std::vector<GLchar> infoLog(static_cast<unsigned long long>(maxLength));
        glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

        // we don't need the program and the shaders anymore
        glDeleteProgram(program);
        glDeleteShader(vertShader);
        glDeleteShader(geomShader);
        glDeleteShader(fragShader);

        std::cout << "ERROR::PROGRAM::LINKING_ERROR" << std::endl;
        std::string infoLogString(infoLog.begin(), infoLog.end());
        std::cout << infoLogString << std::endl;

        return 0;
    }

    // detach shaders after a successful link
    glDetachShader(program, vertShader);
    glDetachShader(program, fragShader);
    if (geomShader != 0)
    {
        glDetachShader(program, geomShader);
    }

    // this function returns 0 if an error occurs creating the program object
    return program;
}

GLuint OpenGLWidget::createShader(std::string source, GLenum type)
{
    std::map<GLenum, std::string> shaderTypeName = {
        {GL_VERTEX_SHADER, "GL_VERTEX_SHADER"},
        {GL_FRAGMENT_SHADER, "GL_FRAGMENT_SHADER"},
        {GL_GEOMETRY_SHADER, "GL_GEOMETRY_SHADER"},
        {GL_COMPUTE_SHADER, "GL_COMPUTE_SHADER"},
        {GL_TESS_CONTROL_SHADER, "GL_TESS_CONTROL_SHADER"},
        {GL_TESS_EVALUATION_SHADER, "GL_TESS_EVALUATION_SHADER"},
    };

    // create an empty shader handle
    // glCreateShader returns 0 if an error occurs creating the shader object
    GLuint shader = glCreateShader(type);

    // send the shader source code to GL
    // std::string's .c_str is NULL character terminated
    const GLchar *sourceCString = static_cast<const GLchar*>(source.c_str());
    glShaderSource(shader, 1, &sourceCString, nullptr);

    // compile the vertex shader
    glCompileShader(shader);

    GLint isCompiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

        // the maxLength includes the NULL character
        std::vector<GLchar> infoLog(static_cast<unsigned long long>(maxLength));
        glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

        // we don't need the shader anymore
        glDeleteShader(shader);

        std::cout << "ERROR::SHADER::COMPILATION_ERROR::" << shaderTypeName[type] << std::endl;
        std::string infoLogString(infoLog.begin(), infoLog.end());
        std::cout << infoLogString << std::endl;

        // this function returns 0 if an error occurs compiling the shader object
        return 0;
    }

    return shader;
}

GLuint OpenGLWidget::addProgram(int index)
{
    if (programs.size() <= index)
    {
        programs.resize(index + 1);
    }

    if (programs[index] == 0)
    {
        std::string prefix = "../FinalProject/shaders/";
        bool geometry = false;

        switch (index)
        {
        case ProgramIndex::POINT:
        {
            prefix += "point";
            // geometry = true;
            break;
        }
        case ProgramIndex::IMPOSTOR:
        {
            prefix += "impostor";
            geometry = true;
            break;
        }
        case ProgramIndex::SSAO_OCCLUSION:
        {
            prefix += "SSAO_occlusion";
            geometry = true;
            break;
        }
        case ProgramIndex::SSAO_BLUR:
        {
            prefix += "SSAO_blur";
            geometry = true;
            break;
        }
        case ProgramIndex::LIGHTING:
        {
            prefix += "lighting";
            geometry = true;
            break;
        }
        case ProgramIndex::SILHOUETTE:
        {
            prefix += "silhouette";
            geometry = true;
            break;
        }
        case ProgramIndex::OUTLINE:
        {
            prefix += "outline";
            geometry = true;
            break;
        }
        case ProgramIndex::MARKER:
        {
            prefix += "marker";
            break;
        }
        case ProgramIndex::QUAD:
        {
            prefix += "quad";
            geometry = true;
            break;
        }
        }

        std::string vert = prefix + "_vert.glsl";
        std::string geom = geometry ? prefix + "_geom.glsl" : std::string();
        std::string frag = prefix + "_frag.glsl";

        programs[index] = createProgram(vert, geom, frag);
    }

    return programs[index];
}

void OpenGLWidget::removeProgram(int index)
{
    if (index < programs.size())
    {
        glDeleteProgram(programs[index]);
        programs.remove(index);
    }
}

// VAOs

GLuint OpenGLWidget::addVAO(int index)
{
    if (VAOs.size() <= index)
    {
        VAOs.resize(index + 1);
    }

    if (VAOs[index] == 0)
    {
        glGenVertexArrays(1, &(VAOs[index]));

        switch (index)
        {
        default:
        {

        }
        }
    }

    return VAOs[index];
}

void OpenGLWidget::removeVAO(int index)
{
    if (index < VAOs.size())
    {
        for (int i = index; i < VAOs.size(); i++)
        {
            glDeleteVertexArrays(1, &(VAOs[i]));
        }
        VAOs.resize(index);
    }
}

// FBOs

GLuint OpenGLWidget::addFBO(int index)
{
    if (FBOs.size() <= index)
    {
        FBOs.resize(index + 1);
    }

    if (FBOs[index] == 0)
    {
        glGenFramebuffers(1, &(FBOs[index]));
    }

    return FBOs[index];
}

// textures

GLuint OpenGLWidget::addTexture(int index)
{
    int w = geometry().width();
    int h = geometry().height();

    if (textures.size() <= index)
    {
        textures.resize(index + 1);
    }

    if (textures[index] == 0)
    {
        glGenTextures(1, &(textures[index]));
    }

    GLuint texture = textures[index];

    switch (index)
    {
    case TextureIndex::CENTER:
    {
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, w, h, 0, GL_RGB, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
        break;
    }
    case TextureIndex::NORMAL:
    {
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, w, h, 0, GL_RGB, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, texture, 0);
        break;
    }
    case TextureIndex::ALBEDO:
    {
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, texture, 0);
        break;
    }
    case TextureIndex::COLOUR:
    {
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, texture, 0);
        break;
    }
    case TextureIndex::SSAO_KERNEL:
    {
        GLsizei size = SSAO.KernelSize;
        const void *pixels = SSAO.kernel.constData();

        glBindTexture(GL_TEXTURE_2D, texture);
        // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, SSAO.KernelSize, SSAO.KernelSize, 0, GL_RGB, GL_FLOAT, &(SSAO.kernel[0]));
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, size, size, 0, GL_RGB, GL_FLOAT, pixels);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        break;
    }
    case TextureIndex::SSAO_NOISE:
    {
        GLsizei size = SSAO.NoiseSize;
        const void *pixels = SSAO.noise.constData();

        glBindTexture(GL_TEXTURE_2D, texture);
        // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, NoiseSize, NoiseSize, 0, GL_RGB, GL_FLOAT, &(SSAO.noise[0]));
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, size, size, 0, GL_RGB, GL_FLOAT, pixels);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        break;
    }
    case TextureIndex::SSAO_OCCLUSION:
    {
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RGB, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
        break;
    }
    case TextureIndex::SSAO_BLUR:
    {
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RGB, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
        break;
    }
    case TextureIndex::SILHOUETTE:
    {
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
        break;
    }
    case TextureIndex::OUTLINE:
    {
        GLsizei size = outline.colours.size();
        const void *pixels = outline.colours.constData();

        glBindTexture(GL_TEXTURE_1D, texture);
        // glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, outline.colours.size(), 0, GL_RGB, GL_FLOAT, &(outline.colours[0]));
        glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB32F, size, 0, GL_RGB, GL_FLOAT, pixels);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        break;
    }
    }

    return texture;
}

// RBOs

GLuint OpenGLWidget::addRBO(int index)
{
    int w = geometry().width();
    int h = geometry().height();

    if (RBOs.size() <= index)
    {
        RBOs.resize(index + 1);
    }

    if (RBOs[index] == 0)
    {
        glGenRenderbuffers(1, &(RBOs[index]));
    }

    GLuint rbo = RBOs[index];

    switch (index)
    {
    case RBOIndex::GEOMETRY:
    {
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
        break;
    }
    case RBOIndex::SILHOUETTE:
    {
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
        break;
    }
    }

    return rbo;
}

// mouse events

void OpenGLWidget::mousePressEvent(QMouseEvent *event)
{
    if (mouse.MouseLeftButton || mouse.MouseMiddleButton || mouse.MouseRightButton || mouse.MouseWheel) return;

    switch (event->button())
    {
    case Qt::LeftButton:
    {
        mouse.lastX = event->pos().x();
        mouse.lastY = event->pos().y();
        mouse.MouseLeftButton = true;
        break;
    }
    case Qt::MiddleButton:
    {
        mouse.lastX = event->pos().x();
        mouse.lastY = event->pos().y();
        mouse.MouseMiddleButton = true;
        break;
    }
    case Qt::RightButton:
    {
        mouse.lastX = event->pos().x();
        mouse.lastY = event->pos().y();
        mouse.MouseRightButton = true;
        break;
    }
    default:
    {
        break;
    }
    }
}

void OpenGLWidget::mouseMoveEvent(QMouseEvent *event)
{
    int x = event->pos().x();
    int y = event->pos().y();
    float factor = 0.005f;

    // hover code here

    makeCurrent();
    glBindFramebuffer(GL_READ_FRAMEBUFFER, addFBO(FBOIndex::GEOMETRY));
    {
        glReadBuffer(GL_COLOR_ATTACHMENT3);

        QVector<GLubyte> pixel(3);
        glReadPixels(x, geometry().height() - y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, &(pixel[0]));

        // QString PixelColor = QColor(pixel[0], pixel[1], pixel[2]).name();
        QColor PixelColor = QColor(pixel[0], pixel[1], pixel[2]);

        QVector<unsigned int> shift = {16, 8, 0};

        int AtomNumber = 0;
        for (int i = 0; i < pixel.size(); i++)
        {
            // AtomNumber |= pixel[i] << shift[i];
            AtomNumber += pixel[i] << shift[i];
        }

        qDebug() << PixelColor.name() << AtomNumber;

        if (trajectory.atoms.keys().contains(AtomNumber))
        {
            emit MouseHoverSignal(trajectory.atoms[AtomNumber]);
        }
        else
        {
            emit MouseNotHoverSignal();
        }


        // glReadBuffer(GL_COLOR_ATTACHMENT0);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
    doneCurrent();

    if (mouse.MouseLeftButton)
    {
        /*
        float dx = factor * (x - mouse.lastX);
        float dy = factor * (y - mouse.lastY);

        QMatrix4x4 rotation;
        rotation.rotate(qRadiansToDegrees(dx), camera.up);
        rotation.rotate(qRadiansToDegrees(dy), camera.right);

        QVector4D column = model.column(3);
        model.setColumn(3, {0.0f, 0.0f, 0.0f, 1.0f});
        model = rotation * model;
        model.setColumn(3, column);

        mouse.lastX = x;
        mouse.lastY = y;
        */

        if (light.free && ShiftDown)
        {
            factor *= 10.0f;

            float dx = factor * (x - mouse.lastX);
            float dy = factor * (mouse.lastY - y);

            QMatrix4x4 translation;
            translation.translate(dx * camera.right);
            translation.translate(dy * camera.up);
            light.model = translation * light.model;

            mouse.lastX = x;
            mouse.lastY = y;

            factor *= 0.1f;
        }
        else
        {
            float dx = factor * (x - mouse.lastX);
            float dy = factor * (y - mouse.lastY);

            QMatrix4x4 rotation;
            rotation.rotate(qRadiansToDegrees(dx), camera.up);
            rotation.rotate(qRadiansToDegrees(dy), camera.right);

            QVector4D column = model.column(3);
            model.setColumn(3, {0.0f, 0.0f, 0.0f, 1.0f});
            model = rotation * model;
            model.setColumn(3, column);

            mouse.lastX = x;
            mouse.lastY = y;
        }
    }

    if (mouse.MouseMiddleButton)
    {
        factor *= 10.0f;

        float dx = factor * (x - mouse.lastX);
        float dy = factor * (mouse.lastY - y);

        QMatrix4x4 translation;
        translation.translate(dx * camera.right);
        translation.translate(dy * camera.up);
        model = translation * model;

        mouse.lastX = x;
        mouse.lastY = y;

        factor *= 0.1f;
    }

    if (mouse.MouseRightButton)
    {
        float dx = factor * (x - mouse.lastX);
        float dy = factor * (y - mouse.lastY);

        camera.yaw += qRadiansToDegrees(dx);
        camera.pitch = qBound(-MAX_PITCH, camera.pitch + qRadiansToDegrees(dy), +MAX_PITCH);

        camera.eye = {
            cos(qDegreesToRadians(camera.yaw)) * cos(qDegreesToRadians(camera.pitch)),
            sin(qDegreesToRadians(camera.pitch)),
            sin(qDegreesToRadians(camera.yaw)) * cos(qDegreesToRadians(camera.pitch)),
        };

        camera.eye = camera.radius * camera.eye.normalized();

        camera.front = (camera.center - camera.eye).normalized();
        camera.right = QVector3D::crossProduct(camera.front, {0.0f, 1.0f, 0.0f}).normalized();
        camera.up = QVector3D::crossProduct(camera.right, camera.front).normalized();

        mouse.lastX = x;
        mouse.lastY = y;
    }
}

void OpenGLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    switch (event->button())
    {
    case Qt::LeftButton:
    {
        mouse.MouseLeftButton = false;
        break;
    }
    case Qt::MiddleButton:
    {
        mouse.MouseMiddleButton = false;
        break;
    }
    case Qt::RightButton:
    {
        mouse.MouseRightButton = false;
        break;
    }
    default:
    {
        break;
    }
    }
}

void OpenGLWidget::wheelEvent(QWheelEvent *event)
{
    if (mouse.MouseLeftButton || mouse.MouseMiddleButton || mouse.MouseRightButton || mouse.MouseWheel) return;

    float dy = event->angleDelta().y() * 0.125f / 15;
    camera.radius = qBound(MIN_RADIUS, camera.radius + dy, MAX_RADIUS);
    camera.eye = camera.radius * camera.eye.normalized();

    qDebug() << camera.radius;
}

// drawing

void OpenGLWidget::DrawPoints()
{
    GLuint program = addProgram(ProgramIndex::POINT);
    glUseProgram(program);

    glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, model.constData());
    glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, view.constData());
    glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, projection.constData());

    glUniform1i(glGetUniformLocation(program, "OutlineMode"), outline.mode);

    // addTexture(TextureIndex::OUTLINE);

    glUniform1i(glGetUniformLocation(program, "outline"), 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, textures[TextureIndex::OUTLINE]);

    glPointSize(10.0f);

    GLuint vao = VAOs[playback.step];

    glBindVertexArray(vao);
    glDrawArrays(GL_POINTS, 0, PointsCount);
    glBindVertexArray(0);
}

void OpenGLWidget::DrawImpostors()
{
    GLuint program = addProgram(ProgramIndex::IMPOSTOR);
    glUseProgram(program);

    glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, model.constData());
    glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, view.constData());
    glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, projection.constData());

    // glUniform1i(glGetUniformLocation(program, "OutlineMode"), outline.mode);
    // glUniform1i(glGetUniformLocation(program, "OutlineTexture"), 0);

    glUniform1i(glGetUniformLocation(program, "grayscale"), outline.active && outline.grayscale);

    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_1D, textures[TextureIndex::OUTLINE]);

    GLuint vao = VAOs[playback.step];

    glBindVertexArray(vao);
    glDrawArrays(GL_POINTS, 0, PointsCount);
    glBindVertexArray(0);
}

void OpenGLWidget::DrawOcclusion()
{
    GLuint program = addProgram(ProgramIndex::SSAO_OCCLUSION);
    glUseProgram(program);

    // uniforms
    const GLfloat *value = projection.constData();
    glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, value);

    glUniform1i(glGetUniformLocation(program, "size"), SSAO.KernelSize);
    glUniform2fv(glGetUniformLocation(program, "scale"), 1, &(SSAO.NoiseScale[0]));
    glUniform1f(glGetUniformLocation(program, "radius"), SSAO.radius);
    glUniform1f(glGetUniformLocation(program, "bias"), SSAO.bias);

    // samplers
    glUniform1i(glGetUniformLocation(program, "center"), 0);
    glUniform1i(glGetUniformLocation(program, "normal"), 1);
    glUniform1i(glGetUniformLocation(program, "kernel"), 2);
    glUniform1i(glGetUniformLocation(program, "noise"), 3);

    // textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[TextureIndex::CENTER]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures[TextureIndex::NORMAL]);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textures[TextureIndex::SSAO_KERNEL]);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, textures[TextureIndex::SSAO_NOISE]);

    // drawing
    glDrawArrays(GL_POINTS, 0, 1);
}

void OpenGLWidget::DrawBlur()
{
    GLuint program = addProgram(ProgramIndex::SSAO_BLUR);
    glUseProgram(program);

    // uniforms
    glUniform1i(glGetUniformLocation(program, "NoiseSize"), SSAO.NoiseSize);

    // samplers
    glUniform1i(glGetUniformLocation(program, "occlusion"), 0);

    // textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[TextureIndex::SSAO_OCCLUSION]);

    // drawing
    glDrawArrays(GL_POINTS, 0, 1);
}

void OpenGLWidget::DrawLighting()
{
    GLuint program = addProgram(ProgramIndex::LIGHTING);
    glUseProgram(program);

    // uniforms
    const GLfloat *value = view.constData();
    glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, value);

    // light
    glUniform3fv(glGetUniformLocation(program, "light.position"), 1, &(light.position[0]));
    glUniform3fv(glGetUniformLocation(program, "light.direction"), 1, &(light.direction[0]));

    glUniform1i(glGetUniformLocation(program, "light.free"), light.free);
    value = light.model.constData();
    glUniformMatrix4fv(glGetUniformLocation(program, "light.model"), 1, GL_FALSE, value);

    glUniform3fv(glGetUniformLocation(program, "light.ambient"), 1, &(light.ambient[0]));
    glUniform3fv(glGetUniformLocation(program, "light.diffuse"), 1, &(light.diffuse[0]));
    glUniform3fv(glGetUniformLocation(program, "light.specular"), 1, &(light.specular[0]));
    glUniform1f(glGetUniformLocation(program, "light.attenuation"), light.attenuation);
    glUniform1f(glGetUniformLocation(program, "light.constant"), light.constant);
    glUniform1f(glGetUniformLocation(program, "light.linear"), light.linear);
    glUniform1f(glGetUniformLocation(program, "light.quadratic"), light.quadratic);

    // material
    glUniform4fv(glGetUniformLocation(program, "factors"), 1, &(factors[0]));
    glUniform1f(glGetUniformLocation(program, "shininess"), shininess);

    // samplers
    glUniform1i(glGetUniformLocation(program, "center"), 0);
    glUniform1i(glGetUniformLocation(program, "normal"), 1);
    glUniform1i(glGetUniformLocation(program, "albedo"), 2);

    // textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[TextureIndex::CENTER]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures[TextureIndex::NORMAL]);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textures[TextureIndex::ALBEDO]);

    if (SSAO.active || true)
    {
        glUniform1i(glGetUniformLocation(program, "SSAO.on"), SSAO.active);
        glUniform1i(glGetUniformLocation(program, "SSAO.sampler"), 3);

        int TextureIndex = SSAO.blur ? TextureIndex::SSAO_BLUR : TextureIndex::SSAO_OCCLUSION;

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, textures[TextureIndex]);
    }

    glDrawArrays(GL_POINTS, 0, 1);
}

void OpenGLWidget::DrawSilhouette()
{
    // filter residues
    // optmized index list

    GLuint program = addProgram(ProgramIndex::SILHOUETTE);
    glUseProgram(program);

    glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, model.constData());
    glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, view.constData());
    glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, projection.constData());

    glUniform1i(glGetUniformLocation(program, "OutlineMode"), outline.mode);

    GLuint vao = VAOs[playback.step];

    glBindVertexArray(vao);
    glDrawArrays(GL_POINTS, 0, PointsCount);
    glBindVertexArray(0);
}

void OpenGLWidget::DrawOutline()
{
    GLuint program = addProgram(ProgramIndex::OUTLINE);
    glUseProgram(program);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[TextureIndex::SILHOUETTE]);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, textures[TextureIndex::OUTLINE]);

    glUniform1i(glGetUniformLocation(program, "silhouette"), 0);
    glUniform1i(glGetUniformLocation(program, "outline"), 1);
    glUniform1i(glGetUniformLocation(program, "thickness"), outline.thickness);

    glDrawArrays(GL_POINTS, 0, 1);
}

void OpenGLWidget::DrawLightMarker()
{
    GLuint program = addProgram(ProgramIndex::MARKER);
    glUseProgram(program);

    // uniforms
    const GLfloat *value;

    value = view.constData();
    glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, value);

    value = projection.constData();
    glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, value);

    // light
    glUniform3fv(glGetUniformLocation(program, "light.position"), 1, &(light.position[0]));
    glUniform3fv(glGetUniformLocation(program, "light.direction"), 1, &(light.direction[0]));

    glUniform1i(glGetUniformLocation(program, "light.free"), light.free);

    value = light.model.constData();
    glUniformMatrix4fv(glGetUniformLocation(program, "light.model"), 1, GL_FALSE, value);

    glUniform3fv(glGetUniformLocation(program, "light.ambient"), 1, &(light.ambient[0]));
    glUniform3fv(glGetUniformLocation(program, "light.diffuse"), 1, &(light.diffuse[0]));
    glUniform3fv(glGetUniformLocation(program, "light.specular"), 1, &(light.specular[0]));
    glUniform1f(glGetUniformLocation(program, "light.attenuation"), light.attenuation);
    glUniform1f(glGetUniformLocation(program, "light.constant"), light.constant);
    glUniform1f(glGetUniformLocation(program, "light.linear"), light.linear);
    glUniform1f(glGetUniformLocation(program, "light.quadratic"), light.quadratic);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 14);
}

void OpenGLWidget::DrawQuad()
{
    GLuint program = addProgram(ProgramIndex::QUAD);
    glUseProgram(program);

    /*
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[TextureIndex::CENTER]);
    glUniform1i(glGetUniformLocation(program, "center"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures[TextureIndex::NORMAL]);
    glUniform1i(glGetUniformLocation(program, "normal"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textures[TextureIndex::ALBEDO]);
    glUniform1i(glGetUniformLocation(program, "albedo"), 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, textures[TextureIndex::COLOUR]);
    glUniform1i(glGetUniformLocation(program, "colour"), 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, textures[TextureIndex::SILHOUETTE]);
    glUniform1i(glGetUniformLocation(program, "silhouette"), 4);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, textures[TextureIndex::SILHOUETTE]);
    glUniform1i(glGetUniformLocation(program, "silhouette"), 4);

    glUniform1i(glGetUniformLocation(program, "SamplerIndex"), SamplerIndex);
    */

    glUniform1i(glGetUniformLocation(program, "sampler"), 0);
    glUniform1i(glGetUniformLocation(program, "index"), SamplerIndex);

    if (SamplerIndex != -1)
    {
        int TextureIndex = samplers[SamplerIndex];

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[TextureIndex]);
    }

    glDrawArrays(GL_POINTS, 0, 1);
}

/*
void OpenGLWidget::SetOutlineColor(bool flag)
{
    ColorScheme scheme;

    outline.schemes.clear();
    outline.colours.clear();

    switch (outline.mode)
    {
    case RESIDUE_RMSF:
    {
        // qDebug() << "RESIDUE_RMSF";

        auto list = trajectory.residues.keys();
        int MaxNumber = *std::max_element(list.begin(), list.end());

        // outline.colors.clear();
        outline.colours.resize(MaxNumber + 1);

        switch (outline.boundary)
        {
        case absolute:
        case relative:
        {
            float min = trajectory.MinResiduesRMSF;
            float max = trajectory.MaxResiduesRMSF;
            scheme = GetColorScheme(min, max, outline.palette, outline.size);

            outline.schemes += scheme;

            for (auto number : list)
            {
                Residue residue = trajectory.residues[number];

                float value = residue.RMSF;
                outline.colours[number] = FromQColorToQVector3D(GetColorStep(scheme, value).color);
            }

            break;
        }
        }

        break;
    }
    case RESIDUE_RMSD:
    {
        // qDebug() << "RESIDUE_RMSD";

        auto list = trajectory.residues.keys();
        int MaxNumber = *std::max_element(list.begin(), list.end());

        // outline.colors.clear();
        outline.colours.resize(MaxNumber + 1);

        switch (outline.boundary)
        {
        case absolute:
        {
            float min = trajectory.MinResiduesRMSD;
            float max = trajectory.MaxResiduesRMSD;
            scheme = GetColorScheme(min, max, outline.palette, outline.size);

            outline.schemes += scheme;

            for (auto number : list)
            {
                Residue residue = trajectory.residues[number];

                float value = residue.RMSDs[playback.step];
                outline.colours[number] = FromQColorToQVector3D(GetColorStep(scheme, value).color);
            }

            break;
        }
        case relative:
        {
            outline.schemes.resize(MaxNumber + 1);

            for (auto number : list)
            {
                Residue residue = trajectory.residues[number];

                float min = residue.MinRMSD;
                float max = residue.MaxRMSD;
                scheme = GetColorScheme(min, max, outline.palette, outline.size);

                outline.schemes[number] = scheme;

                float value = residue.RMSDs[playback.step];
                outline.colours[number] = FromQColorToQVector3D(GetColorStep(scheme, value).color);
            }
            break;
        }
        }

        break;
    }
    case ATOM_RMSF:
    {
        // qDebug() << "ATOM_RMSF";

        auto list = trajectory.atoms.keys();
        int MaxNumber = *std::max_element(list.begin(), list.end());

        // outline.colors.clear();
        outline.colours.resize(MaxNumber + 1);

        switch (outline.boundary)
        {
        case absolute:
        case relative:
        {
            float min = trajectory.MinAtomsRMSF;
            float max = trajectory.MaxAtomsRMSF;
            scheme = GetColorScheme(min, max, outline.palette, outline.size);

            outline.schemes += scheme;

            for (auto number : list)
            {
                Atom atom = trajectory.atoms[number];

                float value = atom.RMSF;
                outline.colours[number] = FromQColorToQVector3D(GetColorStep(scheme, value).color);
            }

            break;
        }
        }

        break;
    }
    case ATOM_RMSD:
    {
        // qDebug() << "ATOM_RMSD";

        auto list = trajectory.atoms.keys();
        int MaxNumber = *std::max_element(list.begin(), list.end());

        // outline.colors.clear();
        outline.colours.resize(MaxNumber + 1);

        switch (outline.boundary)
        {
        case absolute:
        {
            float min = trajectory.MinAtomsRMSD;
            float max = trajectory.MaxAtomsRMSD;
            scheme = GetColorScheme(min, max, outline.palette, outline.size);

            outline.schemes += scheme;

            for (auto number : list)
            {
                Atom atom = trajectory.atoms[number];

                float value = atom.RMSDs[playback.step].lengthSquared();
                outline.colours[number] = FromQColorToQVector3D(GetColorStep(scheme, value).color);
            }

            break;
        }
        case relative:
        {
            outline.schemes.resize(MaxNumber + 1);

            for (auto number : list)
            {
                Atom atom = trajectory.atoms[number];

                float min = atom.MinRMSD;
                float max = atom.MaxRMSD;
                scheme = GetColorScheme(min, max, outline.palette, outline.size);

                outline.schemes[number] = scheme;

                float value = atom.RMSDs[playback.step].lengthSquared();
                outline.colours[number] = FromQColorToQVector3D(GetColorStep(scheme, value).color);
            }
            break;
        }
        }

        break;
    }
    }

    outline.OutlineTextureFlag = true;

    // outline.scheme = scheme;
    emit ColorSchemeSignal(flag);
}
*/

void OpenGLWidget::SetOutlineColor(bool flag)
{
    ColorScheme scheme;

    outline.schemes.clear();
    outline.colours.clear();

    auto FilterColor = [=] (ColorStep step)
    {
        QColor color = (step.number >= outline.filter) ? step.color : Qt::GlobalColor::black;
        return FromQColorToQVector3D(color);
    };

    switch (outline.mode)
    {
    case RESIDUE_RMSF:
    {
        // qDebug() << "RESIDUE_RMSF";

        auto list = trajectory.residues.keys();
        int MaxNumber = *std::max_element(list.begin(), list.end());

        // outline.colors.clear();
        outline.colours.resize(MaxNumber + 1);

        switch (outline.boundary)
        {
        case absolute:
        case relative:
        {
            float min = trajectory.MinResiduesRMSF;
            float max = trajectory.MaxResiduesRMSF;
            scheme = GetColorScheme(min, max, outline.palette, outline.size);

            outline.schemes += scheme;

            for (auto number : list)
            {
                Residue residue = trajectory.residues[number];

                float value = residue.RMSF;
                // outline.colours[number] = FromQColorToQVector3D(GetColorStep(scheme, value).color);
                outline.colours[number] = FilterColor(GetColorStep(scheme, value));


                /*
                ColorStep step = GetColorStep(scheme, residue.RMSF);

                QColor color = (step.number >= outline.filter) ? step.color : Qt::GlobalColor::black;
                outline.colours[number] = FromQColorToQVector3D(color);
                */
            }

            break;
        }
        }

        break;
    }
    case RESIDUE_RMSD:
    {
        // qDebug() << "RESIDUE_RMSD";

        auto list = trajectory.residues.keys();
        int MaxNumber = *std::max_element(list.begin(), list.end());

        // outline.colors.clear();
        outline.colours.resize(MaxNumber + 1);

        switch (outline.boundary)
        {
        case absolute:
        {
            float min = trajectory.MinResiduesRMSD;
            float max = trajectory.MaxResiduesRMSD;
            scheme = GetColorScheme(min, max, outline.palette, outline.size);

            outline.schemes += scheme;

            for (auto number : list)
            {
                Residue residue = trajectory.residues[number];

                float value = residue.RMSDs[playback.step];
                // outline.colours[number] = FromQColorToQVector3D(GetColorStep(scheme, value).color);
                outline.colours[number] = FilterColor(GetColorStep(scheme, value));
            }

            break;
        }
        case relative:
        {
            outline.schemes.resize(MaxNumber + 1);

            for (auto number : list)
            {
                Residue residue = trajectory.residues[number];

                float min = residue.MinRMSD;
                float max = residue.MaxRMSD;
                scheme = GetColorScheme(min, max, outline.palette, outline.size);

                outline.schemes[number] = scheme;

                float value = residue.RMSDs[playback.step];
                // outline.colours[number] = FromQColorToQVector3D(GetColorStep(scheme, value).color);
                outline.colours[number] = FilterColor(GetColorStep(scheme, value));
            }
            break;
        }
        }

        break;
    }
    case ATOM_RMSF:
    {
        // qDebug() << "ATOM_RMSF";

        auto list = trajectory.atoms.keys();
        int MaxNumber = *std::max_element(list.begin(), list.end());

        // outline.colors.clear();
        outline.colours.resize(MaxNumber + 1);

        switch (outline.boundary)
        {
        case absolute:
        case relative:
        {
            float min = trajectory.MinAtomsRMSF;
            float max = trajectory.MaxAtomsRMSF;
            scheme = GetColorScheme(min, max, outline.palette, outline.size);

            outline.schemes += scheme;

            for (auto number : list)
            {
                Atom atom = trajectory.atoms[number];

                float value = atom.RMSF;
                // outline.colours[number] = FromQColorToQVector3D(GetColorStep(scheme, value).color);
                outline.colours[number] = FilterColor(GetColorStep(scheme, value));
            }

            break;
        }
        }

        break;
    }
    case ATOM_RMSD:
    {
        // qDebug() << "ATOM_RMSD";

        auto list = trajectory.atoms.keys();
        int MaxNumber = *std::max_element(list.begin(), list.end());

        // outline.colors.clear();
        outline.colours.resize(MaxNumber + 1);

        switch (outline.boundary)
        {
        case absolute:
        {
            float min = trajectory.MinAtomsRMSD;
            float max = trajectory.MaxAtomsRMSD;
            scheme = GetColorScheme(min, max, outline.palette, outline.size);

            outline.schemes += scheme;

            for (auto number : list)
            {
                Atom atom = trajectory.atoms[number];

                float value = atom.RMSDs[playback.step].lengthSquared();
                // outline.colours[number] = FromQColorToQVector3D(GetColorStep(scheme, value).color);
                outline.colours[number] = FilterColor(GetColorStep(scheme, value));
            }

            break;
        }
        case relative:
        {
            outline.schemes.resize(MaxNumber + 1);

            for (auto number : list)
            {
                Atom atom = trajectory.atoms[number];

                float min = atom.MinRMSD;
                float max = atom.MaxRMSD;
                scheme = GetColorScheme(min, max, outline.palette, outline.size);

                outline.schemes[number] = scheme;

                float value = atom.RMSDs[playback.step].lengthSquared();
                // outline.colours[number] = FromQColorToQVector3D(GetColorStep(scheme, value).color);
                outline.colours[number] = FilterColor(GetColorStep(scheme, value));
            }
            break;
        }
        }

        break;
    }
    }

    outline.OutlineTextureFlag = true;

    // outline.scheme = scheme;
    emit ColorSchemeSignal(flag);
}
