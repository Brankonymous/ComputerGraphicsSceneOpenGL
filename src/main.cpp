#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

unsigned int loadCubemap(vector<std::string> faces);

bool SHADOW_FLAG = true;

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
float exposure = 1.0;

// camera
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct ProgramState {
    glm::vec3 clearColor = glm::vec3(0);
    bool ImGuiEnabled = false;
    Camera camera;
    bool CameraMouseMovementUpdateEnabled = true;
    glm::vec3 backpackPosition = glm::vec3(0.0f);
    glm::vec3 grassPosition = glm::vec3(0.0f);
    glm::vec3 carPosition = glm::vec3(-3.0f, 1.2f, 2.0f);
    glm::vec3 lampPosition = glm::vec3(-20.0f, 1.0f, 13.0f);
    glm::vec3 lamp2Position = glm::vec3(4.0f, 0.0f, 0.0f);
    glm::vec3 catPosition = glm::vec3(-160.0f, 50.0f, -223.0f);
    glm::vec3 roadPosition = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 tablePosition = glm::vec3(5.0f, 1.0f, 4.5f);
    glm::vec3 flowerPosition = glm::vec3(5.0f, 1.1f, 5.5f);
    glm::vec3 treePosition = glm::vec3(4.5f, 0.0f, -4.5f);
    glm::vec3 doorPosition = glm::vec3(3.5f, 0.0f, -4.5f);

    float grassScale = 0.05f;
    float carScale = 0.8f;
    float lampScale = 0.2f;
    float lamp2Scale = 1.2f;
    float catScale = 0.015f;
    float roadScale = 5.0f;
    float tableScale = 0.006f;
    float flowerScale = 2.0f;
    float treeScale = 0.008f;
    float doorScale = 0.05f;

    PointLight pointLight;
    ProgramState()
            : camera(glm::vec3(0.0f, 0.0f, 3.0f)) {}

    void SaveToFile(std::string filename);

    void LoadFromFile(std::string filename);

    glm::vec3 pointLightPositions[3] = {
            glm::vec3(4.8f, 4.0f, 0.9f),
            glm::vec3(-2.3f,1.0f,-0.3f),
            glm::vec3(-3.3f,4.0f,3.2f)
    };

    glm::vec3 diffuse_plight = glm::vec3(4.5f, 0.0f, -4.5f);
    glm::vec3 specular_plight = glm::vec3(0.0f,5.0f,5.0f);
    glm::vec3 ambient_plight = glm::vec3(0.0f, 0.0f, 0.0f);
    float constant_plight = 1.0f;
    float linear_plight = 1.0f;
    float quadratic_plight = 1.0f;
    float shininess_plight = 32.0f;
    glm::vec3 direction_slight = glm::vec3(0.0f, -1.0f, 0.0f);
    glm::vec3 ambient_slight = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 diffuse_slight = glm::vec3(0.0f, 3.0f, 3.0f);
    glm::vec3 specular_slight = glm::vec3(0.0f, 3.0f, 3.0f);
    float constant_slight = 1.0f;
    float linear_slight = 1.0f;
    float quadratic_slight = 1.0f;
    float cutOff_slight = 1.0f;
    float outerCutOff_slight = 20.0f;
};

void renderScene(Shader &shader, vector<Model> &models);

void ProgramState::SaveToFile(std::string filename) {
    std::ofstream out(filename);
    out << clearColor.r << '\n'
        << clearColor.g << '\n'
        << clearColor.b << '\n'
        << ImGuiEnabled << '\n'
        << camera.Position.x << '\n'
        << camera.Position.y << '\n'
        << camera.Position.z << '\n'
        << camera.Front.x << '\n'
        << camera.Front.y << '\n'
        << camera.Front.z << '\n';
}

void ProgramState::LoadFromFile(std::string filename) {
    std::ifstream in(filename);
    if (in) {
        in >> clearColor.r
           >> clearColor.g
           >> clearColor.b
           >> ImGuiEnabled
           >> camera.Position.x
           >> camera.Position.y
           >> camera.Position.z
           >> camera.Front.x
           >> camera.Front.y
           >> camera.Front.z;
    }
}

