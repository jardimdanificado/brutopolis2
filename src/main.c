#include "raylib.h"
#include "raymath.h"
#include <math.h>
#include "bruter.h"

#define MAX_ENEMIES 10
#define BULLET_SPEED 1.0f

typedef struct 
{
    Vector3 position;
    char type; 
    Int capacity; // if storage
    Int content;
    Int 
} Item;

typedef struct 
{
    Vector3 position;
    Vector3 size;
    Vector3 direction;
    Color color;
    Float speed;
    Int current_item;
} Creature;

typedef struct 
{
    Vector3 position;
    Vector3 direction;
    Float speed;
} Bullet;

typedef Stack(Creature) CreatureList;
typedef Stack(Bullet) BulletList;

typedef struct 
{
    CreatureList *creatures;
    BulletList *bullets;
} World;

typedef struct 
{
    Int player_index;

} InternalSystem;

int main(void)
{
    const int screenWidth = 720;
    const int screenHeight = 480;

    InitWindow(screenWidth, screenHeight, "brutopolis 2");
    SetTargetFPS(60);
    Image handimg = LoadImage("data/img/glock-hand.png"); 
    ImageResize(&handimg, 300, 200);
    ImageRotate(&handimg, 25.0f);
    Texture2D handtexture = LoadTextureFromImage(handimg);      // Image converted to texture, uploaded to GPU memory (VRAM)
    UnloadImage(handimg);   // Once image has been converted to texture and uploaded to VRAM, it can be unloaded from RAM

    // Configuração da câmera personalizada
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 1.72f, 3.0f };
    camera.target = (Vector3){ 0.0f, 1.72f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 90.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Controles da câmera
    float mouseSensitivity = 0.003f;
    float playerSpeed = 0.1f;
    float cameraYaw = 0.0f;
    float cameraPitch = 0.0f;

    // Inimigos
    CreatureList *enemies = (CreatureList*)malloc(sizeof(CreatureList));
    stack_init(*enemies);
    
    for (int i = 0; i < MAX_ENEMIES; i++) 
    {
        Creature Creature = {0};
        Creature.position = (Vector3){ GetRandomValue(-10, 10), 0.5f, GetRandomValue(-10, -5) };
        Creature.size = (Vector3){ 1.0f, 1.0f, 1.0f };
        Creature.color = RED;
        stack_push(*enemies, Creature);
    }

    // Balas
    BulletList *bullets = (BulletList*)malloc(sizeof(BulletList));
    stack_init(*bullets);
    
    
    DisableCursor();

    while (!WindowShouldClose())
    {
        // Atualização da câmera
        // Rotação com mouse
        Vector2 mouseDelta = GetMouseDelta();
        cameraYaw += mouseDelta.x * mouseSensitivity;
        cameraPitch -= mouseDelta.y * mouseSensitivity;

        // Limitar rotação vertical
        cameraPitch = Clamp(cameraPitch, -PI/2.5f, PI/2.5f);

        // Calcular direção da câmera
        Vector3 direction = 
        {
            cosf(cameraYaw) * cosf(cameraPitch),
            sinf(cameraPitch),
            sinf(cameraYaw) * cosf(cameraPitch)
        };
        direction = Vector3Normalize(direction);
        camera.target = Vector3Add(camera.position, direction);

        // Movimento do jogador
        Vector3 move = {0};
        if (IsKeyDown(KEY_W)) move.x = 1;
        if (IsKeyDown(KEY_S)) move.x = -1;
        if (IsKeyDown(KEY_A)) move.z = -1;
        if (IsKeyDown(KEY_D)) move.z = 1;

        // Rotacionar movimento de acordo com a direção da câmera
        Vector3 rotatedMove = 
        {
            move.x * cosf(cameraYaw) - move.z * sinf(cameraYaw),
            0.0f,
            move.x * sinf(cameraYaw) + move.z * cosf(cameraYaw)
        };
        camera.position = Vector3Add(camera.position, Vector3Scale(rotatedMove, playerSpeed));

        // Atirar
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) 
        {
            Bullet bullet = {0};
            bullet.position = (Vector3){camera.position.x, camera.position.y, camera.position.z};
            bullet.direction = direction;
            bullet.speed = BULLET_SPEED;
            stack_push(*bullets, bullet);
        }

        // Atualizar balas
        for (int i = 0; i < bullets->size; i++)
        {
            bullets->data[i].position = Vector3Add(bullets->data[i].position, Vector3Scale(bullets->data[i].direction, bullets->data[i].speed));
            
            // Verificar colisão com inimigos
            for (int j = 0; j < enemies->size; j++) 
            {
                if (CheckCollisionBoxSphere(
                (BoundingBox){(Vector3){enemies->data[j].position.x - 0.5f, enemies->data[j].position.y - 0.5f, enemies->data[j].position.z - 0.5f},
                            (Vector3){enemies->data[j].position.x + 0.5f, enemies->data[j].position.y + 0.5f, enemies->data[j].position.z + 0.5f}},
                bullets->data[i].position,
                0.1f))
                {
                    // swap the current Creature with the last Creature, then pop the last Creature
                    stack_fast_remove(*enemies, j);
                    // we need to check the same index again, so we decrement j
                    j--;

                    // same for the bullet
                    stack_fast_remove(*bullets, i);
                    i--;
                }
            }
            
            // Desativar balas muito longe
            if (Vector3Distance(camera.position, bullets->data[i].position) > 50) 
            {
                stack_remove(*bullets, i);
            }
        }

        BeginDrawing();
            ClearBackground(RAYWHITE);

            BeginMode3D(camera);
                // Chão
                DrawPlane((Vector3){0.0f, 0.0f, 0.0f}, (Vector2){50.0f, 50.0f}, LIGHTGRAY);

                // Inimigos
                for (int i = 0; i < enemies->size; i++) 
                {
                    DrawCube(enemies->data[i].position, 1.0f, 1.0f, 1.0f, enemies->data[i].color);
                }

                // Balas
                for (int i = 0; i < bullets->size; i++) 
                {
                    DrawSphere(bullets->data[i].position, 0.01f, BLACK);
                }

            EndMode3D();

            // Mira
            DrawLine(screenWidth/2 - 10, screenHeight/2, screenWidth/2 + 10, screenHeight/2, GREEN);
            DrawLine(screenWidth/2, screenHeight/2 - 10, screenWidth/2, screenHeight/2 + 10, GREEN);

            DrawTexture(handtexture, screenWidth - handtexture.width + 70, screenHeight - handtexture.height+25, WHITE);

            //DrawText(TextFormat("Inimigos restantes: %d", enemies->size), 10, 10, 20, DARKGRAY);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}