


#include <brutopolis.h>

enum 
{
    ITEM_HAND,
    ITEM_REVOLVER,
    ITEM_BULLET_REVOLVER
} items;

const char* item_names[] = 
{
    "hand",
    "revolver",
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

int main(int argc, char* argv[])
{
    VirtualMachine* vm = make_vm();
    init_std(vm);
    
}