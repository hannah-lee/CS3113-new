//Hannah Lee
//HW 2
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


GLuint LoadTexture(const char *image_path) {
    SDL_Surface *surface = IMG_Load(image_path);
    if (!surface){ return -1; }
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    SDL_FreeSurface(surface);

    return textureID;
}


class Paddle{
public:
    Paddle(float position, GLuint picture) : x(position), textureID(picture) {};
    
    //initial position
    float x;
    float y=0.0f; //middle
    GLuint textureID;
    
    //object info
    float width = 2.0f;
    float height = 3.0f;
    
    float left_side;
    float right_side;
    float top;
    float bottom;
    
    //movement
    float speed = 3.5f;
    float dir_y = 0.0f; //direction in y-axis
    const float dir_x = 0.0f; //constant
};
    
class Pong{
public:
    Pong(float position, float angle, GLuint pic): x(position), textureID(pic), angular_dir(angle){};
    
    //initial position
    float angular_dir;
    float x = cos(angular_dir);
    float y= sin(angular_dir);
    GLuint textureID;
    
    //object info
    float width = 0.5f;
    float height = 0.5f;
    
    //movement
    float speed = 2.5f;
    float dir_x = 0.0f;
    float dir_y = 0.0f;
    
    //collision info
    float left;
    float right;
    float top;
    float bottom;
    
    };

bool Collision(Pong ball, Paddle left, Paddle right){
    if (! (ball.bottom > right.top) || ball.top < right.bottom || ball.right<right.left_side){
        return true;
    }
    if (!(ball.bottom>left.top) || ball.top<left.bottom||ball.left>left.right_side){
        return true;
    }
    return false;
}


int main(int argc, char *argv[]){
    //setup start
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glViewport(0, 0, 640, 360);
    //setup end
    
    //this supports textures
    ShaderProgram program;
    program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    
    //load images
    GLuint emojiTexture = LoadTexture(RESOURCE_FOLDER"glow3.png");
    
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);

    projectionMatrix = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
    
    float lastFrameTicks = 0.0f;
    float angle = 0.0f;

    Paddle Left(-5.33f, PaddleTexture);
    Paddle Right(5.33f, PaddleTexture);
    Pong Center(PongTexture, 45);


    SDL_Event event;
    bool done = false;
    while (!done) {{
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        glClearColor(0.2f, 0.2f, 0.2f, 0.1f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        if(keys[SDL_SCANCODE_LEFT]) {
            angle += elapsed * 2.0 * 3.14;// go left!
        } else if(keys[SDL_SCANCODE_RIGHT]) {
            angle -= elapsed * 2.0 * 3.14;
            // go right!
        }
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
            if (winning == 1) {
                glClearColor(1.0, 0.0, 0.0, 1.0);
            }
            else if (winning == 2) {
                glClearColor(0.0, 1.0, 0.0, 1.0);
            }
            //background clear and color
            glClear(GL_COLOR_BUFFER_BIT);
            
            const Uint8 *keys = SDL_GetKeyboardState(NULL);
            
            if(event.type == SDL_KEYDOWN) {
                if(event.key.keysym.scancode == SDL_SCANCODE_SPACE) { //Is key pressed?
                    if(Right.dir_y > -3.0){
                        Right.dir_y -= elapsed * Right.speed;
                    }
                    else if (Right.dir_y < 3.0){
                        Right.dir_y += elapsed * Right.speed;
                    }
                   
                }
            }
            else if(event.type == SDL_MOUSEMOTION) { //Mouse is in motion
                // event.motion.x is the new x position
                // event.motion.y is the new y position
                if (Left.dir_y > -3.0){
                    Left.dir_y -= elapsed * Left.speed;
                    
                }
                else if (Left.dir_y < 3.0){
                    Left.dir_y += elapsed * Left.speed;
                }
                
            }

        }


//    modelMatrix.identity();
//    modelMatrix.Rotate(angle);

    program.SetModelMatrix(modelMatrix);
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);

    glUseProgram(program.programID);

        SDL_GL_SwapWindow(displayWindow);
    }
    SDL_Quit();
    return 0;}


