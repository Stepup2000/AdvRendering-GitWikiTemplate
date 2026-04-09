#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>

#include "core/mesh.h"
#include "core/assimpLoader.h"
#include "core/texture.h"
#include "core/FullscreenQuad.h"
#include "core/TextQuad.h"

#define VSTUDIO
#ifdef VSTUDIO
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#endif

// ------------------ Window + Camera ------------------
int g_width = 800, g_height = 600;
glm::vec3 cameraPos(0.0f,0.0f,10.0f);
glm::vec3 cameraFront(0.0f,0.0f,-1.0f);
glm::vec3 cameraUp(0.0f,1.0f,0.0f);
float cameraSpeedBase = 5.0f;
float yaw=-90.0f, pitch = 0.0f;
float lastX = g_width/2.0f, lastY = g_height/2.0f;
bool firstMouse = true, rightMouseHeld = false;

// ------------------ Scene ------------------
enum class SceneId { Scene1, Scene2 };
SceneId currentScene = SceneId::Scene1;
bool spacePressedLastFrame = false;

// ------------------ Post-processing ------------------
bool enableBloom = false;
bool enableSobel = false;
bool enableGrayscale = false;
bool enableInvert = false;

float bloomThreshold = 0.8f;
float bloomIntensity = 1.5f;

int sobelKernelSize = 7; // 3, 5, 7

// ------------------ FPS ---------------------------
bool recordDeltaTime = false;
float displayedFPS = 0;
std::vector<float> deltaTimeBuffer;
const int maxFrames = 100;

// ------------------ Framebuffers ------------------
GLuint sceneFBO, sceneTex, sceneRBO;
GLuint brightFBO, brightTex;
GLuint pingpongFBO[2], pingpongTex[2];

// ------------------ Shader helpers ------------------
std::string readFileToString(const std::string &path){
    std::ifstream file(path); std::stringstream buffer; buffer << file.rdbuf(); return buffer.str();
}

GLuint generateShader(const std::string &src, GLuint type){
    //Create object
    GLuint s = glCreateShader(type);

    //Set source
    const char* c = src.c_str();
    glShaderSource(s, 1, &c, nullptr);
    glCompileShader(s);

    //Debug when done
    GLint success; glGetShaderiv(s, GL_COMPILE_STATUS, &success);
    if(!success){
        char log[512]; glGetShaderInfoLog(s, 512, nullptr, log);
        std::cout << "Shader compilation error:\n" << log << std::endl;
    }
    return s;
}

GLuint createShaderProgram(const char* vsSrc, const char* fsSrc){
    GLuint vs = generateShader(vsSrc, GL_VERTEX_SHADER);
    GLuint fs = generateShader(fsSrc, GL_FRAGMENT_SHADER);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs); glAttachShader(prog, fs);
    glLinkProgram(prog);
    glDeleteShader(vs); glDeleteShader(fs);
    return prog;
}

// ------------------ Input ------------------
void processInput(GLFWwindow *window, float deltaTime){
    float speed = cameraSpeedBase * deltaTime;
    if(glfwGetKey(window,GLFW_KEY_W)==GLFW_PRESS) cameraPos += speed*cameraFront;
    if(glfwGetKey(window,GLFW_KEY_S)==GLFW_PRESS) cameraPos -= speed*cameraFront;
    if(glfwGetKey(window,GLFW_KEY_A)==GLFW_PRESS) cameraPos -= glm::normalize(glm::cross(cameraFront,cameraUp))*speed;
    if(glfwGetKey(window,GLFW_KEY_D)==GLFW_PRESS) cameraPos += glm::normalize(glm::cross(cameraFront,cameraUp))*speed;
    if(glfwGetKey(window,GLFW_KEY_ESCAPE)==GLFW_PRESS) glfwSetWindowShouldClose(window,true);
}

// ------------------ Callbacks ------------------
void framebufferSizeCallback(GLFWwindow*, int width, int height){
    g_width = width; g_height = height;
    glViewport(0,0,width,height);
}

