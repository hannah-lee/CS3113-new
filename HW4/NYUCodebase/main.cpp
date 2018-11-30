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
#include <unordered_set>
#include "FlareMap.h"

GLuint spritesheet;
SDL_Window* displayWindow;
enum GameMode { STATE_MAIN_MENU, STATE_GAME_LEVEL};
GameMode mode = STATE_MAIN_MENU;
void DrawSpriteSheetSprite(ShaderProgram &program, int index, int spriteCountX,
                           int spriteCountY, GLuint textureID, float width, float height) {
    glBindTexture(GL_TEXTURE_2D, textureID);
    float u = (float)(((int)index) % spriteCountX) / (float) spriteCountX;
    float v = (float)(((int)index) / spriteCountX) / (float) spriteCountY;
    float spriteWidth = 1.0/(float)spriteCountX;
    float spriteHeight = 1.0/(float)spriteCountY;
    float texCoords[] = {
        u, v+spriteHeight,
        u+spriteWidth, v,
        u, v,
        u+spriteWidth, v,
        u, v+spriteHeight,
        u+spriteWidth, v+spriteHeight
    };
    float vertices[] = {-width/2.0f, -height/2.0f, width/2.0f, height/2.0f, -width/2.0f, height/2.0f, width/2.0f, height/2.0f,  -width/2.0f,
        -height/2.0f, width/2.0f, -height/2.0f};
    
    
    // draw our arrays
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);
    
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program.texCoordAttribute);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
}
float lerp(float v0, float v1, float t) {
    return (1.0-t)*v0 + t*v1;
}
float tileSize = 0.09f;
const Uint8 *keys = SDL_GetKeyboardState(NULL);
std::pair<int, int> worldToTileCoordinates(float worldX, float worldY) {
    return {(int)(worldX / tileSize), (int)(worldY / -tileSize)};
}
std::unordered_set<int> solids;
FlareMap map;

class Entity{
public:
    Entity(){
    }
    Entity(float x, float y, float width, float height, int i, std::string t) : index(i), type(t){
        
        position = glm::vec3(x,y,0.0f);
        size = glm::vec3(width, height,0.0f);
        friction = glm::vec3(0.1f, 0.1f, 0.0f);

    }
    void Draw(ShaderProgram &p){
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, position);
        p.SetModelMatrix(modelMatrix);
        DrawSpriteSheetSprite(p, index, 16, 8, spritesheet, size.x, size.y);
    }

    glm::vec3 position;
    glm::vec3 size;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    glm::vec3 friction;
    
    int index;
    std::string type;
    
    void collisionY(){
        std::pair<int, int> tiled_coord = worldToTileCoordinates(position.x, position.y - (size.y/2.0f));
        int tiledX = tiled_coord.first;
        int tiledY = tiled_coord.second;
        if (type == "player")
            std::cout << tiledY << " " << tiledX << std::endl;
        if(tiledY < map.mapHeight && tiledX < map.mapWidth){
            int curr_tile = map.mapData[tiledY][tiledX];
            if (type == "player")
                std::cout << "at tile: " << curr_tile << std::endl;
            if (solids.find(curr_tile) != solids.end()){
                std::cout << "collision" << std::endl;
                float penetration = fabs((-tileSize*tiledY) -(position.y - (size.y/2.0f)));
                position.y += penetration + 0.0001f;
            }

        }
        
        
    }
    
    void Update(float elapsed){
        //start of frame, set acceleration to 0
        //if left, -
        //if right +
        //then apply the updates player.velocity.x += acceleration
        acceleration.x = 0.0f;
        acceleration.y = 0.0f;
        if(type == "player"){
            if(keys[SDL_SCANCODE_LEFT]) {
                // go left
                acceleration.x = -5.0f * elapsed;
            } else if(keys[SDL_SCANCODE_RIGHT]) {
                // go right!
                acceleration.x = 5.0f * elapsed;
            }
        }
        acceleration.y = -1.0f * elapsed; //gravity
        velocity.x = lerp(velocity.x, 0.0f, elapsed * friction.x);
        velocity.y = lerp(velocity.y, 0.0f, elapsed * friction.y);
        velocity.x += acceleration.x * elapsed;
        velocity.y += acceleration.y * elapsed;
        position.y += velocity.y * elapsed;
        collisionY();
        position.x += velocity.x * elapsed;
        //collisionX();
    }
    

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
bool collision(Entity a, Entity b){
    float x = fabs(a.position.x - b.position.x) - (a.size.x+b.size.x)/2;
    float y = fabs(a.position.y-b.position.y)-(a.size.y+b.size.y)/2;
    if(x<0 && y<0){
        return true;
    }
    else{
        return false;
    }
}
//Entity player;
//int bullet_index = 0;
//void shootBullet(std::vector<Entity*>& bullets){
//    bullets[bullet_index % 19]->x = player.x;
//    bullets[bullet_index % 19]->y = player.y + 0.25f;
//    bullet_index++;
//}
//
//bool collision(Entity a, Entity b){
//    float x = fabs(a.x - b.x) - (a.width+b.width)/2;
//    float y = fabs(a.y-b.y)-(a.height+b.height)/2;
//    if(x<0 && y<0){
//        return true;
//    }
//    else{
//        return false;
//    }
//}

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




