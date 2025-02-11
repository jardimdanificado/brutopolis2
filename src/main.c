#include "raylib.h"
#include "raymath.h"
#include <math.h>
#include "bruter.h"

#define MAX_ENEMIES 10
#define BULLET_SPEED 1.0f

enum 
{
    ITEM_HAND,
    ITEM_GLOCK,
    ITEM_BULLET_9MM
} items;

typedef struct 
{
    Vector3 position;
    char type; 
    Int capacity; // if storage
    Int content_type;
    Int content;
} Item;
typedef Stack(Item) ItemList;

typedef struct 
{
    char* name;
    Vector3 position;
    Vector3 size;
    Vector3 direction;
    Color color;
    Float speed;
    ItemList *inventory; 
    Int current_item;
} Creature;
typedef Stack(Creature) CreatureList;

typedef struct 
{
    Vector3 position;
    Vector3 direction;
    Float speed;
} Bullet;
typedef Stack(Bullet) BulletList;

typedef struct 
{
    CreatureList *creatures;
    BulletList *bullets;
} World;

typedef struct 
{
    Vector2 delta; // where applies
    Vector2 position; // where applies
    float sensibility;
} Mouse;

typedef struct 
{
    char* name;
    Int player_index;
    Vector2 resolution;
    World world;
    Camera camera;
    Mouse mouse;
} InternalSystem;

InternalSystem* new_system(char* name, int size_x, int size_y)
{
    InternalSystem* _sys = (InternalSystem*)malloc(sizeof(InternalSystem));
    _sys->player_index = -1;

    _sys->resolution = (Vector2){size_x, size_y};

    _sys->name = str_duplicate(name);

    _sys->world.creatures = (CreatureList*)malloc(sizeof(CreatureList));
    stack_init(*_sys->world.creatures);

    _sys->world.bullets = (BulletList*)malloc(sizeof(BulletList));
    stack_init(*_sys->world.bullets);

    return _sys;
}

void free_system(InternalSystem* _sys)
{
    free(_sys->name);
    stack_free(*_sys->world.creatures);
    stack_free(*_sys->world.bullets);
    free(_sys);
}

void system_startup(InternalSystem* _sys)
{
    InitWindow(_sys->resolution.x, _sys->resolution.y, _sys->name);
    SetTargetFPS(60);
    DisableCursor();

    // camera setup
    _sys->camera.position = (Vector3){ 0.0f, 1.72f, 3.0f };
    _sys->camera.target = (Vector3){ 0.0f, 1.72f, 0.0f };
    _sys->camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    _sys->camera.fovy = 90.0f;
    _sys->camera.projection = CAMERA_PERSPECTIVE;

    // mouse setup
    _sys->mouse.delta = (Vector2) {0,0};
    _sys->mouse.position = (Vector2) {0,0};
    _sys->mouse.sensibility = 0.003f;
}

Creature* new_creature(InternalSystem* _sys, char* name, int x, int y, int z)
{
    Creature* creature = (Creature*)malloc(sizeof(Creature));
    creature->position = (Vector3){ GetRandomValue(-10, 10), 0.5f, GetRandomValue(-10, -5) };
    creature->size = (Vector3){ 1.0f, 1.70f, 1.0f };
    creature->current_item = 0;
    creature->color = RED;
    creature->speed = 0.1f;
    
    creature->inventory = (ItemList*)malloc(sizeof(ItemList));

    creature->name = str_duplicate(name);
    creature->direction = (Vector3){0,0,0};

    stack_init(*creature->inventory);
    
    stack_push(*_sys->world.creatures, *creature);
    return creature;
}

Bullet* new_bullet(InternalSystem* _sys, Vector3 position, Vector3 direction, Float speed)
{
    Bullet *bullet = (Bullet*)malloc(sizeof(Bullet));
    bullet->position = position;
    bullet->direction = direction;
    bullet->speed = speed;
    stack_push(*_sys->world.bullets, *bullet);
    return bullet;
}

