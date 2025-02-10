#ifndef BRUTER_H
#define BRUTER_H 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#define VERSION "0.7.5b"

#define TYPE_ANY 0
#define TYPE_NUMBER 1
#define TYPE_STRING 2
#define TYPE_LIST 3
#define TYPE_BUILTIN 4
#define TYPE_FUNCTION 7

#if __SIZEOF_POINTER__ == 8
    #define Int long
    #define Float double
#else
    #define Int int
    #define Float float
#endif

#ifndef NULL
#define NULL 0
#endif


//stack implementation
#define Stack(T) struct \
{ \
    T *data; \
    Int size; \
    Int capacity; \
}

#define stack_init(s) do \
{ \
    (s).data = NULL; \
    (s).size = 0; \
    (s).capacity = 0;\
} while (0)

// increase the capacity of the stack
#define stack_double(s) do { \
    (s).capacity = (s).capacity == 0 ? 1 : (s).capacity * 2; \
    (s).data = realloc((s).data, (s).capacity * sizeof(*(s).data)); \
} while (0)

// decrease the capacity of the stack
#define stack_half(s) do { \
    (s).capacity /= 2; \
    (s).data = realloc((s).data, (s).capacity * sizeof(*(s).data)); \
    if ((s).size > (s).capacity) { \
        (s).size = (s).capacity; \
    } \
} while (0)

#define stack_push(s, v) do { \
    if ((s).size == (s).capacity) { \
        stack_double(s); \
    } \
    (s).data[(s).size++] = (v); \
} while (0)

#define stack_unshift(s, v) do { \
    if ((s).size == (s).capacity) { \
        stack_double(s); \
    } \
    for (Int i = (s).size; i > 0; i--) { \
        (s).data[i] = (s).data[i - 1]; \
    } \
    (s).data[0] = (v); \
    (s).size++; \
} while (0)

#define stack_pop(s) ((s).data[--(s).size])

#define stack_shift(s) ({ \
    typeof((s).data[0]) ret = (s).data[0]; \
    for (Int i = 0; i < (s).size - 1; i++) { \
        (s).data[i] = (s).data[i + 1]; \
    } \
    (s).size--; \
    ret; \
})

#define stack_free(s) ({free((s).data);free(&s);})

//swap elements from index i1 to index i2
#define stack_swap(s, i1, i2) do { \
    typeof((s).data[i1]) tmp = (s).data[i1]; \
    (s).data[i1] = (s).data[i2]; \
    (s).data[i2] = tmp; \
} while (0)

//insert element v at index i
#define stack_insert(s, i, v) do { \
    if ((s).size == (s).capacity) { \
        stack_double(s); \
    } \
    for (Int j = (s).size; j > i; j--) { \
        (s).data[j] = (s).data[j - 1]; \
    } \
    (s).data[i] = (v); \
    (s).size++; \
} while (0)

//remove element at index i and return it
#define stack_remove(s, i) ({ \
    typeof((s).data[i]) ret = (s).data[i]; \
    for (Int j = i; j < (s).size - 1; j++) { \
        (s).data[j] = (s).data[j + 1]; \
    } \
    (s).size--; \
    ret; \
})

//same as remove but does a swap and pop, faster but the order of the elements will change
#define stack_fast_remove(s, i) ({ \
    typeof((s).data[i]) ret = (s).data[i]; \
    stack_swap(s, i, (s).size - 1); \
    stack_pop(s); \
    ret; \
})

#define stack_find(s, v) ({ \
    Int i = 0; \
    while (i < (s).size && (s).data[i] != (v)) { \
        i++; \
    } \
    i == (s).size ? -1 : i; \
})

#define stack_reverse(s) do { \
    for (Int i = 0; i < (s).size / 2; i++) { \
        stack_swap((s), i, (s).size - i - 1); \
    } \
} while (0)

//Value
typedef union 
{
    Float number;
    Int integer;
    char* string;
    void* pointer;
    char byte[sizeof(Float)];
} Value;

//Hash
typedef struct
{
    char *key;
    Int index;
} Hash;

