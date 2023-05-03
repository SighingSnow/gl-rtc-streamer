//
// Created by 宋庭聿 on 2021/10/22.
//

#include "scene.h"

float lastX = screen_width / 2.0f;
float lastY = screen_height / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

Scene::Scene()
{
    window = nullptr;
    VAO = 0;
    VBO = 0;
    mShader = nullptr;
    camera = new Camera(glm::vec3(1.0,2.18f,8.91f));
    //camera = new Camera(glm::vec3(0.0f,0.0f,3.0f));
    lightPos = glm::vec3(1.0f, 2.0f, 1.0f);
    prev_time = 0.0;
    frame_count = 0;
    delta_t = 0.0;
}

void Scene::AttachStreamer(Streamer *streamer)
{
    streamer_ =  streamer;
}

void Scene::SetUpEnv()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_REFRESH_RATE,30);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    window = glfwCreateWindow(screen_width, screen_height, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return ;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
    }

}

void Scene::SetObjs()
{
    // Compile shaders
    //lightShader = new Shader(cube_vs.c_str(),cube_fs.c_str());
	//cShader = new Shader(cube_vs.c_str(),cube_fs.c_str());
    skybox_ = new SkyBox(*camera);
    skybox_->init();
    mShader = new Shader(mcube_vs.c_str(),mcube_fs.c_str());
    textShader = new Shader(text_vs.c_str(),text_fs.c_str());
    modelShader_ = new Shader(model_vs.c_str(),model_fs.c_str());
    // Bind buffer
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindVertexArray(VAO);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    loadFonts();
    // This is for the text
    glGenVertexArrays(1, &tVAO);
    glGenBuffers(1, &tVBO);
    glBindVertexArray(tVAO);
    glBindBuffer(GL_ARRAY_BUFFER, tVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

}

void Scene::initScene()
{
    initBackground();
    // initGround();
    // initTrees();
    initModel();
}

void Scene::SetCallback()
{
    glfwSetWindowUserPointer(window,this);
    glfwSetCursorPosCallback(window,mouse_callback);
    glfwSetScrollCallback(window,scroll_callback);
    glfwSetFramebufferSizeCallback(window,framebuffer_size_callback);
}

Camera* Scene::get_camera() {
    return camera;
}

void Scene::DrawScene()
{
    int fps;
    int width = 0,height = 0;

    glEnable(GL_DEPTH_TEST);
    time_t now = time(NULL);

    initScene();
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        frame_count++;
        delta_t = currentFrame -prev_time;
        if(delta_t >= 1.0){
            std::string title = std::to_string((double)frame_count/delta_t) + " fps";
            glfwSetWindowTitle(window,title.c_str());
            frame_count = 0;
            prev_time = currentFrame;
        }
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.8f, 0.7f, 0.7f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glfwGetWindowSize(window,&mWidth_,&mHeight_);
        // generalTransform(); // perspective and view matrix
        renderScene();
        //renderTime();

        // Read buffer
        GLubyte buf[SCR_WIDTH*SCR_HEIGHT*3] = {0};
        glReadPixels(0,0,SCR_WIDTH,SCR_HEIGHT,GL_RGB,GL_UNSIGNED_BYTE,buf);
        Notify(buf);
        memset(buf,0,SCR_WIDTH*SCR_HEIGHT*3);
        
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void Scene::generalTransform()
{
    mShader->use();
    glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom), (float)screen_width / (float)screen_height, 0.1f, 100.0f);
    glm::mat4 view = camera->GetViewMatrix();
    mShader->setMat4("projection", projection);
    mShader->setMat4("view", view);
}

void Scene::initBackground()
{
    glm::vec3 lightColor = glm::vec3(1,0.8,0.6);
	glm::vec3 diffuseColor = lightColor * glm::vec3(0.8f);
	glm::vec3 ambientColor = diffuseColor * glm::vec3(1.0f);

    mShader->use();
    mShader->setVec3("lightPos",lightPos);
    mShader->setVec3("viewPos", camera->Position);

    mShader->setVec3("light.ambient",ambientColor);
	mShader->setVec3("light.diffuse",diffuseColor);
	mShader->setVec3("light.specular",1.0f,1.0f,1.0f);
    mShader->setVec3("lightColor", 1.0f, 0.8f, 0.6f);
    
}

