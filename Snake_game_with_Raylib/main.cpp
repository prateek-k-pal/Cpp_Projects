#include <iostream>
//#include <raylib.h>
#include "raylib/raylib/src/raylib.h"
#include <deque>
//#include <raymath.h>
#include "raylib/raylib/src/raymath.h"
using namespace std;


Color black = {0, 0, 0, 255};
Color grey = {200, 200, 200, 170};
Color white = { 255, 255, 255, 255};
Color green = {173, 204, 96, 255};
Color darkGreen = {43, 51, 24, 255};
Color blue = { 0, 0, 200, 200};
Color red = {200, 0, 0, 255};

int cellSize = 30;
int cellCount = 25;
int offset = 75;

class Level{
    public:
            int level;
            Color bgColor;
            Color fgColor;
            double speed;
        Level(int stage, Color bg, Color fg, double s){
            level = stage;
            bgColor = bg;
            fgColor = fg;
            speed = s;
        }
};

Level level = Level( 0, black, white, 0.2);

double lastUpdateTime = 0;

bool ElementInDeque(Vector2 element, deque<Vector2> deque){
    for(unsigned int i = 0; i < deque.size(); i++){
        if(Vector2Equals(deque[i], element)){
            return true;
        }
    }

    return false;
}

bool eventTriggered(double interval){
    double currentTime = GetTime();
    if(currentTime - lastUpdateTime >= interval){
        lastUpdateTime = currentTime;
        return true;
    }

    return false;
}

class Snake{
    public:
        deque<Vector2> body = {Vector2{6,9}, Vector2{5,9}, Vector2{4,9}};
        Vector2 direction = {1,0};
        bool addSegment = false;

        void Draw(Color random){
            for(unsigned int i = 0; i < body.size(); i++){
                int x = body[i].x;
                int y = body[i].y;
                Rectangle segment = Rectangle{(float) offset + x*cellSize, (float) offset + y*cellSize, (float) cellSize, (float) cellSize};
                DrawRectangleRounded(segment, 0.5, 6, random);
            }
        }

        void Update(){
            body.push_front(Vector2Add(body[0], direction));
            if(addSegment == true){
                addSegment = false;
            }
            else{                
                body.pop_back();
            }
        }

        void Reset(){
            body = {Vector2{6,9}, Vector2{5,9}, Vector2{4,9}};
            direction = {1,0};
        }
};

class Food{
    public:
        Vector2 position;
        Texture2D texture;

        Food(deque<Vector2> snakeBody){
            Image image = LoadImage("Graphics/food.png");
            texture = LoadTextureFromImage(image);
            UnloadImage(image);
            position = GenerateRandomPos(snakeBody);
        }

        ~Food(){
            UnloadTexture(texture);
        }

        void Draw(){
            DrawTexture(texture, offset + position.x * cellSize, offset + position.y * cellSize, WHITE);
        }

        Vector2 GenerateRandomCell(){
            float x = GetRandomValue(0, cellCount - 1);
            float y = GetRandomValue(0, cellCount - 1);
            return Vector2{x,y};
        }

        Vector2 GenerateRandomPos(deque<Vector2> snakeBody){
            Vector2 position = GenerateRandomCell();
            while(ElementInDeque(position, snakeBody)){
                position = GenerateRandomCell();
            }
            return position;
        }
};

class Game{
    public:
        Snake snake = Snake();
        Food food = Food(snake.body);
        bool running = true;
        int score = 0;
        Sound eatSound;
        Sound wallSound;

        Game(){
            InitAudioDevice();
            eatSound = LoadSound("Sounds/eat.mp3");
            wallSound = LoadSound("Sounds/wall.mp3");
        }

        ~Game(){
            UnloadSound(eatSound);
            UnloadSound(wallSound);
            CloseAudioDevice();
        }

        void Draw(Color random){
            food.Draw();
            snake.Draw(random);
        }

        void UpdateInfinite(){
            if(running){
                snake.Update();
                CheckCollisionWithEdgesInfinite();
                CheckCollisionWithFood();
                CheckCollisionWithTailInfinite();
            }
        }

        void UpdateLeveled(){
            if(running){
                snake.Update();
                CheckCollisionWithEdgesLeveled();
                CheckCollisionWithFood();
                CheckCollisionWithTailLeveled();
            }
        }