int main(void)
{
    InternalSystem* sys = new_system("brutopolis 3", 800, 600);

    system_startup(sys);

    Image handimg = LoadImage("data/img/glock-hand.png"); 
    ImageResize(&handimg, 300, 200);
    ImageRotate(&handimg, 25.0f);
    Texture2D handtexture = LoadTextureFromImage(handimg);      // Image converted to texture, uploaded to GPU memory (VRAM)
    UnloadImage(handimg);   // Once image has been converted to texture and uploaded to VRAM, it can be unloaded from RAM

    new_creature(sys, "joao451", 0, 0, 0);
    
    for (int i = 0;i < GetRandomValue(2,10);i++)
    {
        char* _name = TextFormat("joao%d", i);
        new_creature(sys, _name, GetRandomValue(-20,20), 0, GetRandomValue(-20,20));
    }

    sys->player_index = 0;

    // Controles da câmera
    float cameraYaw = 0.0f;
    float cameraPitch = 0.0f;

    while (!WindowShouldClose())
    {
        // Atualização da câmera
        // Rotação com mouse
        sys->mouse.delta = GetMouseDelta();
        cameraYaw += sys->mouse.delta.x * sys->mouse.sensibility;
        cameraPitch -= sys->mouse.delta.y * sys->mouse.sensibility;

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
        sys->camera.target = Vector3Add(sys->camera.position, direction);

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
        sys->world.creatures->data[sys->player_index].position = Vector3Add(sys->world.creatures->data[sys->player_index].position, Vector3Scale(rotatedMove, sys->world.creatures->data[sys->player_index].speed));

        sys->camera.position = Vector3Add(
            sys->world.creatures->data[sys->player_index].position,
            (Vector3){0.0f, 1.72f, 0.0f} // Offset de altura dos olhos
        );

        // Atirar
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) 
        {
            Bullet bullet = {0};
            bullet.position = (Vector3){sys->camera.position.x, sys->camera.position.y, sys->camera.position.z};
            bullet.direction = direction;
            bullet.speed = BULLET_SPEED;
            stack_push(*sys->world.bullets, bullet);
        }

        // Atualizar balas
        for (int i = 0; i < sys->world.bullets->size; i++)
        {
            sys->world.bullets->data[i].position = Vector3Add(sys->world.bullets->data[i].position, Vector3Scale(sys->world.bullets->data[i].direction, sys->world.bullets->data[i].speed));
            
            // Verificar colisão com inimigos
            for (int j = 0; j < sys->world.creatures->size; j++) 
            {
                if (CheckCollisionBoxSphere(
                (BoundingBox){(Vector3){sys->world.creatures->data[j].position.x - 0.5f, sys->world.creatures->data[j].position.y - 0.5f, sys->world.creatures->data[j].position.z - 0.5f},
                            (Vector3){sys->world.creatures->data[j].position.x + 0.5f, sys->world.creatures->data[j].position.y + 0.5f, sys->world.creatures->data[j].position.z + 0.5f}},
                sys->world.bullets->data[i].position,
                0.1f))
                {
                    // swap the current Creature with the last Creature, then pop the last Creature
                    stack_fast_remove(*sys->world.creatures, j);
                    // we need to check the same index again, so we decrement j
                    j--;

                    // same for the bullet
                    stack_fast_remove(*sys->world.bullets, i);
                    i--;
                }
            }
            
            // Desativar balas muito longe
            if (Vector3Distance(sys->camera.position, sys->world.bullets->data[i].position) > 50) 
            {
                stack_remove(*sys->world.bullets, i);
            }
        }

        BeginDrawing();
            ClearBackground(RAYWHITE);

            BeginMode3D(sys->camera);
                // Chão
                DrawPlane((Vector3){0.0f, 0.0f, 0.0f}, (Vector2){50.0f, 50.0f}, LIGHTGRAY);

                // Inimigos
                for (int i = 0; i < sys->world.creatures->size; i++) 
                {
                    if (i != sys->player_index)
                        DrawCube(sys->world.creatures->data[i].position, 1.0f, 1.0f, 1.0f, sys->world.creatures->data[i].color);
                }

                // Balas
                for (int i = 0; i < sys->world.bullets->size; i++) 
                {
                    DrawSphere(sys->world.bullets->data[i].position, 0.01f, BLACK);
                }

            EndMode3D();

            // Mira
            DrawLine(sys->resolution.x/2 - 10, sys->resolution.y/2, sys->resolution.x/2 + 10, sys->resolution.y/2, GREEN);
            DrawLine(sys->resolution.x/2, sys->resolution.y/2 - 10, sys->resolution.x/2, sys->resolution.y/2 + 10, GREEN);

            DrawTexture(handtexture, sys->resolution.x - handtexture.width + 70, sys->resolution.y - handtexture.height+25, WHITE);

            //DrawText(TextFormat("Inimigos restantes: %d", enemies->size), 10, 10, 20, DARKGRAY);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}