void Scene::initGround()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0,1.0);
    ground_cubes = vector<MCube>(3);

    glm::vec3 color_noise = glm::vec3(0.1f);
    double i = 0.0;
    for(auto & gc:ground_cubes){
        gc.scale_ = glm::vec3(20,1.0f,20.0f);
        gc.pos_ = glm::vec3(0.0f*i,-i*voxel_size*gc.scale_[1],0.0f);
        gc.color_ = glm::vec3(0.8-i*0.1)*glm::vec3(1.0f,0.8f,0.6f) + 
            color_noise*glm::vec3(dis(gen));
        gc.angle_ = 60.0;
        gc.rotate_axis = glm::vec3(0.0,1.0,0.0);
        i = i+1;
    }
}

void Scene::initTrees()
{  
    std::random_device rd;
    std::mt19937 gen(rd());
    uniform_real_distribution<> dis(0.0,1.0);
    uniform_real_distribution<> bdis(1.0,1.2);
    uniform_real_distribution<> pos_dis(-0.04,0.04); 
    // init tree size;
    trees = std::vector<MTree>(tree_num);
    auto initBranch = [&](MTree& mtree){
        double index = 0;
        glm::vec3 pos = mtree.pos_;
        int one_b_height = bdis(gen);
        std::vector<MCube>& b = mtree.branch_;
        for(MCube & mc: b){
            mc.scale_ = glm::vec3(0.4f,one_b_height,0.4f);
            mc.pos_ = glm::vec3(pos[0],
                pos[1]+index*voxel_size*mc.scale_[1],
                pos[2]);
            mc.color_ = glm::vec3(0.9);
            mc.angle_ = 0.0f;
            mc.rotate_axis = glm::vec3(0.0); 
            index = index+1;
        }
        mtree.theight = 3*one_b_height*voxel_size;
    };
    
    auto initLeaves = [&](glm::vec3 pos,double height,int radius,std::vector<MCube>& leaves){
        double h,d,prob,r;
        MCube leaf;
        for(double i = -radius;i <= radius;i+=1){
            for(double j = -radius;j <= radius;j+=1){
                for(double k = -radius;k <= radius;k+=1){
                    glm::vec3 f = glm::vec3(i/radius,j/radius,k/radius);
                    h = 0.5 - std::max(double(f[1]),-0.5) * 0.5;
                    d = sqrt(std::pow(f[0],2)+pow(f[2],2));
                    prob = std::pow(std::max(0.0,double(1-d)),2) * h;
                    prob += sin(f[0] * 5 + pos[0]) * 0.02;
                    prob += sin(f[1] * 9 + pos[1]) * 0.01;
                    prob += sin(f[2] * 10 + pos[2]) * 0.03;
                    prob = prob < 0.1 ? 0.0:prob;
                    r = dis(gen);
                    if(r < prob){
                        leaf.pos_ = pos + glm::vec3(i/radius/60.0,height+j/radius/100.0,k/radius/60.0);
                        leaf.scale_ = glm::vec3(0.1,0.1,0.1);
                        leaf.color_ = glm::vec3(1.0, 0.4, 0.1);
                        leaves.emplace_back(leaf);
                    }
                }
            }
        }
    };

    for(auto& t:trees){
        t.pos_ = glm::vec3(0.0,0.01,0.0);
        t.pos_[0] = pos_dis(gen);
        t.pos_[2] = pos_dis(gen);
        t.branch_ = std::vector<MCube>(3);
        // tmp
        t.radius = 22;
        initBranch(t);
        initLeaves(t.pos_,t.theight,t.radius,t.leaves_);
        //std::cout<<"leaves num"<<t.leaves_.size()<<std::endl;
    }
    return;
}

void Scene::initModel()
{
    Model * model = new Model("../resource/mesh/mario/Mario.obj");
    models_.push_back(model);
}

void Scene::renderScene()
{
    // renderGround();
    // renderTree();
    renderModel();
    skybox_->draw();
}
void Scene::renderModel()
{
    modelShader_->use();
    glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom), (float)mWidth_ / (float)mHeight_, 0.1f, 100.0f);
    glm::mat4 view = camera->GetViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);
    modelShader_->setMat4("view",view);
    modelShader_->setMat4("projection",projection);
    model = glm::scale(model,glm::vec3(0.01f));
    modelShader_->setMat4("model",model);
    for(auto & model_ : models_) {
        model_->Draw(*modelShader_);
    }
}
void Scene::renderGround()
{
    for(auto& gc:ground_cubes){
        renderCube(gc.pos_,gc.scale_,gc.color_,
            gc.angle_,gc.rotate_axis);
    }
}

void Scene::renderTree()
{
    for(MTree & t : trees) {
        for(MCube & m: t.branch_) {
            renderCube(m.pos_,m.scale_,m.color_);
        }
        for(MCube & m: t.leaves_) {
            renderCube(m.pos_,m.scale_,m.color_);
        }
    }
}