        void CheckCollisionWithFood(){
            if(Vector2Equals(snake.body[0], food.position)){
                food.position = food.GenerateRandomPos(snake.body);
                snake.addSegment = true;
                score++;
                PlaySound(eatSound);
            }
        }

        void CheckCollisionWithEdgesLeveled(){
            if(snake.body[0].x == cellCount || snake.body[0].x == -1){
                level = Level{ 0, green, darkGreen, 0.2};
                GameOver();
            }

            if(snake.body[0].y == cellCount || snake.body[0].y == -1){
                level = Level{ 0, green, darkGreen, 0.2};
                GameOver();
            }
        }

        void CheckCollisionWithEdgesInfinite(){
            if(snake.body[0].x == cellCount){
                snake.body[0].x = 0;
            }
            else if(snake.body[0].x == -1){
                snake.body[0].x = cellCount - 1;
            }

            if(snake.body[0].y == cellCount){
                snake.body[0].y = 0;
            }
            else if(snake.body[0].y == -1){
                snake.body[0].y = cellCount - 1;
            }
        }

        void CheckCollisionWithTailInfinite(){
            deque<Vector2> headlessBody = snake.body;
            headlessBody.pop_front();
            if(ElementInDeque(snake.body[0], headlessBody)){
                GameOver();
            }
        }

        void CheckCollisionWithTailLeveled(){
            deque<Vector2> headlessBody = snake.body;
            headlessBody.pop_front();
            if(ElementInDeque(snake.body[0], headlessBody)){
                level = Level{ 0, green, darkGreen, 0.2};
                GameOver();
            }
        }

        void GameEnd(){

        }

        void GameOver(){
            snake.Reset();
            food.position = food.GenerateRandomPos(snake.body);
            running = false;
            score = 0;
            PlaySound(wallSound);
        }
};



class Play{
    public:
        bool flagLevel = false;
        void snakePlay(bool infinite){
            Game game = Game();
            game.score = 0;
            flagLevel = false;

            while(WindowShouldClose() == false){
                BeginDrawing();

                if(eventTriggered(level.speed)){
                    if(infinite){
                        game.UpdateInfinite();
                    }
                    else{
                        game.UpdateLeveled();
                    }
                }

                game.Draw(level.fgColor);
                
                if(IsKeyPressed(KEY_UP) && game.snake.direction.y != 1){
                    game.snake.direction = {0,-1};
                    game.running = true;
                }

                if(IsKeyPressed(KEY_DOWN) && game.snake.direction.y != -1){
                    game.snake.direction = {0,1};
                    game.running = true;
                }

                if(IsKeyPressed(KEY_LEFT) && game.snake.direction.x != 1){
                    game.snake.direction = {-1,0};
                    game.running = true;
                }

                if(IsKeyPressed(KEY_RIGHT) && game.snake.direction.x != -1){
                    game.snake.direction = {1,0};
                    game.running = true;
                }
                
                ClearBackground(level.bgColor);
                DrawRectangleLinesEx(Rectangle{ (float) offset - 5, (float) offset - 5, (float) cellSize * cellCount + 10, (float) cellSize * cellCount + 10}, 5, level.fgColor);
                if(infinite == 1){
                    DrawText("Infinite Snake", offset - 5, 20, 40, level.fgColor);
                    DrawText(TextFormat("%i", game.score) , offset + cellSize*cellCount, 20, 40, level.fgColor);
                }
                else{
                    DrawText("Level Up Snake", offset - 5, 20, 40, level.fgColor);
                    if(level.level < 4){
                        DrawText(TextFormat("Next Level in %i", ( level.level*10 + 10 ) - game.score) , offset + cellSize*15, 20, 40, level.fgColor);
                    }
                    else{
                        DrawText(TextFormat("Main Menu in %i", ( level.level*10 + 10 ) - game.score) , offset + cellSize*15, 20, 40, level.fgColor);
                    }
                }

                DrawText("Press ESC for Main Menu.", offset , offset + cellSize * cellCount + 20, 20, level.fgColor);

                if(infinite == 0 ){
                    if(level.level == 0 && game.score == 10){
                        ++level.level;
                        flagLevel = true;
                    }
                    else if(level.level == 1 && game.score == 20){
                        ++level.level;
                        flagLevel = true;
                    }
                    else if(level.level == 2 && game.score == 30){
                        ++level.level;
                        flagLevel = true;
                    }
                    else if(level.level == 3 && game.score == 40){
                        ++level.level;
                        flagLevel = true;
                    }
                    else if(level.level == 4 && game.score == 50){
                        game.GameEnd();
                        ++level.level;
                        flagLevel = true;
                    }
                }
                EndDrawing();

                if(flagLevel == true){
                    break;
                }
            }
        }

