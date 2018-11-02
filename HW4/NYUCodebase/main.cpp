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
enum GameMode { STATE_MAIN_MENU, STATE_GAME_LEVEL};
GameMode mode = STATE_MAIN_MENU;
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
    Entity(){
        
        alive = true;
    }
    Entity(float x, float y, float width, float height) : x(x), y(y), width(width), height(height) {
        alive=true;
        
    }
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
        float vertices[] = {-0.05f, -0.05f, 0.05f, -0.05f, 0.05f, 0.05f, -0.05f, -0.05f, 0.05f, 0.05f, -0.05f, 0.05f};
        glVertexAttribPointer(p.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(p.positionAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(p.positionAttribute);
    }
    
    float x;
    float y;
    float width;
    float height;
    float direction;
    bool alive;
    
  
    
    
    SheetSprite tex;
    
};

class GameState{
public:
    Entity *player;
    std::vector<Entity*> bullets;
    std::vector<Entity*> enemies;
    
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
Entity player;
int bullet_index = 0;
void shootBullet(std::vector<Entity*>& bullets){
    bullets[bullet_index % 19]->x = player.x;
    bullets[bullet_index % 19]->y = player.y + 0.25f;
    bullet_index++;
}

bool collision(Entity a, Entity b){
    float x = fabs(a.x - b.x) - (a.width+b.width)/2;
    float y = fabs(a.y-b.y)-(a.height+b.height)/2;
    if(x<0 && y<0){
        return true;
    }
    else{
        return false;
    }
}
void DrawText(ShaderProgram &program, int fontTexture, std::string text, float size, float spacing) {
    float character_size = 1.0/16.0f;
    std::vector<float> vertexData;
    std::vector<float> texCoordData;
    for(int i=0; i < text.size(); i++) {
        int spriteIndex = (int)text[i];
        float texture_x = (float)(spriteIndex % 16) / 16.0f;
        float texture_y = (float)(spriteIndex / 16) / 16.0f;
        vertexData.insert(vertexData.end(), {
            ((size+spacing) * i) + (-0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (-0.5f * size), -0.5f * size,
            ((size+spacing) * i) + (0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (0.5f * size), -0.5f * size,
            ((size+spacing) * i) + (0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (-0.5f * size), -0.5f * size,
        });
        texCoordData.insert(texCoordData.end(), {
            texture_x, texture_y,
            texture_x, texture_y + character_size,
            texture_x + character_size, texture_y,
            texture_x + character_size, texture_y + character_size,
            texture_x + character_size, texture_y,
            texture_x, texture_y + character_size,
        }); }
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program.positionAttribute);
    
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(program.texCoordAttribute);
    
    glDrawArrays(GL_TRIANGLES, 0, 6 * text.length());
    
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
}

GLuint fontTexture;
void drawMainMenu(ShaderProgram &p){
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-1.0f,0.0f, 0.0f));
    p.SetModelMatrix(modelMatrix);
    DrawText(p, fontTexture, "Space Invader", 0.2f, 0.0f);
}

void drawGameLevel(ShaderProgram &p, ShaderProgram &p_untextured, GameState &state){
    //draw player
    state.player->Draw(p);
    //draw bullets
    for (int i=0; i < 20; i++){
        state.bullets[i]->DrawUntex(p_untextured);
    }
    //draw enemies
    for (int i=0; i < 5; i++){
        state.enemies[i]->Draw(p);
    }
}
void Draw(ShaderProgram &p, ShaderProgram &p_untextured, GameState &state){
    switch(mode) {
        case STATE_MAIN_MENU:
            drawMainMenu(p);
            break;
        case STATE_GAME_LEVEL:
            drawGameLevel(p,p_untextured,state);
            break;
    }
    
    
}
const Uint8 *keys = SDL_GetKeyboardState(NULL);

void UpdateGameLevel(GameState &state, float elapsed){
    if(keys[SDL_SCANCODE_LEFT]) {
        if(player.x-player.width/2 > -1.77f){
            player.x -= elapsed;
        }
    } else if(keys[SDL_SCANCODE_RIGHT]) {
        if (player.x+player.width/2 < 1.77f){
            player.x += elapsed;
        }
    }
    //chceck bullet/enemy collision
    for(int b=0; b<20;b++){
        for(int e=0; e<5; e++){
            if(collision(*state.bullets[b], *state.enemies[e])){
                state.enemies[e]->alive = false;
                state.bullets[b]->x = 500.0f;
                state.enemies[e]->x = 500.0f;
            }
        }
    }
    
    //update enemy positions
    for(int i=0;i<5;i++){
        state.enemies[i]->x += state.enemies[i]->direction * elapsed;
    }
    for(int i=0;i<5;i++){
        if (state.enemies[i]->alive){
            if (state.enemies[i]->x - (state.enemies[i]->width / 2) <= -1.77f){
                for(int j=0;j<5;j++){
                    state.enemies[j]->direction *= -1.0f;
                }
            }
            else if (state.enemies[i]->x + (state.enemies[i]->width / 2) >= 1.77f){
                for(int j=0;j<5;j++){
                    state.enemies[j]->direction *= -1.0f;
                }
            }
        }
    }
    
    
    
    //update bullet positions
    for (int i = 0; i < 20; i++){
        state.bullets[i]->y += elapsed;
    }
}

void Update(GameState &state, float elapsed) {
    switch(mode) {
        case STATE_MAIN_MENU:
            break;
        case STATE_GAME_LEVEL:
            UpdateGameLevel(state, elapsed);
            break;
    } }



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
    fontTexture = LoadTexture(RESOURCE_FOLDER"font1.png");
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    
    projectionMatrix = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
    
