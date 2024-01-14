#define _CRT_SECURE_NO_WARNINGS
#define CRES 30
 
#include <iostream>
#include <fstream>
#include <sstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.hpp"
#include "model.hpp"

//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"

unsigned int compileShader(GLenum type, const char* source);
unsigned int createShader(const char* vsSource, const char* fsSource);
static unsigned loadImageToTexture(const char* filepath);
void createAndUpdateCircle(float radius, float centerX, float centerY, unsigned int VAO, unsigned int VBO);

int main(void)
{

    if (!glfwInit())
    {
        std::cout<<"GLFW Biblioteka se nije ucitala! :(\n";
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window;
    unsigned int wWidth = 1600;
    unsigned int wHeight = 800;
    unsigned int mWidth = 400;
    unsigned int mHeight = 400; //dimenzije 2d mape majevice
    const char wTitle[] = "[Olivera Milicic AI 5/2022]";
    window = glfwCreateWindow(wWidth, wHeight, wTitle, NULL, NULL);
    int showSmallMap = 1; //IZIGNORISEMO MALU 2D MAPU sa =0

    if (window == NULL)
    {
        std::cout << "Prozor nije napravljen! :(\n";
        glfwTerminate();
        return 2;
    }

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK)
    {
        std::cout << "GLEW nije mogao da se ucita! :'(\n";
        return 3;
    }
 
    //shaderi
    unsigned int texShader = createShader("tex.vert", "tex.frag");
    unsigned int letShader = createShader("let.vert", "let.frag");
    unsigned int grayrectShader = createShader("rect.vert", "rectgray.frag");
    unsigned int buttonShader = createShader("let.vert", "indikator.frag");
    unsigned int batShader = createShader("bat.vert", "bat.frag");
    unsigned int modelShader = createShader("object.vert", "object.frag");


    //**********************************************   UCITAVANJE  MODELA   ******************************************************
    Model teren("res/teren.obj");
    Model drone("res/dron.obj");


    Shader ModelShader("object.vert", "object.frag");
   

    ModelShader.use();
    ModelShader.setVec3("uLightPos", 0, 1, 3);
    ModelShader.setVec3("uViewPos", 0, 0, 5);
    ModelShader.setVec3("uLightColor", 1, 1, 1);

    //spekularna mapa --- PROVERITI DA LI RADI
    unsigned specularMap = loadImageToTexture("res/Majevica_specular.png");
    glActiveTexture(GL_TEXTURE1); // Use a different texture unit than the diffuse map
    glBindTexture(GL_TEXTURE_2D, specularMap); // Bind the specular map
    ModelShader.setInt("uSpecMap", 1); // Set the shader uniform


    //*******************************    UNIFORME    ********************************
    //perspektiva
    glm::mat4 projection = glm::perspective(glm::radians(90.0f), (float)wWidth / (float)wHeight, 0.1f, 100.0f);
    ModelShader.setMat4("uP", projection);


    //camera1
    glm::vec3 cameraPos1 = glm::vec3(-4.0f, -2.0f, 0.0f);
    glm::vec3 cameraFront1 = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp1 = glm::vec3(0.0f, 1.0f, 0.0f);

    //camera2
    glm::vec3 cameraPos2 = glm::vec3(4.0f, -2.0f, 0.0f);
    glm::vec3 cameraFront2 = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp2 = glm::vec3(0.0f, 1.0f, 0.0f);

    float cameraSpeed = 0.03f;
    float pitch = 0.0f; //Pitch is the angle that shows how much the camera is looking up or down
    float yaw1 = -90.0f; //Yaw is the angle that shows how much the camera is looking left or right
    // pocetni yaw je u pravcu pozitivne x ose, pa ako zelimo da gleda pravo treba -90
    float yaw2 = -90.0f;
    float sensitivity = 0.2f; // Sensitivity of the camera rotation

    //float deltaTime = 0.0f;	// Time between current frame and last frame
    //float lastFrame = 0.0f; // Time of last frame

    glm::mat4 view1 = glm::lookAt(cameraPos1, cameraPos1 + cameraFront1, cameraUp1);
    //glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 5.0f, 7.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    ModelShader.setMat4("uV", view1);

    glm::mat4 view2 = glm::lookAt(cameraPos2, cameraPos2 + cameraFront2, cameraUp2);
    //glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 5.0f, 7.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    ModelShader.setMat4("uV", view2);



    //teren
    
    //glm::mat4 view = glm::mat4(1.0f);
    //glm::mat4 projection = glm::mat4(1.0f);
    //view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
    //projection = glm::perspective(glm::radians(45.0f), (float)wWidth / 2 / (float)wHeight, 0.1f, 100.0f);

    glm::mat4 modelMapa = glm::mat4(1.0f);
    glm::vec3 newCoordinates = glm::vec3(0.0, -4.0, -30.0); //minus ispred Z da bi islo dalje od nas
    modelMapa = glm::translate(glm::mat4(1.0f), newCoordinates);
    modelMapa = glm::rotate(modelMapa, glm::radians(0.5f), glm::vec3(-10.0f, 0.0f, 0.0f));

    unsigned int modelLoc = glGetUniformLocation(modelShader, "uM");

    glUseProgram(modelShader); //Slanje default vrijednosti uniformi
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMapa));


    //dron1
    glm::mat4 modelDron1 = glm::mat4(1.0f); //Matrica transformacija - mat4(1.0f) generise jedinicnu matricu
    glm::vec3 newCoordinates1 = glm::vec3(-4.0, -3.0, 0.0); //pocetne koordinate levog drona
    modelDron1 = glm::translate(glm::mat4(1.0f), newCoordinates1);
    modelDron1 = glm::scale(modelDron1, glm::vec3(0.25f, 0.25f, 0.25f));

    glUseProgram(modelShader); //Slanje default vrijednosti uniformi
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelDron1)); //(Adresa matrice, broj matrica koje saljemo, da li treba da se transponuju, pokazivac do matrica)
    

    //dron2
    glm::mat4 modelDron2 = glm::mat4(1.0f);
    glm::vec3 newCoordinates2 = glm::vec3(4.0, -3.0, 0.0); 
    modelDron2 = glm::translate(glm::mat4(1.0f), newCoordinates2);
    modelDron2 = glm::scale(modelDron2, glm::vec3(0.25f, 0.25f, 0.25f));

    glUseProgram(modelShader);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelDron2));

   

    const int n = 11;

    unsigned int VAO[n];
    glGenVertexArrays(n, VAO);
    unsigned int VBO[n];
    glGenBuffers(n, VBO);

    //pravougaonik za mapu, za providnu teksturu
    float vertices[] =
    {  // x     y    z       s    t
        -1.0, -1.0, 0.0,     0.0, 0.0,     // dole levo
         1.0, -1.0, 0.0,     1.0, 0.0,     // dole desno
        -1.0,  1.0, 0.0,     0.0, 1.0,     // gore levo 
         1.0,  1.0, 0.0,     1.0, 1.0      // gore desno
    };

    unsigned int stride = 5 * sizeof(float);


    //pravougaonik providna tekstura
    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //pravougaonik tekstura mape
    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //NOISE 
    glBindVertexArray(VAO[10]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[10]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    

    //letelica1
    float circle1[CRES * 2 + 4]; // +4 je za x i y koordinate centra kruga, i za x i y od nultog ugla
    float r1 = 0.03; //poluprecnik
    float x1_center = -0.45;
    float y1_center = -0.7;
    createAndUpdateCircle(r1, x1_center, y1_center, VAO[2], VBO[2]);
        
    //letelica2
    float circle2[CRES * 2 + 4]; // +4 je za x i y koordinate centra kruga, i za x i y od nultog ugla
    float r2 = 0.03; //poluprecnik
    float x2_center = 0.45;
    float y2_center = -0.7;
    createAndUpdateCircle(r2, x2_center, y2_center, VAO[3], VBO[3]);


    //zabranjena zona
    float circle3[CRES * 2 + 4];
    float r3 = 0.2;
    float x_center = 0.45;
    float y_center = 0.4;
    createAndUpdateCircle(r3, x_center, y_center, VAO[5], VBO[5]);



    //pravougaonik -  baterija1, baterija 2, zeleno staklo
    float vertices1[] =
    {  // x     y     z         r     g      b    a
        -0.75, -0.93, 0.0,     0.0,  0.0,  0.0,  1.0,
        -0.75, -0.81, 0.0,     0.0,  0.0,  0.0,  1.0,
        -0.1, -0.93,  0.0,     0.0,  0.0,  0.0,  1.0,
        -0.1, -0.81,  0.0,     0.0,  0.0,  0.0,  1.0,

         0.75, -0.93, 0.0,     0.0,  0.0,  0.0,  1.0,
         0.75, -0.81, 0.0,     0.0,  0.0,  0.0,  1.0,
         0.1, -0.93,  0.0,     0.0,  0.0,  0.0,  1.0,
         0.1, -0.81,  0.0,     0.0,  0.0,  0.0,  1.0,

         -1.0, -1.0,  0.0,     0.0,  1.0,  0.0,  0.4,
          1.0, -1.0,  0.0,     0.0,  1.0,  0.0,  0.4,
         -1.0,  1.0,  0.0,     0.0,  1.0,  0.0,  0.4,
          1.0,  1.0,  0.0,     0.0,  1.0,  0.0,  0.4

    };

    //pravougaonik kontrl tabla
    //glBindVertexArray(VAO[1]);
    //glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1), vertices1, GL_STATIC_DRAW);
    //glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    //glEnableVertexAttribArray(0);
    //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
    //glEnableVertexAttribArray(1);
    //glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(5 * sizeof(float)));
    //glEnableVertexAttribArray(2);

    //baterija1
    glBindVertexArray(VAO[8]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[8]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1), vertices1, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    //baterija1
    glBindVertexArray(VAO[9]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[9]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1), vertices1, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    //pravougaonik zeleno staklo
    glBindVertexArray(VAO[4]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[4]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1), vertices1, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    



    float vertices2[] =
    {
        -0.88, -0.87,
         0.88,  -0.87
    };
    glBindVertexArray(VAO[6]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[6]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(VAO[7]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[7]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(0);



    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //tekstura mape
    unsigned checkerTexture1 = loadImageToTexture("res/Majevica.png"); 
    glBindTexture(GL_TEXTURE_2D, checkerTexture1);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    unsigned uTexLoc1 = glGetUniformLocation(texShader, "uTex1");
    glUniform1i(uTexLoc1, 0);

    //providna tekstura
    unsigned checkerTexture2 = loadImageToTexture("res/Providna.png");
    glBindTexture(GL_TEXTURE_2D, checkerTexture2);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    unsigned uTexLoc2 = glGetUniformLocation(texShader, "uTex2");
    glUniform1i(uTexLoc2, 1);

    //tekstura noise
    unsigned checkerTexture3 = loadImageToTexture("res/static_noise.png");
    glBindTexture(GL_TEXTURE_2D, checkerTexture3);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    unsigned uTexLoc3 = glGetUniformLocation(texShader, "uTex3");
    glUniform1i(uTexLoc2, 2);




    unsigned UseTextLoc = glGetUniformLocation(texShader, "usetexture");
    unsigned uPosLoc = glGetUniformLocation(letShader, "uPos");
    unsigned colorLocation = glGetUniformLocation(letShader, "uColor");
    unsigned switchLocation = glGetUniformLocation(buttonShader, "swOnOff");
    unsigned batPosLoc = glGetUniformLocation(batShader, "batPos");

    

    float dx1 = 0;
    float dy1 = 0;
    float dx2 = 0;
    float dy2 = 0;
    float rx1 = 0;
    float ry1 = 0;
    float rx2 = 0;
    float ry2 = 0;
    float directionAngle1 = 90; //90 stepeni - ide pravo odnosno ka gore
    float directionAngle2 = 90;
    float praznjenjeBat1 = 0;
    float praznjenjeBat2 = 0;

    int SwitchOnOff1 = 0;
    int SwitchOnOff2 = 0;
    int LetelicaDeaktivirana1 = 0;
    int LetelicaDeaktivirana2 = 0;
    int CameraActive1 = 0;
    int CameraActive2 = 0;
   
  
    while (!glfwWindowShouldClose(window))
    {
        /*cameraSpeed = 2.5f * deltaTime;
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;*/

        glClearColor(0.0, 0.0, 0.80, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST); //depth test
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //brisemo depth buffer ovde

        glEnable(GL_CULL_FACE); //culling
        glCullFace(GL_BACK);

        //*************************CEO EKRAN**********************************
        glViewport(0, 0, wWidth, wHeight);

        //koristimo ovaj shader za ucitavanje modela
        ModelShader.use();


        //pomeranje drona 1
        if (SwitchOnOff1 == 1) {
         
            //pomeranje drona levo desno, napred nazad
            if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) != GLFW_PRESS) {
                if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                {
                    modelDron1 = glm::translate(modelDron1, glm::vec3(-0.03, 0.0, 0.0)); //Pomeranje (Matrica transformacije, pomeraj po XYZ)
                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelDron1));
                    cameraPos1 -= glm::normalize(glm::cross(cameraFront1, cameraUp1)) * cameraSpeed;
                }
                if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                {
                    modelDron1 = glm::translate(modelDron1, glm::vec3(0.03, 0.0, 0.0));
                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelDron1));
                    cameraPos1 += glm::normalize(glm::cross(cameraFront1, cameraUp1)) * cameraSpeed;
                }
                if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                {
                    modelDron1 = glm::translate(modelDron1, glm::vec3(0.0, 0.0, -0.03));
                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelDron1));
                    cameraPos1 += cameraSpeed * cameraFront1;
                }
                if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                {
                    modelDron1 = glm::translate(modelDron1, glm::vec3(0.0, 0.0, 0.03));
                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelDron1));
                    cameraPos1 -= cameraSpeed * cameraFront1;
                }
            }
            else {
                //pomeranje gore dole
                if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                {
                    if (cameraPos1.y < 10.0f) { // ogranicenje kretanja ka gore
                        modelDron1 = glm::translate(modelDron1, glm::vec3(0.0, 0.03, 0.0));
                        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelDron1));
                        cameraPos1.y += cameraSpeed;
                    }
                }
                if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                {
                    modelDron1 = glm::translate(modelDron1, glm::vec3(0.0, -0.03, 0.0));
                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelDron1));
                    cameraPos1.y -= cameraSpeed;
                    if (cameraPos1.y < -4.0f) LetelicaDeaktivirana1 = 1;  //kad dodje do poda unisti se
                }
                //rotiranje levo desno
                if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                {
                    modelDron1 = glm::rotate(modelDron1, glm::radians(0.2f), glm::vec3(0.0f, 1.0f, 0.0f)); //Rotiranje (Matrica transformacije, ugao rotacije u radijanima, osa rotacije)
                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelDron1));
                    yaw1 -= sensitivity;
                    directionAngle1 += sensitivity;
                }
                if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                {
                    modelDron1 = glm::rotate(modelDron1, glm::radians(-0.2f), glm::vec3(0.0f, 1.0f, 0.0f));
                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelDron1));
                    yaw1 += sensitivity;
                    directionAngle1 -= sensitivity;
                }
            }
        }

        //paljenje/gasenje kamere1
        if (glfwGetKey(window, GLFW_KEY_1) && glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
        {
            CameraActive1 = 1;
        }
        else if (glfwGetKey(window, GLFW_KEY_1) && glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
        {
            CameraActive1 = 0;
        }

        //update-ovanje rotiranja kamere
        glm::vec3 direction1;
        direction1.x = cos(glm::radians(yaw1)) * cos(glm::radians(pitch));
        direction1.y = sin(glm::radians(pitch));
        direction1.z = sin(glm::radians(yaw1)) * cos(glm::radians(pitch));
        cameraFront1 = glm::normalize(direction1);


        //ucitavanje drona 2
        ModelShader.setMat4("uM", modelDron2);
        drone.Draw(ModelShader);

        if (SwitchOnOff2 == 1) {
            //pomeranje drona levo desno, napred nazad
            if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) != GLFW_PRESS) {
                if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
                {
                    modelDron2 = glm::translate(modelDron2, glm::vec3(-0.03, 0.0, 0.0)); //Pomeranje (Matrica transformacije, pomeraj po XYZ)
                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelDron2));
                    cameraPos2 -= glm::normalize(glm::cross(cameraFront2, cameraUp2)) * cameraSpeed;
                }
                if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
                {
                    modelDron2 = glm::translate(modelDron2, glm::vec3(0.03, 0.0, 0.0));
                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelDron2));
                    cameraPos2 += glm::normalize(glm::cross(cameraFront2, cameraUp2)) * cameraSpeed;
                }
                if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
                {
                    modelDron2 = glm::translate(modelDron2, glm::vec3(0.0, 0.0, -0.03));
                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelDron2));
                    cameraPos2 += cameraSpeed * cameraFront2;
                }
                if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
                {
                    modelDron2 = glm::translate(modelDron2, glm::vec3(0.0, 0.0, 0.03));
                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelDron2));
                    cameraPos2 -= cameraSpeed * cameraFront2;
                }
            }
            else {
                //pomeranje gore dole
                if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) && glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
                {
                    if (cameraPos2.y < 10.0f) { // ogranicenje kretanja ka gore
                        modelDron2 = glm::translate(modelDron2, glm::vec3(0.0, 0.03, 0.0));
                        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelDron2));
                        cameraPos2.y += cameraSpeed;
                    }
                }
                if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) && glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
                {
                    modelDron2 = glm::translate(modelDron2, glm::vec3(0.0, -0.03, 0.0));
                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelDron2));
                    cameraPos2.y -= cameraSpeed;
                    if (cameraPos2.y < -4.0f) LetelicaDeaktivirana2 = 1;  //kad dodje do poda unisti se
                }
                //rotiranje levo desno
                if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) && glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
                {
                    modelDron2 = glm::rotate(modelDron2, glm::radians(0.2f), glm::vec3(0.0f, 1.0f, 0.0f)); //Rotiranje (Matrica transformacije, ugao rotacije u radijanima, osa rotacije)
                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelDron2));
                    yaw2 -= sensitivity;
                    directionAngle2 += sensitivity;
                }
                if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) && glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
                {
                    modelDron2 = glm::rotate(modelDron2, glm::radians(-0.2f), glm::vec3(0.0f, 1.0f, 0.0f));
                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelDron2));
                    yaw2 += sensitivity;
                    directionAngle2 -= sensitivity;
                }
            }
        }

        //paljenje/gasenje kamere
        if (glfwGetKey(window, GLFW_KEY_2) && glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
        {
            CameraActive2 = 1;
        }
        else if (glfwGetKey(window, GLFW_KEY_2) && glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
        {
            CameraActive2 = 0;
        }

        //update-ovanje rotiranja kamere 2
        glm::vec3 direction2;
        direction2.x = cos(glm::radians(yaw2)) * cos(glm::radians(pitch));
        direction2.y = sin(glm::radians(pitch));
        direction2.z = sin(glm::radians(yaw2)) * cos(glm::radians(pitch));
        cameraFront2 = glm::normalize(direction2);


        //****************************   LEVI EKRAN KAMERA   *****************************
        glViewport(0, 0, wWidth / 2, wHeight);

        
        if (LetelicaDeaktivirana1 == 1 || CameraActive1 == 0) {
            glDisable(GL_DEPTH_TEST);
            //pravougaonik noise
            glUniform1i(UseTextLoc, 1);
            glUseProgram(texShader);
            glBindVertexArray(VAO[10]);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, checkerTexture3);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        else {
           
            glEnable(GL_DEPTH_TEST);
            ModelShader.use();
            glm::mat4 view1 = glm::lookAt(cameraPos1, cameraPos1 + cameraFront1, cameraUp1);
            ModelShader.setMat4("uV", view1);

            //iscrtavanje terena
            ModelShader.setMat4("uM", modelMapa);
            teren.Draw(ModelShader);

            ////iscrtavanje drona 1
            //ModelShader.setMat4("uM", modelDron1);
            //drone.Draw(ModelShader);

            //iscrtavanje drona 2
            if (LetelicaDeaktivirana2 == 0) {
                ModelShader.setMat4("uM", modelDron2);
                drone.Draw(ModelShader);
            }
        }
       

        //****************************   DESNI EKRAN KAMERA   *****************************
        glViewport(wWidth / 2, 0, wWidth / 2, wHeight);

        if (LetelicaDeaktivirana2 == 1 || CameraActive2 == 0) {
            glDisable(GL_DEPTH_TEST);
            //pravougaonik noise
            glUniform1i(UseTextLoc, 1);
            glUseProgram(texShader);
            glBindVertexArray(VAO[10]);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, checkerTexture3);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        else {
            glEnable(GL_DEPTH_TEST);

        
  
            ModelShader.use();
            glm::mat4 view2 = glm::lookAt(cameraPos2, cameraPos2 + cameraFront2, cameraUp2);
            ModelShader.setMat4("uV", view2);

            //iscrtavanje terena
            ModelShader.setMat4("uM", modelMapa);
            teren.Draw(ModelShader);

   
            //iscrtavanje drona 1
            if (LetelicaDeaktivirana1 == 0) {
                ModelShader.setMat4("uM", modelDron1);
                drone.Draw(ModelShader);
            }

            ////ucitavanje drona 2
            //ModelShader.setMat4("uM", modelDron2);
            //drone.Draw(ModelShader);

            
        }
        

        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);


        if (showSmallMap == 1) {
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            {
                glfwSetWindowShouldClose(window, GL_TRUE);
            }


            if (glfwGetKey(window, GLFW_KEY_1) && glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
            {
                SwitchOnOff1 = 1;
            }
            else if (glfwGetKey(window, GLFW_KEY_1) && glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
            {
                SwitchOnOff1 = 0;
            }


            if (glfwGetKey(window, GLFW_KEY_2) && glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
            {
                SwitchOnOff2 = 1;
            }
            else if (glfwGetKey(window, GLFW_KEY_2) && glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
            {
                SwitchOnOff2 = 0;
            }

            float step = 0.001; //pomeraj je 0.001

            if (SwitchOnOff1 == 1) {
                if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) != GLFW_PRESS) {  //ako nije pritusnut CNTRL onda izvrsavaj

                    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                    {
                        if (dy1 <= 1.8) {
                            //  dy1 += step; //pomeraj
                            dx1 = dx1 + step * cos(glm::radians(directionAngle1));
                            dy1 = dy1 + step * sin(glm::radians(directionAngle1));
                            //kretanje pod odredjenim uglom!!
                        }
                        else LetelicaDeaktivirana1 = 1;
                    }
                    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                    {
                        if (dy1 >= -0.3) {
                            //dy1 -= step; 
                            dx1 = dx1 + step * cos(glm::radians(directionAngle1 + 180));
                            dy1 = dy1 + step * sin(glm::radians(directionAngle1 + 180));
                        }
                        else LetelicaDeaktivirana1 = 1;
                    }
                    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                    {
                        if (dx1 <= 1.6) {
                           // dx1 += step;
                            dx1 = dx1 + step * cos(glm::radians(directionAngle1 - 90));
                            dy1 = dy1 + step * sin(glm::radians(directionAngle1 - 90));
                        }
                        else LetelicaDeaktivirana1 = 1;
                    }
                    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                    {
                        if (dx1 >= -0.6) {
                           // dx1 -= step;
                            dx1 = dx1 + step * cos(glm::radians(directionAngle1 + 90));
                            dy1 = dy1 + step * sin(glm::radians(directionAngle1 + 90));
                        }
                        else LetelicaDeaktivirana1 = 1;
                    }
                }
                //smanjivanje i povecavanje kruga na 2D mapi
                else {
                    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                    {
                        r1 += 0.0005;
                        createAndUpdateCircle(r1, x1_center, y1_center, VAO[2], VBO[2]);

                    }
                    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                    {
                        r1 -= 0.0005;
                        createAndUpdateCircle(r1, x1_center, y1_center, VAO[2], VBO[2]);

                    }
                }
            }


            if (SwitchOnOff2 == 1) {
                if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) != GLFW_PRESS) {
                    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
                    {
                        if (dy2 <= 1.8)
                        {   //dy2 += 0.001;
                            dx2 = dx2 + step * cos(glm::radians(directionAngle2));
                            dy2 = dy2 + step * sin(glm::radians(directionAngle2));
                        }else LetelicaDeaktivirana2 = 1;
                    }
                    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
                    {
                        if (dy2 >= -0.3) {
                            //dy2 -= 0.001;
                            dx2 = dx2 + step * cos(glm::radians(directionAngle2 + 180));
                            dy2 = dy2 + step * sin(glm::radians(directionAngle2 + 180));
                        }
                        else LetelicaDeaktivirana2 = 1;
                    }
                    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
                    {
                        if (dx2 <= 0.6) {
                            // dx2 += 0.001;
                            dx2 = dx2 + step * cos(glm::radians(directionAngle2 - 90));
                            dy2 = dy2 + step * sin(glm::radians(directionAngle2 - 90));
                        }
                        else LetelicaDeaktivirana2 = 1;
                    }
                    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
                    {
                        if (dx2 >= -1.6) {
                            // dx2 -= 0.001;
                            dx2 = dx2 + step * cos(glm::radians(directionAngle2 + 90));
                            dy2 = dy2 + step * sin(glm::radians(directionAngle2 + 90));
                        }
                        else LetelicaDeaktivirana2 = 1;
                    }
                }
                    //smanjivanje i povecavanje kruga na 2D mapi
                    else {
                        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
                        {
                            r2 += 0.0005;
                            createAndUpdateCircle(r2, x2_center, y2_center, VAO[3], VBO[3]);

                        }
                        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
                        {
                            r2 -= 0.0005;
                            createAndUpdateCircle(r2, x2_center, y2_center, VAO[3], VBO[3]);

                        }
                    }
            }

            //praznjenje baterije dron 1
            if (SwitchOnOff1 == 1) praznjenjeBat1 += 0.01;  //ukoliko je dron ulkjucen trosi se baterija
            if (CameraActive1 == 1)  praznjenjeBat1 += 0.01 * 0.2;  //ako je kamera ukljucena trosi se 20% vise baterije
            if (praznjenjeBat1 > 250) LetelicaDeaktivirana1 = 1;  //kad se baterija isprazni dron se ugasi

            //praznjenje baterije dron 2
            if (SwitchOnOff2 == 1) praznjenjeBat2 += 0.01;  //ukoliko je dron ulkjucen trosi se baterija
            if (CameraActive2 == 1)  praznjenjeBat2 += 0.01 * 0.2;  //ako je kamera ukljucena trosi se 20% vise baterije
            if (praznjenjeBat2 > 250) LetelicaDeaktivirana2 = 1;  //kad se baterija isprazni dron se ugasi

            //sudaranje letelica
            double d = sqrt(((x1_center + dx1) - (x2_center + dx2)) * ((x1_center + dx1) - (x2_center + dx2)) + ((y1_center + dy1) - (y2_center + dy2)) * ((y1_center + dy1) - (y2_center + dy2)));
            if (d < r1 + r2) {
                LetelicaDeaktivirana1 = 1;
                LetelicaDeaktivirana2 = 1;
            }

            //ulazak u zabranjenu zonu
            double d1 = sqrt(((x1_center + dx1) - x_center) * ((x1_center + dx1) - x_center) + ((y1_center + dy1) - y_center) * ((y1_center + dy1) - y_center));
            if (d1 < r1 + r3) {
                LetelicaDeaktivirana1 = 1;
            }

            double d2 = sqrt(((x2_center + dx2) - x_center) * ((x2_center + dx2) - x_center) + ((y2_center + dy2) - y_center) * ((y2_center + dy2) - y_center));
            if (d2 < r2 + r3) {
                LetelicaDeaktivirana2 = 1;
            }



            //glClearColor(0.50, 0.0, 0.50, 1.0);


            //pravougaonik tekstura mape
            glViewport(wWidth / 2 - mWidth / 2, 0, mWidth, mHeight);//iscrtavanje prozora na sredini ekrana, smanjen
            glUniform1i(UseTextLoc, 0);
            glUseProgram(texShader);
            glBindVertexArray(VAO[1]);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, checkerTexture1);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glBindTexture(GL_TEXTURE_2D, 0);

            //letelica1
            if (LetelicaDeaktivirana1 == 0) {
                glUseProgram(letShader);
                glUniform4f(colorLocation, 0.36, 0.25, 0.83, 1.0);

               glUniform2f(uPosLoc, dx1, dy1);

                glBindVertexArray(VAO[2]);
                glDrawArrays(GL_TRIANGLE_FAN, 0, sizeof(circle1) / (2 * sizeof(float)));
            }

            //letelice2
            if (LetelicaDeaktivirana2 == 0) {
                glUseProgram(letShader);
                glUniform4f(colorLocation, 0.0, 0.6, 1.0, 1.0);
                glUniform2f(uPosLoc, dx2, dy2);
                glBindVertexArray(VAO[3]);
                glDrawArrays(GL_TRIANGLE_FAN, 0, sizeof(circle2) / (2 * sizeof(float)));
            }

            //zabranjena zona
            glUseProgram(letShader);
            glUniform4f(colorLocation, 1.0, 0.0, 0.0, 0.5);
            glUniform2f(uPosLoc, 0, 0);
            glBindVertexArray(VAO[5]);
            glDrawArrays(GL_TRIANGLE_FAN, 0, sizeof(circle3) / (2 * sizeof(float)));

            //pravougaonik kontrl tbl
            //glUseProgram(grayrectShader);
            //glBindVertexArray(VAO[1]);
            //glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


            //pravougaonik zeleno staklo
            glUseProgram(grayrectShader);
            glBindVertexArray(VAO[4]);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDrawArrays(GL_TRIANGLE_STRIP, 8, 4);


            glViewport(0, 0, wWidth / 2, wHeight);   //iscrtavanje prozora na levoj strani ekrana - kamera levog drona


            //pravougaonik providna tekstura
            glUniform1i(UseTextLoc, 1);
            glUseProgram(texShader);
            glBindVertexArray(VAO[0]);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, checkerTexture2);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glBindTexture(GL_TEXTURE_2D, 0);

            //baterija1
            if (LetelicaDeaktivirana1 == 0) {
                glUseProgram(batShader);
                glUniform1f(batPosLoc, 360 - praznjenjeBat1);
                glBindVertexArray(VAO[8]);
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            }
            //dugme on/off 1
            glUseProgram(buttonShader);
            glUniform2f(uPosLoc, 0, 0);
            if (LetelicaDeaktivirana1 == 1) {
                SwitchOnOff1 = 0;
            }
            glUniform1i(switchLocation, SwitchOnOff1);
            glBindVertexArray(VAO[6]);
            glEnable(GL_PROGRAM_POINT_SIZE);
            glPointSize(35);
            glDrawArrays(GL_POINTS, 0, 1);


            glViewport(wWidth / 2, 0, wWidth / 2, wHeight);   //iscrtavanje prozora na desnoj strani ekrana - kamera desnog drona

            //baterija2
            if (LetelicaDeaktivirana2 == 0) {
                glUseProgram(batShader);
                glUniform1f(batPosLoc, 1500 - praznjenjeBat2);
                glBindVertexArray(VAO[9]);
                glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);
            }
            //dugme on/off 2
            glUseProgram(buttonShader);
            glUniform2f(uPosLoc, 0, 0);
            if (LetelicaDeaktivirana2 == 1) {
                SwitchOnOff2 = 0;
            }
            glUniform1i(switchLocation, SwitchOnOff2);
            glBindVertexArray(VAO[7]);
            glEnable(GL_PROGRAM_POINT_SIZE);
            glPointSize(35);
            glDrawArrays(GL_POINTS, 0, 1);

        }

       

        glfwSwapBuffers(window);
        glfwPollEvents(); 
    }

    glDeleteTextures(1, &checkerTexture1);
    glDeleteTextures(1, &checkerTexture2);
    glDeleteTextures(1, &checkerTexture3);
    glDeleteBuffers(n, VBO);
    glDeleteVertexArrays(n, VAO);
    glDeleteProgram(texShader);
    glDeleteProgram(letShader);
    glDeleteProgram(grayrectShader);


    glfwTerminate();
    return 0;
}


 void createAndUpdateCircle(float radius, float centerX, float centerY, unsigned int VAO, unsigned int VBO) {
     float circle[CRES * 2 + 4]; // Array to hold circle vertices

     // Set center coordinates of the circle
     circle[0] = centerX; // Center X
     circle[1] = centerY; // Center Y

     // Calculate circle vertices
     for (int i = 0; i <= CRES; i++) {
         circle[2 + 2 * i] = centerX + radius * cos((3.141592 / 180) * (i * 360 / CRES));
         circle[2 + 2 * i + 1] = centerY + radius * sin((3.141592 / 180) * (i * 360 / CRES));
     }

     // Bind and update VAO and VBO
     glBindVertexArray(VAO);
     glBindBuffer(GL_ARRAY_BUFFER, VBO);
     glBufferData(GL_ARRAY_BUFFER, sizeof(circle), circle, GL_STATIC_DRAW);
     glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
     glEnableVertexAttribArray(0);
 }

