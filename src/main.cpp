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

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

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

    float grassScale = 0.05f;
    float carScale = 0.8f;
    float lampScale = 0.2f;
    float lamp2Scale = 1.2f;
    float catScale = 0.015f;
    float roadScale = 5.0f;
    float tableScale = 0.006f;
    float flowerScale = 2.0f;
    float treeScale = 0.008f;

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

    // Enable Culling Face
    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_FRONT);
    // glFrontFace(GL_CW);

    // build and compile shaders
    // -------------------------
    Shader ourShader("resources/shaders/2.model_lighting.vs", "resources/shaders/2.model_lighting.fs");
    Shader skyboxShader("resources/shaders/skybox.vs","resources/shaders/skybox.fs");

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

    vector<std::string> skyboxImages =
            {
                    "resources/objects/skybox/right.jpg",
                    "resources/objects/skybox/left.jpg",
                    "resources/objects/skybox/top.jpg",
                    "resources/objects/skybox/bottom.jpg",
                    "resources/objects/skybox/front.jpg",
                    "resources/objects/skybox/back.jpg"
            };
    unsigned int cubemapTexture = loadCubemap(skyboxImages);

    // load models
    // -----------
    Model ourModel("resources/objects/backpack/backpack.obj");
    Model grassModel("resources/objects/grass/10450_Rectangular_Grass_Patch_v1_iterations-2.obj");
    Model carModel("resources/objects/car/S15_bonnet.obj");
    Model lampModel("resources/objects/Street Lamp/StreetLamp.obj");
    Model lamp2Model("resources/objects/lamp2/source/street-lamp-obj/farola1.obj");
    Model catModel("resources/objects/cat/source/cat-obj/cat.obj");
    Model tableModel("resources/objects/table/source/table/table.obj");
    Model flowerModel("resources/objects/flower/Scaniverse.obj");
    Model treeModel("resources/objects/coconutTree/coconutTreeBended.obj");

    ourModel.SetShaderTextureNamePrefix("material.");
    grassModel.SetShaderTextureNamePrefix("material.");
    carModel.SetShaderTextureNamePrefix("material.");
    lampModel.SetShaderTextureNamePrefix("material.");
    lamp2Model.SetShaderTextureNamePrefix("material.");
    catModel.SetShaderTextureNamePrefix("material.");
    tableModel.SetShaderTextureNamePrefix("material.");
    flowerModel.SetShaderTextureNamePrefix("material.");
    treeModel.SetShaderTextureNamePrefix("material.");

    PointLight& pointLight = programState->pointLight;
    pointLight.position = glm::vec3(-4.0f,2.7f,-1.6f);
    pointLight.ambient = glm::vec3(0.5, 0.1, 0.1);
    pointLight.diffuse = glm::vec3(0.95f, 1 ,1);
    pointLight.specular = glm::vec3(1.0, 1.0, 1.0);
    pointLight.constant = 0.6f;
    pointLight.linear = 1.0f;
    pointLight.quadratic = 0.0f;

    glm::vec3 ambient(0,0,0);

    // SkyBox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);


    // draw in wireframe
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_CULL_FACE);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
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

        // FIRST LIGHT SOURCE -------------------------------------------------------=
        /*
        ourShader.setVec3("pointLight[0].position",programState->pointLightPositions[0]);
        ourShader.setVec3("pointLight[0].ambient", programState->ambient_plight);
        ourShader.setVec3("pointLight[0].diffuse", programState->diffuse_plight);
        ourShader.setVec3("pointLight[0].specular", programState->specular_plight);
        ourShader.setFloat("pointLight[0].constant", programState->constant_plight);
        ourShader.setFloat("pointLight[0].linear", programState->linear_plight);
        ourShader.setFloat("pointLight[0].quadratic", programState->quadratic_plight);
        ourShader.setVec3("viewPosition", programState->camera.Position);
        ourShader.setFloat("material.shininess", programState->shininess_plight);

        ourShader.setVec3("spotLight[0].position", programState->pointLightPositions[0]);
        ourShader.setVec3("spotLight[0].direction", programState->direction_slight);
        ourShader.setVec3("spotLight[0].ambient", programState->ambient_slight);
        ourShader.setVec3("spotLight[0].diffuse", programState->diffuse_slight);
        ourShader.setVec3("spotLight[0].specular", programState->specular_slight);
        ourShader.setFloat("spotLight[0].constant", programState->constant_slight);
        ourShader.setFloat("spotLight[0].linear", programState->linear_slight);
        ourShader.setFloat("spotLight[0].quadratic", programState->quadratic_slight);
        ourShader.setFloat("spotLight[0].cutOff", glm::cos(glm::radians(programState->cutOff_slight)));
        ourShader.setFloat("spotLight[0].outerCutOff", glm::cos(glm::radians(programState->outerCutOff_slight)));

        lightParams[0].position = glm::vec3(4.8f, 4.0f, 0.9f);
        lightParams[0].ambient_plight = glm::vec3(0.0f, 4.0f, 10.0f);
        lightParams[0].diffuse_plight = glm::vec3(0.5f, 0.0f, -2.5f);
        lightParams[0].specular_plight = glm::vec3(-1.0f, 5.0f, 16.0f);
        lightParams[0].constant_plight = 1.0f;
        lightParams[0].linear_plight = 1.0f;
        lightParams[0].quadratic_plight = 0.2f;
        lightParams[0].shininess_plight = 32.0f;

        lightParams[0].direction_slight = glm::vec3(0.0f, -1.0f, 0.0f);
        lightParams[0].ambient_slight = glm::vec3(0.0f, -4.0f, -1.0f);
        lightParams[0].diffuse_slight = glm::vec3(-1.0f, 0.0f, 1.0f);
        lightParams[0].specular_slight = glm::vec3(2.0f, 0.0f, 0.0f);
        lightParams[0].constant_slight = 1.0f;
        lightParams[0].linear_slight = 1.0f;
        lightParams[0].quadratic_slight = 0.35f;
        lightParams[0].cutOff_slight = 1.0f;
        lightParams[0].outerCutOff_slight = 50.0f;

         */

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
        ourShader.setVec3("pointLight[2].position",programState->pointLightPositions[2]);
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

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = programState->camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // Render the loaded models //

        glm::mat4 model = glm::mat4(1.0f);

        // make function?
        // drawModel(scaleVal=programState->grassScale, translationVal=programState->carPosition);

        // Grass model
        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(programState->grassScale));
        model = glm::rotate(model, glm::radians(90.f), glm::vec3(-1.0, 0.0, 0.0));
        ourShader.setMat4("model", model);
        grassModel.Draw(ourShader);

        // Car model
        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(programState->carScale));
        model = glm::translate(model, programState->carPosition);
        ourShader.setMat4("model", model);
        carModel.Draw(ourShader);

        // Lamp model
        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(programState->lampScale));
        model = glm::translate(model, programState->lampPosition);
        ourShader.setMat4("model", model);
        lampModel.Draw(ourShader);

        // Lamp2 model
        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(programState->lamp2Scale));
        model = glm::translate(model, programState->lamp2Position);
        ourShader.setMat4("model", model);
        lamp2Model.Draw(ourShader);

        // Cat model
        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(programState->catScale));
        model = glm::translate(model, programState->catPosition);
        ourShader.setMat4("model", model);
        catModel.Draw(ourShader);

        // Table model
        model = glm::mat4(1.0f);
        model = glm::translate(model, programState->tablePosition);
        model = glm::scale(model, glm::vec3(programState->tableScale));
        ourShader.setMat4("model", model);
        tableModel.Draw(ourShader);

        // Flower model
        model = glm::mat4(1.0f);
        model = glm::translate(model, programState->flowerPosition);
        model = glm::scale(model, glm::vec3(programState->flowerScale));
        ourShader.setMat4("model", model);
        flowerModel.Draw(ourShader);

        // Tree model
        model = glm::mat4(1.0f);
        model = glm::translate(model, programState->treePosition);
        model = glm::scale(model, glm::vec3(programState->treeScale));
        ourShader.setMat4("model", model);
        treeModel.Draw(ourShader);


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
        ImGui::Begin("Hello window");
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