void Scene::renderCube(glm::vec3 pos,glm::vec3 scale,glm::vec3 color,
    double angle ,glm::vec3 rotate_axis
    )
{
    mShader->use();
    mShader->setVec3("material.ambient", color);
    mShader->setVec3("material.diffuse", color);
    mShader->setVec3("material.specular", 0.4f, 0.4f, 0.4f); 
    mShader->setFloat("material.shininess", 20.0f);
    // transfom
    glm::mat4 model = glm::mat4(1.0);
    model = glm::translate(model,pos);
    //model = glm::rotate(model,glm::radians(-120.0f),rotate_axis);
    model = glm::scale(model,scale*glm::vec3(voxel_size));

    mShader->setMat4("model",model);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES,0,36);
    return;
}

void Scene::renderLight()
{
    glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera->GetViewMatrix();
        
    cShader->use();
    cShader->setMat4("projection", projection);
    cShader->setMat4("view", view);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, lightPos);
    model = glm::scale(model, glm::vec3(voxel_size*10)); // a smaller cube
    cShader->setMat4("model", model);

    glBindVertexArray(lightCubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void Scene::renderTime()
{
    textShader->use();
    glm::mat4 tproj = glm::ortho(0.0f, static_cast<GLfloat>(SCR_WIDTH), 0.0f, static_cast<GLfloat>(SCR_HEIGHT));
    textShader->setMat4("projection",tproj);
    auto ms = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch());
    std::string ms_text = std::to_string(ms.count());
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    renderText(ms_text,25.0f, 25.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
    glDisable(GL_BLEND);
}

void Scene::Notify(uint8_t* buf) {
    streamer_->encode(buf);
}

void Scene::Terminate()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
}

void Scene::loadFonts()
{
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

    FT_Face face;
    if (FT_New_Face(ft, "../src/fonts/notomono.ttf", 0, &face))
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
    
    FT_Set_Pixel_Sizes(face, 0, 48);

    // Disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); 
    // we only bind numbers
    for (GLubyte c = 48; c < 57; c++)
    {
        // Load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // Generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<GLuint>(face->glyph->advance.x)
        };
        Characters.insert(std::pair<GLchar, Character>(c, character));
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    // Destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

void Scene::renderText(std::string text,GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
{
    textShader->use();
    glUniform3f(glGetUniformLocation(textShader->ID, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(tVAO);

    // 遍历文本中所有的字符
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;
        // 对每个字符更新VBO
        GLfloat vertices[6][4] = {
            { xpos,     ypos + h,   0.0, 0.0 },            
            { xpos,     ypos,       0.0, 1.0 },
            { xpos + w, ypos,       1.0, 1.0 },

            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos + w, ypos,       1.0, 1.0 },
            { xpos + w, ypos + h,   1.0, 0.0 }           
        };
        // 在四边形上绘制字形纹理
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // 更新VBO内存的内容
        glBindBuffer(GL_ARRAY_BUFFER, tVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // 绘制四边形
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // 更新位置到下一个字形的原点，注意单位是1/64像素
        x += (ch.Advance >> 6) * scale; // 位偏移6个单位来获取单位为像素的值 (2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Scene::clientKeyboardCallback(char c)
{
    switch(c){
        case 'w':this->get_camera()->ProcessKeyboard(FORWARD,deltaTime); break;
        case 's':this->get_camera()->ProcessKeyboard(BACKWARD,deltaTime); break;
        case 'a':this->get_camera()->ProcessKeyboard(LEFT,deltaTime); break;
        case 'd':this->get_camera()->ProcessKeyboard(RIGHT,deltaTime); break;
        default: break;
    }
}

void Scene::clientMouseMoveCallback(double x,double y)
{
    if (firstMouse)
    {
        lastX = x;
        lastY = y;
        firstMouse = false;
    }

    float xoffset = x - lastX;
    float yoffset = lastY - y; // reversed since y-coordinates go from bottom to top

    lastX = x;
    lastY = y;

    this->get_camera()->ProcessMouseMovement(xoffset, yoffset);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    void* data = glfwGetWindowUserPointer(window);
    Scene* scene = static_cast<Scene*>(data);

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        scene->get_camera()->ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        scene->get_camera()->ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        scene->get_camera()->ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        scene->get_camera()->ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    void* data = glfwGetWindowUserPointer(window);
    Scene* scene = static_cast<Scene*>(data);
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    scene->get_camera()->ProcessMouseMovement(xoffset, yoffset);
}

// glfssw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    void* data = glfwGetWindowUserPointer(window);
    Scene* scene = static_cast<Scene*>(data);
    scene->get_camera()->ProcessMouseScroll(yoffset);
}