void mouseCallback(GLFWwindow*, double xpos, double ypos){
    //Check if something happend
    if(!rightMouseHeld) return;
    if(firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    //Calculate offset and clamp to prevent flip
    float xoffset = xpos-lastX, yoffset = lastY-ypos;
    lastX = xpos; lastY = ypos;
    float sensitivity = 0.1f; xoffset *= sensitivity; yoffset *= sensitivity;
    yaw += xoffset; pitch += yoffset;
    pitch = glm::clamp(pitch,-89.0f,89.0f);
    glm::vec3 dir;

    //Calculate new dir
    dir.x = cos(glm::radians(yaw))*cos(glm::radians(pitch));
    dir.y = sin(glm::radians(pitch));
    dir.z = sin(glm::radians(yaw))*cos(glm::radians(pitch));
    cameraFront = glm::normalize(dir);
}

// ------------------ Framebuffer creation ------------------
void createFramebuffer(GLuint &FBO, GLuint &colorTex, GLuint &RBO, int width, int height){
    // Generate and bind
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    // Create color texture and attach to FBO
    glGenTextures(1, &colorTex);
    glBindTexture(GL_TEXTURE_2D, colorTex);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB16F,width,height,0,GL_RGB,GL_FLOAT,nullptr);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex,0);

    // Create depth-stencil renderbuffer and attach to FBO
    glGenRenderbuffers(1,&RBO);
    glBindRenderbuffer(GL_RENDERBUFFER,RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width,height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,RBO);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER,0);
}

void writeCSV(const std::vector<float>& frameTimes)
{
    const std::string filename = "frame_times.csv";
    std::ofstream file(filename);

    if (!file) {
        std::cerr << "Error: Could not open " << filename << " for writing.\n";
        return;
    }

    // Write header
    file << "Frame;DeltaTime;FPS\n";

    for (size_t frame = 0; frame < frameTimes.size(); ++frame)
    {
        const float deltaTime = frameTimes[frame];
        const float fps = (deltaTime > 0.0f) ? 1.0f / deltaTime : 0.0f;

        file << frame << ";"
             << deltaTime << ";"
             << fps << "\n";
    }

    std::cout << "Saved " << frameTimes.size() << " frames to " << filename << "\n";
}