    program.SetModelMatrix(modelMatrix);
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    
    program_unt.SetModelMatrix(modelMatrix);
    program_unt.SetProjectionMatrix(projectionMatrix);
    program_unt.SetViewMatrix(viewMatrix);
    
    glUseProgram(program.programID);
    glUseProgram(program_unt.programID);
    
    
    float lastFrameTicks = 0.0f;
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    GameState gameState;
   
    SheetSprite mySprite = SheetSprite(spriteSheetTexture, 425.0f/1024.0f, 468.0f/1024.0f, 93.0f/1024.0f, 84.0f/1024.0f, 0.2f);
    player.tex = mySprite;
    player.x = 0.0f;
    player.y = -0.5f;
    player.height = 0.5f * mySprite.size * 2;
    player.width =0.5f * mySprite.size * (mySprite.width / mySprite.height) * 2;
    gameState.player = &player;
    
    
    
    //creating bullets
    for (int i = 0; i < 20; ++i){
        Entity *bullet = new Entity(500.0f, 0.0f, 0.1f, 0.1f);
        gameState.bullets.push_back(bullet);
    }
    
    SheetSprite enemyTex = SheetSprite(spriteSheetTexture, 425.0f/1024.0f, 384.0f/1024.0f, 93.0f/1024.0f, 84.0f/1024.0f, 0.2f);
    float enemyWidth = 0.5f * enemyTex.size * (enemyTex.width / enemyTex.height) * 2;
    float enemyHeight = 0.5f * enemyTex.size * 2;
    Entity *enemy = new Entity(0.0f, 0.5f, enemyWidth, enemyHeight);
    enemy->direction = -1.0f;
    enemy->tex = enemyTex;
    gameState.enemies.push_back(enemy);
    
    float enemy_pos = 0.25f;
    for(int i=0; i<2; ++i){
        Entity *left = new Entity(enemy_pos, 0.5f, enemyWidth, enemyHeight);
        left->direction = -1.0f;
        left->tex = enemyTex;
        Entity *right = new Entity(-enemy_pos, 0.5f, enemyWidth, enemyHeight);
        right->direction = -1.0f;
        right->tex = enemyTex;
        gameState.enemies.push_back(left);
        gameState.enemies.push_back(right);
        enemy_pos *= 2;
        
    }
    
    
    SDL_Event event;
    bool done = false;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }else if(event.type == SDL_KEYDOWN){
                if (event.key.keysym.scancode == SDL_SCANCODE_SPACE){
                    if(mode == STATE_MAIN_MENU){
                        mode = STATE_GAME_LEVEL;
                    }
                    else {
                        shootBullet(gameState.bullets);
                    }
                    
                    
                    
                }
            }
            
        }
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        glClear(GL_COLOR_BUFFER_BIT);
        
        
        Update(gameState, elapsed);
        Draw(program, program_unt, gameState);
        
        
        
        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}
