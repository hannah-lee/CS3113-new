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

#include <vector>

SDL_Window* displayWindow;

class SheetSprite {
public:
    SheetSprite(){};
    SheetSprite(
                GLuint t, float givenU, float givenV, float width, float height, float
                size) : textureID(t), u(givenU), v(givenV), width(width), height(height), size(size) {}
    void Draw(ShaderProgram &program);
    float size;
    GLuint textureID;
    float u;
    float v;
    float width;
    float height;
};


void SheetSprite::Draw(ShaderProgram &program) {
    glBindTexture(GL_TEXTURE_2D, textureID);
    GLfloat texCoords[] = {
        u, v+height,
        u+width, v,
        u, v,
        u+width, v,
        u, v+height,
        u+width, v+height
    };
    float aspect = width / height;
    float vertices[] = {
        -0.5f * size * aspect, -0.5f * size,
        0.5f * size * aspect, 0.5f * size,
        -0.5f * size * aspect, 0.5f * size,
        0.5f * size * aspect, 0.5f * size,
        -0.5f * size * aspect, -0.5f * size ,
        0.5f * size * aspect, -0.5f * size};
    
    // draw our arrays
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);
    
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program.texCoordAttribute);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
}

class Entity{
public:
    Entity(){}
    Entity(float x, float y) : x(x), y(y) {}
    void Draw(ShaderProgram &p){
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(x,y, 0.0f));
        p.SetModelMatrix(modelMatrix);
        tex.Draw(p);
    }
    
    void DrawUntex(ShaderProgram &p){ //draws squares
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(x,y, 0.0f));
        p.SetModelMatrix(modelMatrix);
        float vertices[] = {-0.25, -0.25, 0.25, -0.25, 0.25, 0.25, -0.25, -0.25, 0.25, 0.25, -0.25, 0.25};
        glVertexAttribPointer(p.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(p.positionAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(p.positionAttribute);
    }
    
    float x;
    float y;
    
   // float playerTrueWidth = 0.5f * texture.size * (texture.width / texture.height) * 2;
    //float playerTrueHeight = 0.5f * texture.size * 2;
    
    
    SheetSprite tex;
    
};

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
Entity player(0.0f, 0.0f);
int bullet_index = 0;
void shootBullet(std::vector<Entity*>& bullets){
    bullets[bullet_index % 19]->x = player.x;
    bullets[bullet_index % 19]->y = player.y + 0.25f;
    bullet_index++;
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
    program_unt.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    
    GLuint spriteSheetTexture = LoadTexture(RESOURCE_FOLDER"sheet.png");
   
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    
    projectionMatrix = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
    
    program.SetModelMatrix(modelMatrix);
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    
    glUseProgram(program.programID);
    glUseProgram(program_unt.programID);
    
    
    float lastFrameTicks = 0.0f;
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    SheetSprite mySprite = SheetSprite(spriteSheetTexture, 425.0f/1024.0f, 468.0f/1024.0f, 93.0f/1024.0f, 84.0f/1024.0f, 0.2f);
    player.tex = mySprite;
    
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    
    std::vector<Entity*> bullets;
    //creating bullets
    for (int i = 0; i < 20; ++i){
        Entity *bullet = new Entity(500.0f, 0.0f);
        bullets.push_back(bullet);
    }
    
    
    SDL_Event event;
    bool done = false;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }else if(event.type == SDL_KEYDOWN){
                if (event.key.keysym.scancode == SDL_SCANCODE_SPACE){
                    int temp = bullet_index % 19;
                    std::cout << bullets[temp]->x  <<std::endl;

                    shootBullet(bullets);
                    
                    
                }
            }
            
        }
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        glClear(GL_COLOR_BUFFER_BIT);
        
        player.Draw(program);
        if(keys[SDL_SCANCODE_LEFT]) {
            player.x -= elapsed;
        } else if(keys[SDL_SCANCODE_RIGHT]) {
//            if (player.x+player.width/2 < 1.77f){
                player.x += elapsed;
//            }
        }
        
        for (int i = 0; i < 20; i++){
            bullets[i]->y += elapsed;
        }
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        program_unt.SetModelMatrix(modelMatrix);
        float vertices[] = {-0.25f, -0.25f, 0.25f, -0.25f, 0.25f, 0.25f, -0.25f, -0.25f, 0.25f, 0.25f, -0.25f, 0.25f};
        glVertexAttribPointer(program_unt.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program_unt.positionAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program_unt.positionAttribute);
        for (int i=0; i < 20; i++){
            bullets[i]->DrawUntex(program_unt);
            std::cout << bullets[i]->x  << " " << bullets[i]->y <<std::endl;
        }
        
        
        
        
        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}