// ------------------ Main ------------------
int main(){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(g_width,g_height,"500330 StephanGibson render engine",nullptr,nullptr);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    FullscreenQuad quad;

    // ------------------ ImGui ------------------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window,true);
    ImGui_ImplOpenGL3_Init("#version 400");

    glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    // ------------------ Load models ------------------
    core::Model suzanne = core::AssimpLoader::loadModel("models/nonormalmonkey.obj");
    core::Model cube = core::AssimpLoader::loadModel("models/Cube.obj");
    core::Texture cubeTexture("textures/Checker.png");
    core::Texture quadTexture("textures/Checker.png"); // quad in world
    core::Texture leftTex("textures/Monkey_1.png");
    core::Texture rightTex("textures/Monkey_2.png");

    std::vector<core::Model> scene1, scene2;

    core::Model left= suzanne; left.translate(glm::vec3(-1.5f,0,0));
    core::Model right= suzanne; right.translate(glm::vec3(1.5f,0,0));

    left.setTexture(leftTex.getId());
    right.setTexture(rightTex.getId());
    cube.setTexture(cubeTexture.getId());

    scene1.push_back(cube);
    scene2.push_back(left); scene2.push_back(right);

    TextQuad textQuad(quadTexture.getId());
    textQuad.setPosition(glm::vec3(0.0f, 1.0f, -5.0f));
    textQuad.setScale(glm::vec3(2.0f, 1.0f, 1.0f));


    // ------------------ ADS Shader ------------------
    std::string adsVS = readFileToString("shaders/ads.vs");
    std::string adsFS = readFileToString("shaders/ads.fs");
    GLuint shader = createShaderProgram(adsVS.c_str(), adsFS.c_str());

    GLint modelUniform = glGetUniformLocation(shader,"model");
    GLint viewUniform = glGetUniformLocation(shader,"view");
    GLint projUniform = glGetUniformLocation(shader,"projection");
    GLint lightPosUniform = glGetUniformLocation(shader,"lightPos");
    GLint viewPosUniform = glGetUniformLocation(shader,"viewPos");
    GLint lightColorUniform = glGetUniformLocation(shader,"lightColor");
    GLint constantUniform = glGetUniformLocation(shader,"constant");
    GLint linearUniform = glGetUniformLocation(shader,"linear");
    GLint quadraticUniform = glGetUniformLocation(shader,"quadratic");
    GLint diffuseMapUniform = glGetUniformLocation(shader,"diffuseMap");
    GLint useTextureUniform = glGetUniformLocation(shader,"useTexture");

    // ------------------ Post-processing Shaders ------------------
    std::string quadVS = readFileToString("shaders/quad.vs");
    std::string brightFS = readFileToString("shaders/bright.fs");
    std::string gaussianFS = readFileToString("shaders/gaussian.fs");
    std::string combineFS = readFileToString("shaders/combine.fs");
    std::string sobelFS = readFileToString("shaders/sobel.fs");
    std::string grayscaleFS = readFileToString("shaders/grayscale.fs");
    std::string invertFS = readFileToString("shaders/invert.fs");

    GLuint brightShader = createShaderProgram(quadVS.c_str(), brightFS.c_str());
    GLuint gaussianShader = createShaderProgram(quadVS.c_str(), gaussianFS.c_str());
    GLuint combineShader = createShaderProgram(quadVS.c_str(), combineFS.c_str());
    GLuint sobelShader = createShaderProgram(quadVS.c_str(), sobelFS.c_str());
    GLuint grayscaleShader = createShaderProgram(quadVS.c_str(), grayscaleFS.c_str());
    GLuint invertShader = createShaderProgram(quadVS.c_str(), invertFS.c_str());

    // ------------------ Framebuffers ------------------
    createFramebuffer(sceneFBO, sceneTex, sceneRBO, g_width, g_height);

    // Create bright-pass framebuffer
    glGenFramebuffers(1,&brightFBO);
    glBindFramebuffer(GL_FRAMEBUFFER,brightFBO);
    glGenTextures(1,&brightTex);
    glBindTexture(GL_TEXTURE_2D,brightTex);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB16F,g_width,g_height,0,GL_RGB,GL_FLOAT,nullptr);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,brightTex,0);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE) std::cout<<"Bright FBO incomplete\n";
    glBindFramebuffer(GL_FRAMEBUFFER,0);

    // Create ping pong framebuffer
    glGenFramebuffers(2,pingpongFBO);
    glGenTextures(2,pingpongTex);
    for(int i = 0; i<2; i++){
        glBindFramebuffer(GL_FRAMEBUFFER,pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D,pingpongTex[i]);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGB16F,g_width,g_height,0,GL_RGB,GL_FLOAT,nullptr);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,pingpongTex[i],0);
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE)
            std::cout<<"Pingpong FBO "<<i<<" incomplete\n";
    }
    glBindFramebuffer(GL_FRAMEBUFFER,0);

    // Set up perspective projection matrix
    glm::mat4 projection = glm::perspective(glm::radians(45.0f),(float)g_width/(float)g_height,0.1f,100.0f);

    //Initialize time and stats
    double lastTime = glfwGetTime(); float deltaTime = 0.0f; float rotationSpeed = 100.0f;
    float fps = 0.0f, fpsAccumulator = 0.0f; int fpsFrames = 0; double fpsTimer = 0.0;

    // ------------------ Main Loop ------------------
    while(!glfwWindowShouldClose(window)){
        double currentTime = glfwGetTime();
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        if (deltaTime > 0.0f)
            fps = 1.0f / deltaTime;

        // Accumulate time and frames
        fpsAccumulator += deltaTime;
        fpsFrames++;

        // Update FPS once per second
        if (fpsAccumulator >= 1.0f) {
            fps = fpsFrames / fpsAccumulator;  // average FPS over ~1 second
            fpsAccumulator = 0.0f;
            fpsFrames = 0;
            displayedFPS = fps;
        }

        // ------------------ FrameTime Recording ------------------
        if(recordDeltaTime) {
            deltaTimeBuffer.push_back(deltaTime);

            if(deltaTimeBuffer.size() >= maxFrames) {
                recordDeltaTime = false;
                writeCSV(deltaTimeBuffer);
            }
        }

        //Mouse input
        if(glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_RIGHT)==GLFW_PRESS){
            rightMouseHeld = true;
            glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_DISABLED);
        } else {
            rightMouseHeld = false;
            firstMouse = true;
            glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_NORMAL);
        }

        //Scene switching
        bool spaceNow = glfwGetKey(window,GLFW_KEY_SPACE)==GLFW_PRESS;
        if(spaceNow && !spacePressedLastFrame) currentScene = (currentScene==SceneId::Scene1)? SceneId::Scene2 : SceneId::Scene1;
        spacePressedLastFrame=spaceNow;
        processInput(window, deltaTime);

        //Camera matrix
        glm::mat4 view = glm::lookAt(cameraPos,cameraPos+cameraFront,cameraUp);

        // --------- Render scene to sceneFBO ---------
        glBindFramebuffer(GL_FRAMEBUFFER,sceneFBO);
        glClearColor(0.0f,0.0f,0.0f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //Rotate scene 1
        if(currentScene == SceneId::Scene1) {
            scene1[0].rotate(glm::vec3(0,1,0), glm::radians(rotationSpeed)*deltaTime);
        }

        //Set lightning uniforms
        glUseProgram(shader);
        glUniform3f(lightPosUniform,0.0f,5.0f,5.0f);
        glUniform3fv(viewPosUniform,1,glm::value_ptr(cameraPos));
        glUniform3f(lightColorUniform,3.0f,3.0f,3.0f);
        glUniform1f(constantUniform,1.0f); glUniform1f(linearUniform,0.09f); glUniform1f(quadraticUniform,0.032f);

        //Render models
        const std::vector<core::Model>* activeScene = (currentScene == SceneId::Scene1) ? &scene1 : &scene2;

        for (const core::Model &m : *activeScene) {
            glm::mat4 modelM = m.getModelMatrix();
            glUniformMatrix4fv(modelUniform, 1, GL_FALSE, glm::value_ptr(modelM));
            glUniformMatrix4fv(viewUniform, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(projUniform, 1, GL_FALSE, glm::value_ptr(projection));

            // Bind texture if it has one
            if (m.hasTexture) {
                glUniform1i(useTextureUniform, 1);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, m.textureId);
                glUniform1i(diffuseMapUniform, 0);
            } else {
                glUniform1i(useTextureUniform, 0);
            }

            m.render();
        }

        // --------- Render world-space quad with texture -----------------
        glUseProgram(shader); // ensure the shader is active
        glUniformMatrix4fv(viewUniform, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projUniform, 1, GL_FALSE, glm::value_ptr(projection));
        glUniform1i(useTextureUniform, 1);
        textQuad.render(shader);


        // --------- Bright-pass ---------
        glBindFramebuffer(GL_FRAMEBUFFER, brightFBO);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(brightShader);
        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D,sceneTex);
        glUniform1i(glGetUniformLocation(brightShader,"sceneTex"),0);
        glUniform1f(glGetUniformLocation(brightShader,"threshold"), bloomThreshold);
        quad.render();
        glBindFramebuffer(GL_FRAMEBUFFER,0);

        // --------- Gaussian blur ---------
        bool horizontal = true, first_pass = true; int blurPasses=10;
        glUseProgram(gaussianShader);
        for(int i = 0; i < blurPasses; i++){
            glBindFramebuffer(GL_FRAMEBUFFER,pingpongFBO[horizontal]);
            glUniform1i(glGetUniformLocation(gaussianShader,"horizontal"),horizontal);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D,first_pass?brightTex:pingpongTex[!horizontal]);
            quad.render();
            horizontal = !horizontal;
            if(first_pass) first_pass = false;
        }
        glBindFramebuffer(GL_FRAMEBUFFER,0);

        // --------- Final combine ---------
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);
        if(enableBloom){
            glUseProgram(combineShader);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, sceneTex);
            glUniform1i(glGetUniformLocation(combineShader,"scene"),0);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, pingpongTex[!horizontal]);
            glUniform1i(glGetUniformLocation(combineShader,"bloomBlur"),1);

            glUniform1f(glGetUniformLocation(combineShader,"intensity"), bloomIntensity);

            quad.render();
        }
        else if(enableSobel){
            glUseProgram(sobelShader);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, sceneTex);
            glUniform1i(glGetUniformLocation(sobelShader,"sceneTex"),0);

            glUniform1i(glGetUniformLocation(sobelShader,"kernelSize"), sobelKernelSize);

            quad.render();
        }
        else if(enableGrayscale){
            glUseProgram(grayscaleShader);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, sceneTex);
            glUniform1i(glGetUniformLocation(grayscaleShader,"sceneTex"),0);

            quad.render();
        }
        else if(enableInvert){
            glUseProgram(invertShader);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, sceneTex);
            glUniform1i(glGetUniformLocation(invertShader,"sceneTex"),0);

            quad.render();
        }
        else{
            glUseProgram(combineShader);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, sceneTex);
            glUniform1i(glGetUniformLocation(combineShader,"scene"),0);

            glUniform1f(glGetUniformLocation(combineShader,"intensity"),0.0f);

            quad.render();
        }
        glEnable(GL_DEPTH_TEST);

        // --------- ImGui ---------
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("Scene");
        ImGui::Text("SPACE to switch scenes");
        ImGui::Text("Current: %s", currentScene==SceneId::Scene1?"Scene 1":"Scene 2");
        ImGui::Text("FPS: %.1f", displayedFPS);

        const char* kernelOptions[] = { "3x3", "5x5", "7x7" };
        int kernelIndex = (sobelKernelSize == 3 ? 0 : sobelKernelSize == 5 ? 1 : 2);

        if(ImGui::Combo("Kernel Size", &kernelIndex, kernelOptions, 3)){
            sobelKernelSize = (kernelIndex == 0 ? 3 : kernelIndex == 1 ? 5 : 7);
        }
        ImGui::Checkbox("Sobel",&enableSobel);

        // Record deltaTime button
        if(ImGui::Button("Record 100 Frames")) {
            recordDeltaTime = true;
            deltaTimeBuffer.clear();
        }

        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ------------------ Cleanup ------------------
    glDeleteProgram(shader);
    glDeleteProgram(brightShader);
    glDeleteProgram(gaussianShader);
    glDeleteProgram(combineShader);
    glDeleteProgram(sobelShader);
    glDeleteProgram(grayscaleShader);
    glDeleteProgram(invertShader);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}