unsigned int compileShader(GLenum type, const char* source)
{
    std::string content = "";
    std::ifstream file(source);
    std::stringstream ss;
    if (file.is_open())
    {
        ss << file.rdbuf();
        file.close();
        std::cout << "Uspjesno procitao fajl sa putanje \"" << source << "\"!" << std::endl;
    }
    else {
        ss << "";
        std::cout << "Greska pri citanju fajla sa putanje \"" << source << "\"!" << std::endl;
    }
     std::string temp = ss.str();
     const char* sourceCode = temp.c_str();

    int shader = glCreateShader(type);
    
    int success;
    char infoLog[512];
    glShaderSource(shader, 1, &sourceCode, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        if (type == GL_VERTEX_SHADER)
            printf("VERTEX");
        else if (type == GL_FRAGMENT_SHADER)
            printf("FRAGMENT");
        printf(" sejder ima gresku! Greska: \n");
        printf(infoLog);
    }
    return shader;
}
unsigned int createShader(const char* vsSource, const char* fsSource)
{

    unsigned int program; 
    unsigned int vertexShader; 
    unsigned int fragmentShader; 

    program = glCreateProgram(); 

    vertexShader = compileShader(GL_VERTEX_SHADER, vsSource); 
    fragmentShader = compileShader(GL_FRAGMENT_SHADER, fsSource); 

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program); 
    glValidateProgram(program); 

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_VALIDATE_STATUS, &success); 
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(program, 512, NULL, infoLog);
        std::cout << "Objedinjeni sejder ima gresku! Greska: \n";
        std::cout << infoLog << std::endl;
    }

    glDetachShader(program, vertexShader);
    glDeleteShader(vertexShader);
    glDetachShader(program, fragmentShader);
    glDeleteShader(fragmentShader);

    return program;
}

static unsigned loadImageToTexture(const char* filePath) {
    int TextureWidth;
    int TextureHeight;
    int TextureChannels;
    unsigned char* ImageData = stbi_load(filePath, &TextureWidth, &TextureHeight, &TextureChannels, 0);
    if (ImageData != NULL)
    {
        stbi__vertical_flip(ImageData, TextureWidth, TextureHeight, TextureChannels);

        GLint InternalFormat = -1;
        switch (TextureChannels) {
        case 1: InternalFormat = GL_RED; break;
        case 3: InternalFormat = GL_RGB; break;
        case 4: InternalFormat = GL_RGBA; break;
        default: InternalFormat = GL_RGB; break;
        }

        unsigned int Texture;
        glGenTextures(1, &Texture);
        glBindTexture(GL_TEXTURE_2D, Texture);
        glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, TextureWidth, TextureHeight, 0, InternalFormat, GL_UNSIGNED_BYTE, ImageData);
        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(ImageData);
        return Texture;
    }
    else
    {
        std::cout << "Textura nije ucitana! Putanja texture: " << filePath << std::endl;
        stbi_image_free(ImageData);
        return 0;
    }
}