        void infiniteSnake(){
            snakePlay(1);
        }

        void Transition(Level bf, Level af){
            deque<Vector2> body = { Vector2{5,13}, Vector2{4,13}, Vector2{3,13}, Vector2{2,13}, Vector2{1,13}, Vector2{0,13}};

            while(body[0].x < cellCount){
                BeginDrawing();

                ClearBackground(black);
                DrawRectangleLinesEx(Rectangle{ (float) offset - 5, (float) offset - 5, (float) cellSize * cellCount + 10, (float) cellSize * cellCount + 10}, 5, white);
                DrawText("Transition", offset - 5, 20, 40, white);
                DrawText("Press ESC for Main Menu.", offset , offset + cellSize * cellCount + 20, 20, white);
                DrawRectangle(offset, offset, (cellSize*cellCount)/2, cellSize*cellCount, bf.bgColor);
                DrawRectangle(offset + (cellSize*cellCount)/2 , offset, (cellSize*cellCount)/2, cellSize*cellCount, af.bgColor);

                for(unsigned int i = 0; i < body.size(); i++){
                    int x = body[i].x;
                    int y = body[i].y;
                    if(x <= cellCount/2 ){
                        Rectangle segment = Rectangle{(float) offset + x*cellSize, (float) offset + y*cellSize, (float) cellSize, (float) cellSize};
                        DrawRectangleRounded(segment, 0.5, 6, bf.fgColor);
                    }
                    else if(x > cellCount/2 ){
                        Rectangle segment = Rectangle{(float) offset + x*cellSize, (float) offset + y*cellSize, (float) cellSize, (float) cellSize};
                        DrawRectangleRounded(segment, 0.5, 6, af.fgColor);
                    }
                }

                if(eventTriggered(0.1)){
                    body.push_front(Vector2Add(body[0], Vector2{1,0}));
                    body.pop_back();
                }

                if(body[0].x == cellCount){
                    eventTriggered(0.5);
                }
                
                EndDrawing();
            }
        }

        void leveledSnake(){
            Level temp = Level( 0, black, white, 0.2);
            while(level.level < 5){
                switch (level.level){
                    case 1:
                        Transition(temp, level);
                        snakePlay(0);
                        if(level.level == 2){
                            temp = Level(1, blue, red, 0.188);
                            level = Level(2, grey, darkGreen, 0.176);
                        }
                        break;

                    case 2:
                        Transition(temp, level);
                        snakePlay(0);
                        if(level.level == 3){
                            temp = Level(2, grey, darkGreen, 0.176);
                            level = Level(3, blue, grey, 0.164);
                        }
                        break;

                    case 3:
                        Transition(temp, level);
                        snakePlay(0);
                        if(level.level == 4){
                            temp = Level(3, blue, grey, 0.164);
                            level = Level(4, green, red, 0.15);
                        }
                        break;

                    case 4:
                        Transition(temp, level);
                        snakePlay(0);
                        break;

                    default:
                        level = Level(0, green, darkGreen, 0.2);
                        snakePlay(0);
                        if(level.level == 1){
                            temp = Level(0, green, darkGreen, 0.2);
                            level = Level(1, blue, red, 0.188);
                        }
                        break;
                }

                if(flagLevel == false){
                    break;
                }

            }
            level = Level(0, black, white, 0.2);
        }
};

class Info{
    int page = 0;
    Texture2D texture_infinite, texture_levelup;
    public:
        Info(){
            Image image = LoadImage("Graphics/level_up.png");
            texture_levelup = LoadTextureFromImage(image);
            image = LoadImage("Graphics/infinite_snake.png");
            texture_infinite = LoadTextureFromImage(image);
            UnloadImage(image);
        }