ProgramState *programState;

void DrawImGui(ProgramState *programState);

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    programState = new ProgramState;
    programState->LoadFromFile("resources/program_state.txt");
    if (programState->ImGuiEnabled) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    // Init Imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    // enable blending and culling face
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CW);

    // build and compile shaders
    // -------------------------
    Shader ourShader("resources/shaders/advanced_lightning.vs", "resources/shaders/advanced_lightning.fs");
    Shader skyboxShader("resources/shaders/skybox.vs","resources/shaders/skybox.fs");
    Shader shadowShader("resources/shaders/shadows.vs", "resources/shaders/shadows.fs", "resources/shaders/shadows.geom");
    Shader blurShader("resources/shaders/blur.vs", "resources/shaders/blur.fs");
    Shader bloomShader("resources/shaders/bloom.vs", "resources/shaders/bloom.fs");

    float skyboxVertices[] = {
            // positions
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
    };

    float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };

    vector<std::string> skyboxImages =
            {
                    "resources/objects/skybox/right.jpg",
                    "resources/objects/skybox/left.jpg",
                    "resources/objects/skybox/bottom.jpg",
                    "resources/objects/skybox/top.jpg",
                    "resources/objects/skybox/front.jpg",
                    "resources/objects/skybox/back.jpg"
            };
    unsigned int cubemapTexture = loadCubemap(skyboxImages);

    // load models
    // -----------
    vector<Model> models;
    Model grassModel("resources/objects/grass/10450_Rectangular_Grass_Patch_v1_iterations-2.obj"); models.push_back(grassModel);
    Model carModel("resources/objects/car/S15_bonnet.obj"); models.push_back(carModel);
    Model lampModel("resources/objects/Street Lamp/StreetLamp.obj"); models.push_back(lampModel);
    Model lamp2Model("resources/objects/lamp2/source/street-lamp-obj/farola1.obj"); models.push_back(lamp2Model);
    Model catModel("resources/objects/cat/source/cat-obj/cat.obj"); models.push_back(catModel);
    Model tableModel("resources/objects/table/source/table/table.obj"); models.push_back(tableModel);
    Model flowerModel("resources/objects/flower/Scaniverse.obj"); models.push_back(flowerModel);
    Model treeModel("resources/objects/coconutTree/coconutTreeBended.obj"); models.push_back(treeModel);
    Model doorModel("resources/objects/glassdoor/Glass Door.obj"); models.push_back(doorModel);

    grassModel.SetShaderTextureNamePrefix("material.");
    carModel.SetShaderTextureNamePrefix("material.");
    lampModel.SetShaderTextureNamePrefix("material.");
    lamp2Model.SetShaderTextureNamePrefix("material.");
    catModel.SetShaderTextureNamePrefix("material.");
    tableModel.SetShaderTextureNamePrefix("material.");
    flowerModel.SetShaderTextureNamePrefix("material.");
    treeModel.SetShaderTextureNamePrefix("material.");
    doorModel.SetShaderTextureNamePrefix("material.");

    PointLight& pointLight = programState->pointLight;
    pointLight.position = glm::vec3(-4.0f,2.7f,-1.6f);
    pointLight.ambient = glm::vec3(0.5, 0.1, 0.1);
    pointLight.diffuse = glm::vec3(0.95f, 1 ,1);
    pointLight.specular = glm::vec3(1.0, 1.0, 1.0);
    pointLight.constant = 0.6f;
    pointLight.linear = 1.0f;
    pointLight.quadratic = 0.0f;

    glm::vec3 ambient(0,0,0);

    /* BLOOM AND HDR IMPLEMENTATION - NOT WORKING
    // ---------------------- code copied from learnopengl ----------------------- //
    // configure (floating point) framebuffers
    // ---------------------------------------
    unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    // create 2 floating point color buffers (1 for normal rendering, other for brightness threshold values)
    unsigned int colorBuffers[2];
    glGenTextures(2, colorBuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // attach texture to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
    }
    // create and attach depth buffer (renderbuffer)
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ping-pong-framebuffer for blurring
    unsigned int pingpongFBO[2];
    unsigned int pingpongColorbuffers[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
        // also check if framebuffers are complete (no need for depth buffer)
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
    }
    // ---------------------- code copied from learnopengl ----------------------- //

    // setup quad VAO
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
     */

    // configure depth map FBO
    // -----------------------
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    // create depth cubemap texture
    unsigned int depthCubemap;
    glGenTextures(1, &depthCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
    for (unsigned int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // SkyBox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        glm::mat4 model = glm::mat4(1.0f);

        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);


        // render
        // ------
        glClearColor(programState->clearColor.r, programState->clearColor.g, programState->clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // don't forget to enable shader before setting uniforms
        ourShader.use();
        ourShader.setVec3("cameraPos", programState->camera.Position);

        // FIRST LIGHT SOURCE -------------------------------------------------------=
        ourShader.setVec3("pointLight[0].position", programState->pointLightPositions[0]);
        ourShader.setVec3("pointLight[0].ambient", glm::vec3(0.0f, 4.0f, 10.0f));
        ourShader.setVec3("pointLight[0].diffuse", glm::vec3(0.5f, 0.0f, -2.5f));
        ourShader.setVec3("pointLight[0].specular", glm::vec3(-1.0f, 5.0f, 16.0f));
        ourShader.setFloat("pointLight[0].constant", 1.0f);
        ourShader.setFloat("pointLight[0].linear", 1.0f);
        ourShader.setFloat("pointLight[0].quadratic", 0.2f);
        ourShader.setVec3("viewPosition", programState->camera.Position);
        ourShader.setFloat("material.shininess", 32.0f);

        ourShader.setVec3("spotLight[0].position", programState->pointLightPositions[0]);
        ourShader.setVec3("spotLight[0].direction", glm::vec3(0.0f, -1.0f, 0.0f));
        ourShader.setVec3("spotLight[0].ambient", glm::vec3(0.0f, -4.0f, -1.0f));
        ourShader.setVec3("spotLight[0].diffuse", glm::vec3(-1.0f, 0.0f, 1.0f));
        ourShader.setVec3("spotLight[0].specular", glm::vec3(2.0f, 0.0f, 0.0f));
        ourShader.setFloat("spotLight[0].constant", 1.0f);
        ourShader.setFloat("spotLight[0].linear", 1.0f);
        ourShader.setFloat("spotLight[0].quadratic", 0.35f);
        ourShader.setFloat("spotLight[0].cutOff", glm::cos(glm::radians(1.0f)));
        ourShader.setFloat("spotLight[0].outerCutOff", glm::cos(glm::radians(50.0f)));

        //SECOND LIGHT SOURCE -------------------------
        ourShader.setVec3("pointLight[1].position", programState->pointLightPositions[1]);
        ourShader.setVec3("pointLight[1].ambient", glm::vec3(2.0f, 2.0f, -2.0f));
        ourShader.setVec3("pointLight[1].diffuse", glm::vec3(5.5f, 3.0f, -14.5f));
        ourShader.setVec3("pointLight[1].specular", glm::vec3(21.0f, 0.0f, 0.0f));
        ourShader.setFloat("pointLight[1].constant", 2.4f);
        ourShader.setFloat("pointLight[1].linear", 0.75f);
        ourShader.setFloat("pointLight[1].quadratic", 0.7f);
        ourShader.setVec3("viewPosition", programState->camera.Position);
        ourShader.setFloat("material.shininess", 32.0f);

        ourShader.setVec3("spotLight[1].position", programState->pointLightPositions[1]);
        ourShader.setVec3("spotLight[1].direction", glm::vec3(0.0f, 0.0f, 1.0f));
        ourShader.setVec3("spotLight[1].ambient", glm::vec3(10.0f, -1.0f, 2.0f));
        ourShader.setVec3("spotLight[1].diffuse", glm::vec3(34.0f, 4.0f, 12.0f));
        ourShader.setVec3("spotLight[1].specular", glm::vec3(-6.0f, 15.0f, 9.0f));
        ourShader.setFloat("spotLight[1].constant", 2.7f);
        ourShader.setFloat("spotLight[1].linear", 0.0f);
        ourShader.setFloat("spotLight[1].quadratic", 5.1f);
        ourShader.setFloat("spotLight[1].cutOff", glm::cos(glm::radians(0.750f)));
        ourShader.setFloat("spotLight[1].outerCutOff", glm::cos(glm::radians(90.0f)));

        //THIRD LIGHT SOURCE------------------------------------------
        ourShader.setVec3("pointLight[2].position", programState->pointLightPositions[2]);
        ourShader.setVec3("pointLight[2].ambient", glm::vec3(22.0f, -48.0f, 0.0f));
        ourShader.setVec3("pointLight[2].diffuse", glm::vec3(0.0f, 10.0f, -2.0f));
        ourShader.setVec3("pointLight[2].specular", glm::vec3(41.0f, 4.0f, -22.0f));
        ourShader.setFloat("pointLight[2].constant", 0.9f);
        ourShader.setFloat("pointLight[2].linear", 1.6f);
        ourShader.setFloat("pointLight[2].quadratic", 2.5f);
        ourShader.setVec3("viewPosition", programState->camera.Position);
        ourShader.setFloat("material.shininess", 32.0f);

        ourShader.setVec3("spotLight[2].position", programState->pointLightPositions[2]);
        ourShader.setVec3("spotLight[2].direction", glm::vec3(0.0f, -1.0f, 0.0f));
        ourShader.setVec3("spotLight[2].ambient", glm::vec3(9.0f, 1.0f, 0.0f));
        ourShader.setVec3("spotLight[2].diffuse", glm::vec3(25.0f, 36.0f, -5.0f));
        ourShader.setVec3("spotLight[2].specular", glm::vec3(2.0f, 1.0f, -5.0f));
        ourShader.setFloat("spotLight[2].constant", 0.5f);
        ourShader.setFloat("spotLight[2].linear", 0.65f);
        ourShader.setFloat("spotLight[2].quadratic", 0.2f);
        ourShader.setFloat("spotLight[2].cutOff", glm::cos(glm::radians(1.0f)));
        ourShader.setFloat("spotLight[2].outerCutOff", glm::cos(glm::radians(40.0f)));

        float near_plane = 1.0f;
        float far_plane  = 10.0f;

        if (SHADOW_FLAG) {
            glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float) SHADOW_WIDTH / (float) SHADOW_HEIGHT,
                                                    near_plane, far_plane);
            std::vector<glm::mat4> shadowTransforms;
            for (int i = 1; i < 2; i++) {
                // 0. create depth cube map transformation matrices
                // ------------------------------------------------
                shadowTransforms.clear();
                shadowTransforms.push_back(shadowProj * glm::lookAt(programState->pointLightPositions[i],
                                                                    programState->pointLightPositions[i] +
                                                                    glm::vec3(1.0f, 0.0f, 0.0f),
                                                                    glm::vec3(0.0f, -1.0f, 0.0f)));
                shadowTransforms.push_back(shadowProj * glm::lookAt(programState->pointLightPositions[i],
                                                                    programState->pointLightPositions[i] +
                                                                    glm::vec3(-1.0f, 0.0f, 0.0f),
                                                                    glm::vec3(0.0f, -1.0f, 0.0f)));
                shadowTransforms.push_back(shadowProj * glm::lookAt(programState->pointLightPositions[i],
                                                                    programState->pointLightPositions[i] +
                                                                    glm::vec3(0.0f, 1.0f, 0.0f),
                                                                    glm::vec3(0.0f, 0.0f, 1.0f)));
                shadowTransforms.push_back(shadowProj * glm::lookAt(programState->pointLightPositions[i],
                                                                    programState->pointLightPositions[i] +
                                                                    glm::vec3(0.0f, -1.0f, 0.0f),
                                                                    glm::vec3(0.0f, 0.0f, -1.0f)));
                shadowTransforms.push_back(shadowProj * glm::lookAt(programState->pointLightPositions[i],
                                                                    programState->pointLightPositions[i] +
                                                                    glm::vec3(0.0f, 0.0f, 1.0f),
                                                                    glm::vec3(0.0f, -1.0f, 0.0f)));
                shadowTransforms.push_back(shadowProj * glm::lookAt(programState->pointLightPositions[i],
                                                                    programState->pointLightPositions[i] +
                                                                    glm::vec3(0.0f, 0.0f, -1.0f),
                                                                    glm::vec3(0.0f, -1.0f, 0.0f)));

                // Render scene to depth cube map
                // ---------------------------------
                glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
                glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
                glClear(GL_DEPTH_BUFFER_BIT);
                shadowShader.use();
                for (unsigned int i = 0; i < 6; ++i)
                    shadowShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
                shadowShader.setFloat("far_plane", far_plane);
                shadowShader.setVec3("lightPos", programState->pointLightPositions[i]);
                renderScene(shadowShader, models);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
            }
        }

        ourShader.use();

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = programState->camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // set depthMaps
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ourShader.use();
        ourShader.setFloat("far_plane", far_plane);
        ourShader.setInt("depthMap", 1);
        ourShader.setBool("shadow_flag", SHADOW_FLAG);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);

        // Render the loaded models //
        renderScene(ourShader, models);

        // Draw Skybox
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        view = glm::mat4(glm::mat3(programState->camera.GetViewMatrix()));
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);

        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);

        /* BLOOM AND HDR IMPLEMENTATION - NOT WORKING
        // 2. blur bright fragments with two-pass Gaussian Blur
        // --------------------------------------------------
        bool horizontal = true, first_iteration = true;
        unsigned int amount = 10;
        blurShader.use();
        for (unsigned int i = 0; i < amount; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            blurShader.setInt("horizontal", horizontal);
            glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glBindVertexArray(0);
            horizontal = !horizontal;
            if (first_iteration)
                first_iteration = false;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 3. now render floating point color buffer to 2D quad and tonemap HDR colors to default framebuffer's (clamped) color range
        // --------------------------------------------------------------------------------------------------------------------------
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        bloomShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
        bloomShader.setInt("bloom", true);
        bloomShader.setFloat("exposure", exposure);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
        cout << "exposure: " << exposure << endl;
         */

        if (programState->ImGuiEnabled)
            DrawImGui(programState);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    programState->SaveToFile("resources/program_state.txt");
    delete programState;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------

    // deallocate
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
    // glDeleteVertexArrays(1, &quadVAO);
    // glDeleteBuffers(1, &quadVBO);

    glfwTerminate();
    return 0;
}

unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void renderScene(Shader &shader, vector<Model> &models) {
    // Render the loaded models //
    glm::mat4 model = glm::mat4(1.0f);

    // Grass model
    model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(programState->grassScale));
    model = glm::rotate(model, glm::radians(90.f), glm::vec3(-1.0, 0.0, 0.0));
    shader.setMat4("model", model);
    models[0].Draw(shader);

    // Car model
    model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(programState->carScale));
    model = glm::translate(model, programState->carPosition);
    shader.setMat4("model", model);
    models[1].Draw(shader);

    // Lamp model
    model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(programState->lampScale));
    model = glm::translate(model, programState->lampPosition);
    shader.setMat4("model", model);
    models[2].Draw(shader);

    // Lamp2 model
    model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(programState->lamp2Scale));
    model = glm::translate(model, programState->lamp2Position);
    shader.setMat4("model", model);
    models[3].Draw(shader);

    // Cat model
    model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(programState->catScale));
    model = glm::translate(model, programState->catPosition);
    shader.setMat4("model", model);
    models[4].Draw(shader);

    // Table model
    model = glm::mat4(1.0f);
    model = glm::translate(model, programState->tablePosition);
    model = glm::scale(model, glm::vec3(programState->tableScale));
    shader.setMat4("model", model);
    models[5].Draw(shader);

    // Flower model
    model = glm::mat4(1.0f);
    model = glm::translate(model, programState->flowerPosition);
    model = glm::scale(model, glm::vec3(programState->flowerScale));
    shader.setMat4("model", model);
    models[6].Draw(shader);

    // Tree model
    model = glm::mat4(1.0f);
    model = glm::translate(model, programState->treePosition);
    model = glm::scale(model, glm::vec3(programState->treeScale));
    shader.setMat4("model", model);
    models[7].Draw(shader);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(KEY_UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(KEY_DOWN, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(KEY_RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(KEY_LEFT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
        exposure = max(0.0, exposure-0.1);
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        exposure = min(1.0, exposure+0.1);

    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
        SHADOW_FLAG = !SHADOW_FLAG;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (programState->CameraMouseMovementUpdateEnabled)
        programState->camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    programState->camera.ProcessMouseScroll(yoffset);
}

void DrawImGui(ProgramState *programState) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();


    {
        static float f = 0.0f;
        ImGui::Begin("Debug params");
        ImGui::Text("Hello text");
        ImGui::SliderFloat("Float slider", &f, 0.0, 1.0);
        ImGui::ColorEdit3("Background color", (float *) &programState->clearColor);

        ImGui::DragFloat3("lightParams[1].position", (float*)&programState->pointLightPositions[2]);

        ImGui::DragFloat3("lightParams[1].ambient_plight", (float*)&programState->ambient_plight);
        ImGui::DragFloat3("lightParams[1].diffuse_plight", (float*)&programState->diffuse_plight);
        ImGui::DragFloat3("lightParams[1].specular_plight", (float*)&programState->specular_plight);
        ImGui::DragFloat("lightParams[1].constant_plight", &programState->constant_plight, 0.05, 0.0, 10.0);
        ImGui::DragFloat("lightParams[1].linear_plight", &programState->linear_plight, 0.05, 0.0, 10.0);
        ImGui::DragFloat("programState->quadratic_plight", &programState->quadratic_plight, 0.05, 0.0, 10.0);
        ImGui::DragFloat("material.shininess", &programState->shininess_plight, 0.05, 0.0, 100.0);

        ImGui::DragFloat3("lightParams[1].direction", (float*)&programState->direction_slight);
        ImGui::DragFloat3("lightParams[1].ambient_slight", (float*)&programState->ambient_slight);
        ImGui::DragFloat3("lightParams[1].diffuse_slight", (float*)&programState->diffuse_slight);
        ImGui::DragFloat3("lightParams[1].specular_slight", (float*)&programState->specular_slight);
        ImGui::DragFloat("lightParams[1].constant_slight", &programState->constant_slight, 0.05, 0.0, 10.0);
        ImGui::DragFloat("lightParams[1].linear_slight", &programState->linear_slight, 0.05, 0.0, 10.0);
        ImGui::DragFloat("lightParams[1].quadratic_slight", &programState->quadratic_slight, 0.05, 0.0, 10.0);
        ImGui::DragFloat("lightParams[1].cutOff_slight", &programState->cutOff_slight, 0.05, 0.0, 360.0);
        ImGui::DragFloat("lightParams[1].outerCutOff_slight", &programState->outerCutOff_slight, 0.05, 0.0, 360.0);

        ImGui::End();
    }

    {
        ImGui::Begin("Camera info");
        const Camera& c = programState->camera;
        ImGui::Text("Camera position: (%f, %f, %f)", c.Position.x, c.Position.y, c.Position.z);
        ImGui::Text("(Yaw, Pitch): (%f, %f)", c.Yaw, c.Pitch);
        ImGui::Text("Camera front: (%f, %f, %f)", c.Front.x, c.Front.y, c.Front.z);
        ImGui::Checkbox("Camera mouse update", &programState->CameraMouseMovementUpdateEnabled);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        programState->ImGuiEnabled = !programState->ImGuiEnabled;
        if (programState->ImGuiEnabled) {
            programState->CameraMouseMovementUpdateEnabled = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
}