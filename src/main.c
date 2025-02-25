#include "raylib.h"
#include "raymath.h"
#include <math.h>
#include "bruter.h"

#define MAX_ENEMIES 10
#define BULLET_SPEED 1.0f

enum 
{
    ITEM_HAND,
    ITEM_REVOLVER,
    ITEM_BULLET_REVOLVER
} items;

const char* item_names[] = 
{
    "hand",
    "38tão enferrujado",
    "revolver bullets"
};

const char* equip_image_paths[] = 
{
    "data/img/equip_hand.png",
    "data/img/equip_revolver.png",
    "data/img/equip_bullet_revolver.png"
};

const char* item_image_paths[] = 
{
    "data/img/item_hand.png",
    "data/img/item_revolver.png",
    "data/img/item_bullet_revolver.png"
};

const int item_capacities[] = 
{
    0,
    6,
    12
};

// CREATURE DEFINES
#define CREATURE_IDLE 0
#define CREATURE_RELOAD 1
#define CREATURE_SHOOT 2

typedef struct 
{
    Vector3 position;
    char type; 
    Int capacity; // if storage
    Int content_type;
    Int content;
} Item;
typedef List(Item) ItemList;

typedef struct 
{
    char* name;
    Vector3 position;
    Vector3 size;
    Vector3 rotation;
    Vector3 direction;
    Color color;
    Float speed;
    ItemList *inventory; 
    Int current_item;
    Int status;
    Int aceleration;
} Creature;
typedef List(Creature) CreatureList;

typedef struct 
{
    Vector3 position;
    Vector3 direction;
    Float speed;
} Bullet;
typedef List(Bullet) BulletList;

typedef struct 
{
    CreatureList *creatures;
    BulletList *bullets;
    ItemList *items;
} World;

typedef struct 
{
    Vector2 delta; // where applies
    Vector2 position; // where applies
    float sensibility;
} Mouse;

typedef List(BoundingBox) BoundingBoxList;



typedef List(Texture2D) TextureList;
typedef List(Model) ModelList;

typedef struct
{
    Int model_id;
    BoundingBoxList *hitboxes;
    char* name;
} Map;

typedef List(Map) MapList;


typedef struct 
{
    char* name;
    Int player_index;
    Vector2 resolution;
    World world;
    Camera camera;
    Mouse mouse;
    TextureList *equip_textures;
    TextureList *item_textures;
    ModelList *models;
    MapList *maps;
    Int current_map;
} InternalSystem;

// macro to acess &sys->world.creatures->data[name];
#define creature(name) sys->world.creatures->data[name]

InternalSystem* new_system(char* name, int size_x, int size_y)
{
    InternalSystem* _sys = (InternalSystem*)malloc(sizeof(InternalSystem));
    _sys->player_index = -1;

    _sys->resolution = (Vector2){size_x, size_y};

    _sys->name = str_duplicate(name);

    _sys->world.creatures = list_init(CreatureList);

    _sys->world.bullets = list_init(BulletList);

    _sys->equip_textures = list_init(TextureList);

    _sys->item_textures = list_init(TextureList);

    _sys->models = list_init(ModelList);

    _sys->maps = list_init(MapList);

    _sys->current_map = 0;

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

    return _sys;
}

void free_system(InternalSystem* _sys)
{
    free(_sys->name);
    list_free(*_sys->world.creatures);
    list_free(*_sys->world.bullets);
    free(_sys);
}

void system_startup(InternalSystem* _sys)
{
    InitWindow(_sys->resolution.x, _sys->resolution.y, _sys->name);
    SetTargetFPS(60);
    DisableCursor();
}

Int new_creature(InternalSystem* _sys, char* name, int x, int y, int z)
{
    Creature* creature = (Creature*)malloc(sizeof(Creature));
    creature->position = (Vector3){ x, y, z };
    creature->size = (Vector3){ 1.0f, 1.70f, 1.0f };
    creature->current_item = 0;
    creature->color = RED;
    creature->speed = 0.1f;
    creature->status = 0;
    
    creature->inventory = (ItemList*)malloc(sizeof(ItemList));

    creature->name = str_duplicate(name);
    creature->direction = (Vector3){0,0,0};
    creature->rotation = (Vector3){0,0,0};

    creature->aceleration = 1;

    creature->inventory = list_init(ItemList);
    
    list_push(*_sys->world.creatures, *creature);
    Int id = _sys->world.creatures->size - 1;
    return id;
}

