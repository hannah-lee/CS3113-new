#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;


GLuint LoadTexture(const char *filePath) {
    int w,h,comp;
    unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
    if(image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }
    GLuint retTexture;
    glGenTextures(1, &retTexture);
    glBindTexture(GL_TEXTURE_2D, retTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(image);
    return retTexture;
}

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 640, 360);
    
    //this supports textures
    ShaderProgram program;
    program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    
    //program for untextured
    ShaderProgram program_unt;
    program_unt.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    
    GLuint glow2Texture = LoadTexture(RESOURCE_FOLDER"glow2.png");
    GLuint glow4Texture = LoadTexture(RESOURCE_FOLDER"glow4.png");
    GLuint glow3Texture = LoadTexture(RESOURCE_FOLDER"glow3.png");
    GLuint starGoldTexture = LoadTexture(RESOURCE_FOLDER"starGold.png");
    
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    
    projectionMatrix = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
    
    glUseProgram(program.programID);
    glUseProgram(program_unt.programID);
    
    
    float angle = 0.0f;
    float lastFrameTicks = 0.0f;
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    SDL_Event event;
    bool done = false;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
        }
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        angle += elapsed;
        // rotate matrix by angle
        // draw sprite
        
        glClear(GL_COLOR_BUFFER_BIT);

        //untextured image
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(2.0f,0.0f, 1.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(2.f, 2.0f, 1.0f));
        program_unt.SetModelMatrix(modelMatrix);
        program_unt.SetProjectionMatrix(projectionMatrix);
        program_unt.SetViewMatrix(viewMatrix);
        program_unt.SetColor(0.2f, 0.8f, 0.4f, 1.0f);
        
        float vertices[] = {0.5f, -0.5f, 0.0f, 0.5f, -0.5f, -0.5f};
        glVertexAttribPointer(program_unt.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program_unt.positionAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 3);
       
        program.SetModelMatrix(modelMatrix);
        program.SetProjectionMatrix(projectionMatrix);
        program.SetViewMatrix(viewMatrix);
     
        //glow2 = strongest glow
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(2.0f, 2.0f, 1.0f)); //scale
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.8f, 0.0f, 0.0f)); //translate
        
        program.SetModelMatrix(modelMatrix);
        
        float glow2Vertices[] = {-0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, glow2Vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        
        float glow2TexCoords[] = {0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f};
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, glow2TexCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        glBindTexture(GL_TEXTURE_2D, glow2Texture);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        //glow4 = middle glow
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(2.0f, 2.0f, 1.0f)); //scale
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.3f, 0.0f)); //translate
        
        program.SetModelMatrix(modelMatrix);
        
        float glow4Vertices[] = {-0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, glow4Vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        
        float glow4TexCoords[] = {0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f};
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, glow4TexCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        glBindTexture(GL_TEXTURE_2D, glow4Texture);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);

        //glow3 = weakest glow
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(2.0f, 2.0f, 1.0f)); //scale
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.8f, 0.0f, 0.0f)); //translate
        
        program.SetModelMatrix(modelMatrix);
        
        float glow3Vertices[] = {-0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, glow3Vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        
        float glow3TexCoords[] = {0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f};
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, glow3TexCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        glBindTexture(GL_TEXTURE_2D, glow3Texture);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        //starGold
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f)); //scale
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -0.5f, 0.0f)); //translate
        modelMatrix = glm::rotate(modelMatrix, -angle, glm::vec3(0.0f, 0.0f, 1.0f)); //rotate
        
        program.SetModelMatrix(modelMatrix);
        
        float starGoldVertices[] = {-0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, starGoldVertices);
        glEnableVertexAttribArray(program.positionAttribute);
        
        float starGoldTexCoords[] = {0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f};
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, starGoldTexCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        glBindTexture(GL_TEXTURE_2D, starGoldTexture);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}