void drawTileMap(ShaderProgram& p){
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    p.SetModelMatrix(modelMatrix);
    std::vector<float> vertexData;
    std::vector<float> texCoordData;
    int SPRITE_COUNT_X = 16;
    int SPRITE_COUNT_Y = 8;
    for(int y=0; y < map.mapHeight; y++) {
        for(int x=0; x < map.mapWidth; x++) {
            if (map.mapData[y][x] != 0){
                float u = (float)(((int)map.mapData[y][x]) % SPRITE_COUNT_X) / (float) SPRITE_COUNT_X;
                float v = (float)(((int)map.mapData[y][x]) / SPRITE_COUNT_X) / (float) SPRITE_COUNT_Y;
                float spriteWidth = 1.0f/(float)SPRITE_COUNT_X;
                float spriteHeight = 1.0f/(float)SPRITE_COUNT_Y;
                vertexData.insert(vertexData.end(), {
                    tileSize * x, -tileSize * y,
                    tileSize * x, (-tileSize * y)-tileSize,
                    (tileSize * x)+tileSize, (-tileSize * y)-tileSize,
                    tileSize * x, -tileSize * y,
                    (tileSize * x)+tileSize, (-tileSize * y)-tileSize,
                    (tileSize * x)+tileSize, -tileSize * y
                });
                texCoordData.insert(texCoordData.end(), {
                    u, v,
                    u, v+(spriteHeight),
                    u+spriteWidth, v+spriteHeight,
                    
                    u, v,
                    u+spriteWidth, v+spriteHeight,
                    u+spriteWidth, v
                });
            }
            
        }
    }
    glBindTexture(GL_TEXTURE_2D, spritesheet); //change texture
    glVertexAttribPointer(p.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(p.positionAttribute);
    
    glVertexAttribPointer(p.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(p.texCoordAttribute);
    
    glDrawArrays(GL_TRIANGLES, 0, vertexData.size()/2);
    
    glDisableVertexAttribArray(p.positionAttribute);
    glDisableVertexAttribArray(p.texCoordAttribute);
}
std::vector<Entity> entities;
void Update(float elapsed){
    for( int i=0; i<entities.size(); i++){
        entities[i].Update(elapsed);
    }
        for(int i=0; i< entities.size(); i++){
            for(int j=0; j<entities.size(); j++){
                if (i != j && collision(entities[i], entities[j])){
                    if (entities[i].type == "enemy" && entities[j].type == "player"){
                        entities[i].position.x = 500.0f;
                    }
                    else if (entities[j].type == "enemy" && entities[i].type == "player"){
                        entities[j].position.x = 500.0f;
    
                    }
                }
            }
        }

    
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
    map.Load(RESOURCE_FOLDER"untitled..txt");
    solids.insert(2);
    glViewport(0, 0, 640, 360);
    
    //this supports textures
    ShaderProgram program;
    program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    
    //program for untextured
    ShaderProgram program_unt;
    program_unt.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    
//    GLuint spriteSheetTexture = LoadTexture(RESOURCE_FOLDER"sheet.png");
    fontTexture = LoadTexture(RESOURCE_FOLDER"font1.png");
    spritesheet = LoadTexture(RESOURCE_FOLDER"arne_sprites.png");
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
    for(FlareMapEntity &entity : map.entities) {
        std::cout << entity.type << std::endl;
        if(entity.type == "player"){
            Entity player1 =Entity(entity.x * tileSize,entity.y * -tileSize, tileSize, tileSize, 98, "player");
            entities.push_back(player1);
        }
        else if(entity.type == "enemy"){
            Entity enemy1 =Entity(entity.x * tileSize, entity.y * -tileSize,tileSize, tileSize, 80, "enemy");
            entities.push_back(enemy1);
        }
        }
        //place at entity.x & entity.y
        //based on entity.type which is an  std::string


    SDL_Event event;
    bool done = false;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }else if(event.type == SDL_KEYDOWN){
                if (event.key.keysym.scancode == SDL_SCANCODE_SPACE){
                    
                    
                }
            }
            
        }
        
        
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        glClear(GL_COLOR_BUFFER_BIT);

        drawTileMap(program);
        
        //drawing entities
        for( int i=0; i<entities.size(); i++){
            entities[i].Draw(program);
            //determines player & centers screen at player
            if (entities[i].type == "player"){
                viewMatrix = glm::mat4(1.0f);
                viewMatrix = glm::translate(viewMatrix, glm::vec3(-entities[i].position.x,-entities[i].position.y, 0.0f));
                program.SetViewMatrix(viewMatrix);
            }
            
        }
        
        Update(elapsed);
        
        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}