Bullet* new_bullet(InternalSystem* _sys, Vector3 position, Vector3 direction, Float speed)
{
    Bullet *bullet = (Bullet*)malloc(sizeof(Bullet));
    bullet->position = position;
    bullet->direction = direction;
    bullet->speed = speed;
    list_push(*_sys->world.bullets, *bullet);
    return bullet;
}

void load_texture(InternalSystem* _sys, char* path, bool is_equip)
{
    Image handimg = LoadImage(path);
    ImageResize(&handimg, 450, 300);
    //ImageRotate(&handimg, 25.0f);
    Texture2D texture = LoadTextureFromImage(handimg);      // Image converted to texture, uploaded to GPU memory (VRAM)
    UnloadImage(handimg);   // Once image has been converted to texture and uploaded to VRAM, it can be unloaded from RAM

    if (is_equip)
        list_push(*_sys->equip_textures, texture);
    else
        list_push(*_sys->item_textures, texture);
}

Int load_model(InternalSystem* _sys, char* path)
{
    Model model = LoadModel(path);
    list_push(*_sys->models, model);
    return _sys->models->size - 1;
}

Item* new_item(char* name, char type, int capacity, int content_type, int content)
{
    Item* item = (Item*)malloc(sizeof(Item));
    item->type = type;
    item->capacity = capacity;
    item->content_type = content_type;
    item->content = content;
    return item;
}

void reload_item(InternalSystem* sys, Creature* creature)
{
    switch (creature->inventory->data[creature->current_item].type)
    {
        case ITEM_REVOLVER:
            if (creature->inventory->data[creature->current_item].content == creature->inventory->data[creature->current_item].capacity)
                return;
            
            for (int i = 0; i < creature->inventory->size; i++)
            {
                if (creature->inventory->data[i].type == ITEM_BULLET_REVOLVER)
                {
                    int needed = creature->inventory->data[creature->current_item].capacity - creature->inventory->data[creature->current_item].content;
                    if (creature->inventory->data[i].content >= needed)
                    {
                        creature->inventory->data[creature->current_item].content += needed;
                        creature->inventory->data[i].content -= needed;
                        if (creature->inventory->data[i].content == 0)
                            list_fast_remove(*creature->inventory, i);// remove the empty item
                        break;
                    }
                    else
                    {
                        creature->inventory->data[creature->current_item].content += creature->inventory->data[i].content;
                        creature->inventory->data[i].content = 0;
                        list_fast_remove(*creature->inventory, i);// remove the empty item
                        i--;// we need to check the same index again
                    }
                }
            }
            break;
        default:
            // need a message system to show messages to the player
            // "can't reload this item"
            break;
    }
}

void use_item(InternalSystem* sys, Creature* creature)
{
    switch (creature->inventory->data[creature->current_item].type)
    {
    case ITEM_HAND:
        break;
    case ITEM_REVOLVER:

        if (creature->inventory->data[creature->current_item].content == 0)
        {
            // need a message system to show messages to the player
            // "no bullets"
            // try to reload
            reload_item(sys,creature);
            return;
        }

        creature->inventory->data[creature->current_item].content--;

        Bullet bullet = {0};
        bullet.position = (Vector3){creature->position.x, creature->position.y + 1.7f, creature->position.z};
        bullet.direction = (Vector3){creature->direction.x, creature->direction.y, creature->direction.z};
        bullet.speed = BULLET_SPEED;
        list_push(*sys->world.bullets, bullet);
        break;
    case ITEM_BULLET_REVOLVER:
        break;
    default:
        break;
    }
}

void new_map(InternalSystem* sys, char* name, int model_id)
{
    Map map = {0};
    map.name = str_duplicate(name);
    map.model_id = model_id;
    map.hitboxes = list_init(BoundingBoxList);
    // mesh 0 is the map, all other meshes are hitboxes
    for (int i = 1; i < sys->models->data[model_id].meshCount; i++)
    {
        BoundingBox box = GetMeshBoundingBox(sys->models->data[model_id].meshes[i]);
        list_push(*map.hitboxes, box);
    }
    list_push(*sys->maps, map);
}

bool check_move_collision(InternalSystem* sys, Vector3 position, Vector3 move, float size)
{
    bool collision = false;
    for (int i = 0; i < sys->maps->data[sys->current_map].hitboxes->size; i++)
    {
        if (CheckCollisionBoxSphere(sys->maps->data[sys->current_map].hitboxes->data[i], Vector3Add(position, move), size) == 1)
        {
            collision = true;
            break;
        }
    }
    return collision;
}