        ~Info(){
            UnloadTexture(texture_infinite);
            UnloadTexture(texture_levelup);
        }

        void howToPlay(){
            while(WindowShouldClose() == false){
                BeginDrawing();
                if(IsKeyPressed(KEY_RIGHT) && page < 2){
                    ++page;
                }
                else if(IsKeyPressed(KEY_LEFT) && page < 2){
                    --page;
                }
                else if(page > 1){
                    page = 0;
                }
                else if( page < 0 ){
                    page = 1;
                }

                switch(page){
                    case 1:
                        ClearBackground(black);
                        DrawRectangleLinesEx(Rectangle{ (float) offset - 5, (float) offset - 5, (float) cellSize * cellCount + 10, (float) cellSize * cellCount + 10}, 5, white);
                        DrawText("How to Play", offset - 5, 20, 40, white);
                        DrawText("This game has 2 game modes:-\n\n\t\t1. Infinite Snake\n\n\t\t2. Level Up Snake\n\n", offset + cellSize - 15, offset + cellSize, 20, white);
                        DrawText("Level Up Snake", offset + cellSize - 15, offset + cellSize*5, 40, white);
                        DrawText("The game is played with the arrow keys (up, down, right, left) there\n\nare boundaries and if the snake reaches a boundary its game over.\n\nThe snake also gets killed is by hitting its head on its body. Once its\n\nGame Over you can restart it. Pressing multiple keys also means\n\nGame Over so keep that in mind. There are 5 levels in total.", offset + cellSize - 15, offset + cellSize*7, 20, white);
                        DrawTextureEx(texture_levelup, Vector2{(float) offset + cellSize*6, (float) offset + cellSize * 12}, 0, 0.4, WHITE);

                        DrawText("Press Left for prev page.", offset + cellSize - 15 , offset + cellSize * (cellCount - 1), 20, white);
                        DrawText("Press ESC for Main Menu.", offset , offset + cellSize * cellCount + 20, 20, white);
                        break;

                    default:
                        ClearBackground(green);
                        DrawRectangleLinesEx(Rectangle{ (float) offset - 5, (float) offset - 5, (float) cellSize * cellCount + 10, (float) cellSize * cellCount + 10}, 5, darkGreen);
                        DrawText("How to Play", offset - 5, 20, 40, darkGreen);
                        DrawText("This game has 2 game modes:-\n\n\t\t1. Infinite Snake\n\n\t\t2. Level Up Snake\n\n", offset + cellSize - 15, offset + cellSize, 20, darkGreen);
                        DrawText("Infinite Snake", offset + cellSize - 15, offset + cellSize*5, 40, darkGreen);
                        DrawText("The game is played with the arrow keys (up, down, right, left) there\n\nare no boundaries and if the snake reaches a boundary it emerges\n\non the opposite side. The only way the snake gets killed is by hitting\n\nits head on its body. Once its Game Over you can restart it. Pressing\n\nmultiple keys also means Game Over so keep that in mind.", offset + cellSize - 15, offset + cellSize*7, 20, darkGreen);
                        DrawTextureEx(texture_infinite, Vector2{(float) offset + cellSize*6, (float) offset + cellSize * 12}, 0, 0.4, WHITE);

                        DrawText("Press Right for next page.", offset + cellSize - 15 , offset + cellSize * (cellCount - 1), 20, darkGreen);
                        DrawText("Press ESC for Main Menu.", offset , offset + cellSize * cellCount + 20, 20, darkGreen);
                        break;
                }

                EndDrawing();
            }
        }

        void credits(){
            while(WindowShouldClose() == false){
                BeginDrawing();
                ClearBackground(black);
                DrawRectangleLinesEx(Rectangle{ (float) offset - 5, (float) offset - 5, (float) cellSize * cellCount + 10, (float) cellSize * cellCount + 10}, 5, white);
                DrawText("Credits", offset - 5, 20, 40, white);
                DrawText("This game is made by Prateek Kumar Pal\n\n\n( github @ prateek-k-pal )\n\n\n\n\nIt is further developed from a template from Nick Koumaris\n\n\nusing Cpp and Raylib library. The game and the template is\n\n\nunder MIT Copyright License.", offset + cellSize - 10, offset + cellSize, 24, white);
                EndDrawing();
            }
        }
};

