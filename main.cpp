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
static unsigned loadImageToTexture(const char* Majevica);
static unsigned loadImageToTexture(const char* Providna);



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
    int showSmallMap = 1; //IZIGNORISEMO MALU 2D MAPU

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
 
    //**********************************************UCITAVANJE*MODELA******************************************************
    Model drone("res/dron.obj");

    Shader DroneShader("drone.vert", "drone.frag");

    DroneShader.use();
    DroneShader.setVec3("uLightPos", 0, 1, 3);
    DroneShader.setVec3("uViewPos", 0, 0, 5);
    DroneShader.setVec3("uLightColor", 1, 1, 1);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)wWidth / (float)wHeight, 0.1f, 100.0f);
    DroneShader.setMat4("uP", projection);
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    DroneShader.setMat4("uV", view);
    glm::mat4 model = glm::mat4(1.0f);


    //shaderi
    unsigned int texShader = createShader("tex.vert", "tex.frag");
    unsigned int letShader = createShader("let.vert", "let.frag");
    unsigned int grayrectShader = createShader("rect.vert", "rectgray.frag");
    unsigned int buttonShader = createShader("let.vert", "indikator.frag");
    unsigned int batShader = createShader("bat.vert", "bat.frag");


    const int n = 10;

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

    

    //letelica1
    float circle1[CRES * 2 + 4]; // +4 je za x i y koordinate centra kruga, i za x i y od nultog ugla
    float r1 = 0.03; //poluprecnik

    float x1_center = -0.45;
    float y1_center = -0.7;


    circle1[0] = x1_center; //Centar X0
    circle1[1] = y1_center; //Centar Y0
    int i;
    for (i = 0; i <= CRES; i++)
    {
        circle1[2 + 2 * i] = x1_center + r1 * cos((3.141592 / 180) * (i * 360 / CRES)); //Xi
        circle1[2 + 2 * i + 1] = y1_center + r1 * sin((3.141592 / 180) * (i * 360 / CRES)); //Yi
    }

    glBindVertexArray(VAO[2]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(circle1), circle1, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


    //letelica2
    float circle2[CRES * 2 + 4]; // +4 je za x i y koordinate centra kruga, i za x i y od nultog ugla
    float r2 = 0.03; //poluprecnik

    float x2_center = 0.45;
    float y2_center = -0.7;


    circle2[0] = x2_center; //Centar X0
    circle2[1] = y2_center; //Centar Y0
    for (i = 0; i <= CRES; i++)
    {
        circle2[2 + 2 * i] = x2_center + r2 * cos((3.141592 / 180) * (i * 360 / CRES)); //Xi
        circle2[2 + 2 * i + 1] = y2_center + r2 * sin((3.141592 / 180) * (i * 360 / CRES)); //Yi
    }

    glBindVertexArray(VAO[3]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(circle2), circle2, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //zabranjena zona
    float circle3[CRES * 2 + 4];
    float r3 = 0.2;
    float x_center = 0.45;
    float y_center = 0.4;

    circle3[0] = x_center; 
    circle3[1] = y_center; 
    for (i = 0; i <= CRES; i++)
    {
        circle3[2 + 2 * i] = x_center + r3 * cos((3.141592 / 180) * (i * 360 / CRES));
        circle3[2 + 2 * i + 1] = y_center + r3 * sin((3.141592 / 180) * (i * 360 / CRES));
    }
    glBindVertexArray(VAO[5]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[5]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(circle3), circle3, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);



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
    unsigned UseTextLoc = glGetUniformLocation(texShader, "usetexture");



    unsigned uPosLoc = glGetUniformLocation(letShader, "uPos");
    unsigned colorLocation = glGetUniformLocation(letShader, "uColor");
    unsigned switchLocation = glGetUniformLocation(buttonShader, "swOnOff");
    unsigned batPosLoc = glGetUniformLocation(batShader, "batPos");

    


    float dx1 = 0;
    float dy1 = 0;
    float dx2 = 0;
    float dy2 = 0;
    float praznjenjeBat1 = 0;
    float praznjenjeBat2 = 0;

    int SwitchOnOff1 = 0;
    int SwitchOnOff2 = 0;
    int LetelicaDeaktivirana1 = 0;
    int LetelicaDeaktivirana2 = 0;

  
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.80, 0.80, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST); //depth test

        //glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //brisemo depth buffer ovde

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        glViewport(0, 0, wWidth / 2, wHeight);

        DroneShader.use();
        DroneShader.setMat4("uM", model);
        drone.Draw(DroneShader);

        glViewport(wWidth / 2, 0, wWidth / 2, wHeight);
        DroneShader.setMat4("uM", model);
        drone.Draw(DroneShader);

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


            if (SwitchOnOff1 == 1) {
                if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                {
                    if (dy1 <= 1.8)
                        dy1 += 0.001;
                    else LetelicaDeaktivirana1 = 1;
                }
                if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                {
                    if (dy1 >= -0.3)
                        dy1 -= 0.001;
                    else LetelicaDeaktivirana1 = 1;
                }
                if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                {
                    if (dx1 <= 1.6)
                        dx1 += 0.001;
                    else LetelicaDeaktivirana1 = 1;
                }
                if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                {
                    if (dx1 >= -0.6)
                        dx1 -= 0.001;
                    else LetelicaDeaktivirana1 = 1;
                }
            }


            if (SwitchOnOff2 == 1) {
                if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
                {
                    if (dy2 <= 1.8)
                        dy2 += 0.001;
                    else LetelicaDeaktivirana2 = 1;
                }
                if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
                {
                    if (dy2 >= -0.3)
                        dy2 -= 0.001;
                    else LetelicaDeaktivirana2 = 1;
                }
                if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
                {
                    if (dx2 <= 0.6)
                        dx2 += 0.001;
                    else LetelicaDeaktivirana2 = 1;
                }
                if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
                {
                    if (dx2 >= -1.6)
                        dx2 -= 0.001;
                    else LetelicaDeaktivirana2 = 1;
                }
            }

            //baterija
            if (SwitchOnOff1 == 1) {

                praznjenjeBat1 += 0.009;
                if (praznjenjeBat1 > 250) {
                    LetelicaDeaktivirana1 = 1;
                }
            }
            if (SwitchOnOff2 == 1) {

                praznjenjeBat2 += 0.009;
                if (praznjenjeBat2 > 250) {
                    LetelicaDeaktivirana2 = 1;
                }

            }

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
    glDeleteBuffers(n, VBO);
    glDeleteVertexArrays(n, VAO);
    glDeleteProgram(texShader);
    glDeleteProgram(letShader);
    glDeleteProgram(grayrectShader);


    glfwTerminate();
    return 0;
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