int main(void)
{
    InternalSystem* sys = new_system("brutopolis 2 - o auto do céu", 800, 450);

    system_startup(sys);

    load_texture(sys, "data/img/item_hand.png", false);
    load_texture(sys, "data/img/equip_hand.png", true);
    load_texture(sys, "data/img/item_revolver.png", false);
    load_texture(sys, "data/img/equip_revolver.png", true);
    load_texture(sys, "data/img/item_bullet_revolver.png", false);
    load_texture(sys, "data/img/equip_bullet_revolver.png", true);
    load_model(sys, "data/model/base.obj");

    new_map(sys, "test map", load_model(sys, "data/model/map0/map.obj"));

    /*Texture2D creaturetexture = LoadTexture("data/img/creature.png");

    Rectangle source = { 0.0f, 0.0f, (float)creaturetexture.width, (float)creaturetexture.height };

    // NOTE: Billboard locked on axis-Y
    Vector3 billUp = { 0.0f, 1.0f, 0.0f };

    // Set the height of the rotating billboard to 1.0 with the aspect ratio fixed
    Vector2 size = { source.width/source.height, 1.0f };*/

    Int player_id = new_creature(sys, "joao451", 4*2-2, 10, 4*12-2);
    Item* hand = new_item("hand", ITEM_HAND, 0, 0, 0);
    Item* revolver = new_item("revolver", ITEM_REVOLVER, item_capacities[ITEM_REVOLVER], ITEM_BULLET_REVOLVER, 6);
    Item* bullet_revolver = new_item("buller_revolver", ITEM_BULLET_REVOLVER, item_capacities[ITEM_BULLET_REVOLVER], 0, item_capacities[ITEM_BULLET_REVOLVER]);
    sys->player_index = 0;

    list_push(*creature(player_id).inventory, *hand);
    list_push(*creature(player_id).inventory, *revolver);
    list_push(*creature(player_id).inventory, *bullet_revolver);
    
    for (int i = 0;i < GetRandomValue(2,100);i++)
    {
        char* _name = TextFormat("joao%d", i);
        new_creature(sys, _name, GetRandomValue(-20,20), 15, GetRandomValue(-20,20));
        // lets set a random rotation
        sys->world.creatures->data[sys->world.creatures->size-1].rotation = (Vector3){0,GetRandomValue(-180,180),0};

    }


    while (!WindowShouldClose())
    {
        // Atualização da câmera
        // Rotação com mouse
        sys->mouse.delta = GetMouseDelta();
        creature(player_id).rotation.x += sys->mouse.delta.x * sys->mouse.sensibility;
        creature(player_id).rotation.y -= sys->mouse.delta.y * sys->mouse.sensibility;

        // Limitar rotação vertical
        creature(player_id).rotation.y = Clamp(creature(player_id).rotation.y, -PI/2.5f, PI/2.5f);

        // Calcular direção da câmera
        creature(player_id).direction = (Vector3)
        {
            cosf(creature(player_id).rotation.x) * cosf(creature(player_id).rotation.y),
            sinf(creature(player_id).rotation.y),
            sinf(creature(player_id).rotation.x) * cosf(creature(player_id).rotation.y)
        };
        creature(player_id).direction = Vector3Normalize(creature(player_id).direction);
        sys->camera.target = Vector3Add(sys->camera.position, creature(player_id).direction);

        // Movimento do jogador
        Vector3 move = {0};
        if (IsKeyDown(KEY_W)) move.x = 1;
        if (IsKeyDown(KEY_S)) move.x = -1;
        if (IsKeyDown(KEY_A)) move.z = -1;
        if (IsKeyDown(KEY_D)) move.z = 1;
        // mouse wheel to scroll through items
        int mouse_wheel = GetMouseWheelMove();
        if (mouse_wheel != 0)
        {
            creature(player_id).current_item += mouse_wheel;
            creature(player_id).current_item = Clamp(creature(player_id).current_item, 0, creature(player_id).inventory->size - 1);
        }

        // Rotacionar movimento de acordo com a direção da câmera
        Vector3 rotatedMove = 
        {
            move.x * cosf(creature(player_id).rotation.x) - move.z * sinf(creature(player_id).rotation.x),
            0.0f,
            move.x * sinf(creature(player_id).rotation.x) + move.z * cosf(creature(player_id).rotation.x)
        };

        if(check_move_collision(sys, creature(player_id).position, rotatedMove, 0.1) == 0)
            creature(player_id).position = Vector3Add(creature(player_id).position, Vector3Scale(rotatedMove, creature(player_id).speed));

        sys->camera.position = Vector3Add(
            creature(player_id).position,
            (Vector3){0.0f, 1.72f, 0.0f}
        );

        // Atirar
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) 
        {
            use_item(sys, &creature(player_id));
        }
        else if (IsKeyDown(KEY_R))
        {
            reload_item(sys, &creature(player_id));
        }

        // Atualizar balas
        for (int i = 0; i < sys->world.bullets->size; i++)
        {
            sys->world.bullets->data[i].position = Vector3Add(sys->world.bullets->data[i].position, Vector3Scale(sys->world.bullets->data[i].direction, sys->world.bullets->data[i].speed));
            
            // Verificar colisão com inimigos
            for (int j = 0; j < sys->world.creatures->size; j++) 
            {
                if (CheckCollisionBoxSphere(
                (BoundingBox){(Vector3){sys->world.creatures->data[j].position.x - 0.4f, sys->world.creatures->data[j].position.y, sys->world.creatures->data[j].position.z - 0.5f},
                            (Vector3){sys->world.creatures->data[j].position.x + 0.4f, sys->world.creatures->data[j].position.y + 1.93, sys->world.creatures->data[j].position.z + 0.5f}},
                sys->world.bullets->data[i].position,
                0.1f))
                {
                    // swap the current Creature with the last Creature, then pop the last Creature
                    list_fast_remove(*sys->world.creatures, j);
                    // we need to check the same index again, so we decrement j
                    j--;

                    // same for the bullet
                    list_fast_remove(*sys->world.bullets, i);
                    i--;
                }
            }
            
            // Desativar balas muito longe
            if (Vector3Distance(sys->camera.position, sys->world.bullets->data[i].position) > 50) 
            {
                list_fast_remove(*sys->world.bullets, i);
            }
        }

        // gravity
        for (int i = 0; i < sys->world.creatures->size; i++)
        {
            bool collision = false;
            int j = 0;
            // check hitboxes
            for (j = 0; j < sys->maps->data[sys->current_map].hitboxes->size; j++)
            {
                if (check_move_collision(sys, creature(i).position, (Vector3){0,-0.1,0}, 0.1))
                {

                    collision = true;
                    break;
                }
            }

            if (!collision)
            {
                creature(i).position.y -= 0.1f;
            }
            else 
            {
                creature(i).position.y = sys->maps->data[sys->current_map].hitboxes->data[j].max.y + 0.2;
            }
        }

        BeginDrawing();
            ClearBackground(BLACK);

            BeginMode3D(sys->camera);
                //DrawModel(sys->models->data[1], (Vector3){0,0,0}, 1.0f, WHITE);

                // draw map (mesh, material, Matrix)
                DrawMesh(sys->models->data[sys->maps->data[sys->current_map].model_id].meshes[0], sys->models->data[sys->maps->data[sys->current_map].model_id].materials[0], MatrixIdentity());

                for (int i = 0; i < sys->world.creatures->size; i++) 
                {
                    // size 1x1.7x1
                    if (i != sys->player_index)
                        DrawModelEx(sys->models->data[0], (Vector3){sys->world.creatures->data[i].position.x, sys->world.creatures->data[i].position.y, sys->world.creatures->data[i].position.z}, (Vector3){0,1,0}, sys->world.creatures->data[i].rotation.y, (Vector3){1,1,1}, RED);
                        //DrawBillboardPro(sys->camera, creaturetexture, source, (Vector3){sys->world.creatures->data[i].position.x, sys->world.creatures->data[i].position.y + 0.85f, sys->world.creatures->data[i].position.z}, billUp, (Vector2){1.0f, 1.7f}, (Vector2){0.5f, 0.5f}, 0, WHITE);
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

            DrawTexture(sys->equip_textures->data[creature(player_id).current_item],
            sys->resolution.x - sys->equip_textures->data[creature(player_id).current_item].width + 70, sys->resolution.y - sys->equip_textures->data[creature(player_id).current_item].height+25, WHITE);

            DrawText(TextFormat("%s %d/%d", item_names[creature(player_id).inventory->data[creature(player_id).current_item].type], creature(player_id).inventory->data[creature(player_id).current_item].content, creature(player_id).inventory->data[creature(player_id).current_item].capacity), 10, 10, 20, DARKGRAY);
            //DrawText(TextFormat("Inimigos restantes: %d", enemies->size), 10, 10, 20, DARKGRAY);
            DrawText(TextFormat("Player position: %f %f %f", creature(player_id).position.x, creature(player_id).position.y, creature(player_id).position.z), 10, 30, 20, DARKGRAY);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}