class Menu{
    public:
        int arrow = 0;
        Play play = Play();
        Info info = Info();
        void draw(){
            DrawText("Retro Snake", offset + cellSize*6 + 2, offset + cellSize*1 , 60, white);
            DrawRectangleLinesEx(Rectangle{ (float) offset - 5, (float) offset - 5, (float) cellSize * cellCount + 10, (float) cellSize * cellCount + 10}, 5, white);
            DrawText("Use Arrow keys and Enter to select", offset + cellSize*6 + 7, offset + cellSize*4 , 20, white);
            DrawRectangleLinesEx(Rectangle{ (float) offset + cellSize*6 + 2, (float) offset + cellSize*5, (float) cellSize*12 + 26, (float) cellSize * 3 + 26}, 2, white);
            DrawText("Infinite Snake", offset + cellSize*7, offset + cellSize*6 + 10, 40, white);
            DrawRectangleLinesEx(Rectangle{ (float) offset + cellSize*6 + 2, (float) offset + cellSize*10, (float) cellSize*12 + 26, (float) cellSize * 3 + 26}, 2, white);
            DrawText("Level Up Snake", offset + cellSize*7, offset + cellSize*11 + 10, 40, white);
            DrawRectangleLinesEx(Rectangle{ (float) offset + cellSize*6 + 2, (float) offset + cellSize*15, (float) cellSize*12 + 26, (float) cellSize * 3 + 26}, 2, white);
            DrawText("How to Play", offset + cellSize*7, offset + cellSize*16 + 10, 40, white);
            DrawRectangleLinesEx(Rectangle{ (float) offset + cellSize*6 + 2, (float) offset + cellSize*20, (float) cellSize*12 + 26, (float) cellSize * 3 + 26}, 2, white);
            DrawText("Credits", offset + cellSize*7, offset + cellSize*21 + 10, 40, white);
        }

        void drawArrow(int x, int y){
            DrawLineEx(Vector2{ (float) offset + cellSize*x, (float) offset + cellSize*y}, Vector2{ (float) offset + cellSize*(x - 2), (float) offset + cellSize*y}, 5, white);
            DrawLineEx(Vector2{ (float) offset + cellSize*x, (float) offset + cellSize*y}, Vector2{ (float) offset + cellSize*(x - 1) + 15, (float) offset + cellSize*(y-1) + 15}, 5, white);
            DrawLineEx(Vector2{ (float) offset + cellSize*x, (float) offset + cellSize*y}, Vector2{ (float) offset + cellSize*(x - 1) + 15, (float) offset + cellSize*(y+1) - 15}, 5, white);
        }

        void start(){

            switch(arrow){
                case 1:
                drawArrow(5,12);
                break;

                case 2:
                drawArrow(5,17);
                break;

                case 3:
                drawArrow(5, 22);
                break;

                default:
                drawArrow(5,7);
                break;                
            }

            if(IsKeyPressed(KEY_DOWN)){
                ++arrow;
                if(arrow > 3){
                    arrow = 0;
                }
            }
            if(IsKeyPressed(KEY_UP)){
                --arrow;
                if(arrow < 0){
                    arrow = 3;
                }
            }

            if(IsKeyPressed(KEY_ENTER)){
                switch (arrow)
                {
                    case 0:
                        play.infiniteSnake();
                        break;

                    case 1:
                        play.leveledSnake();
                        break;

                    case 2:
                        info.howToPlay();
                        break;
                    
                    case 3:
                        info.credits();
                        break;
                }
            }
        }
};

int main () {

    cout<<"\nStarting the game..."<<endl;
    InitWindow(cellSize*cellCount + (2*offset), cellSize*cellCount  + (2*offset), "Retro Snake");
    SetTargetFPS(60);

    Menu menu = Menu();

    while (WindowShouldClose() == false)
    {
        BeginDrawing();
        ClearBackground(black);
        menu.draw();
        menu.start();
        DrawText("Press ESC to Exit.", offset , offset + cellSize * cellCount + 20, 20, white);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}