//List
typedef Stack(Value) ValueList;
typedef Stack(Hash) HashList;
typedef Stack(char*) StringList;
typedef Stack(Int) IntList;
typedef Stack(char) CharList;


typedef struct
{
    StringList *varnames;
    char *code;
} InternalFunction;

typedef struct
{
    ValueList *stack;
    CharList *typestack;
    HashList *hashes;
    IntList *unused;
    Int (*interpret)(void*, char*, HashList*);
} VirtualMachine;

//Function
typedef Int (*Function)(VirtualMachine*, IntList*, HashList*);

//String
extern char* str_duplicate(const char *str);
extern char* str_nduplicate(const char *str, Int n);
extern char* str_format(const char *fmt, ...);
extern char* str_sub(const char *str, Int start, Int end);
extern char* str_concat(const char *str1, const char *str2);
extern Int str_find(const char *str, const char *substr);
extern char* str_replace(const char *str, const char *substr, const char *replacement);
extern char* str_replace_all(const char *str, const char *substr, const char *replacement);

extern StringList* str_split(char *str, char *delim);
extern StringList* str_split_char(char *str, char delim);
extern StringList* special_space_split(char *str);
extern StringList* special_split(char *str, char delim);

#define is_true(value, __type) (__type == value.integer == 0 ? 0 : 1)

// #define is_space(c) (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f')

// variable

extern ValueList* make_value_list();
extern IntList* make_int_list();
extern StringList* make_string_list();
extern CharList* make_char_list();

extern VirtualMachine* make_vm();
extern void free_vm(VirtualMachine *vm);
extern void unuse_var(VirtualMachine *vm, Int index);

extern Int new_number(VirtualMachine *vm, Float number);
extern Int new_string(VirtualMachine *vm, char *str);
extern Int new_builtin(VirtualMachine *vm, Function function);
extern Int new_var(VirtualMachine *vm);
extern Int new_list(VirtualMachine *vm);

extern Value value_duplicate(Value value, char type);

extern Int register_var(VirtualMachine *vm, char* varname);
extern Int register_string(VirtualMachine *vm, char* varname, char* string);
extern Int register_number(VirtualMachine *vm, char* varname, Float number);
extern Int register_builtin(VirtualMachine *vm, char* varname, Function function);
extern Int register_list(VirtualMachine *vm, char* varname);

extern Int hash_find(VirtualMachine *vm, char *key);
extern void hash_set(VirtualMachine *vm, char *key, Int index);
extern void hash_unset(VirtualMachine *vm, char *key);

// eval
// pass NULL as context if you don't want to use a local context
extern Int eval(VirtualMachine *vm, char *cmd, HashList *context);

extern void print_element(VirtualMachine *vm, Int index);


// macros
#define data(index) (vm->stack->data[index])
#define data_t(index) (vm->typestack->data[index])
#define data_unused(index) (vm->unused->data[index])
#define data_temp(index) (vm->temp->data[index])

#define hash(index) (vm->hashes->data[index])

#define arg(index) (vm->stack->data[args->data[index]])
#define arg_i(index) (args->data[index])
#define arg_t(index) (vm->typestack->data[args->data[index]])

#define function(name) Int name(VirtualMachine *vm, IntList *args, HashList *context)
#define init(name) void init_##name(VirtualMachine *vm)

Int interpret(VirtualMachine *vm, IntList *args, HashList *context);

// functions
IntList* parse(void* _vm, char* cmd, HashList* context);

// stringify funcstion
char* list_stringify(VirtualMachine* vm, IntList *list);
char* function_stringify(VirtualMachine* vm, InternalFunction *func);

// <libraries header>

#ifndef ARDUINO

extern char* readfile(char *filename);
extern void writefile(char *filename, char *content);
extern Int repl(VirtualMachine *vm);

#endif

#ifdef __EMSCRIPTEN__

//declarations
Int wasm_new_vm();
void wasm_destroy_vm(Int index);
char* wasm_eval(Int index, char *cmd);
#endif

#endif