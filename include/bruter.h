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


#define VERSION "0.7.5c"

#define TYPE_ANY 0
#define TYPE_NUMBER 1
#define TYPE_STRING 2
#define TYPE_LIST 3

// we use Int and Float instead of int and float because we need to use always the pointer size for any type that might share the fundamental union type;
// bruter use a union as universal type, and bruter is able to manipulate and use pointers direcly so we need to use the pointer size;
#if __SIZEOF_POINTER__ == 8
    #define Int long
    #define Float double
#else
    #define Int int
    #define Float float
#endif

// c_list.h must be included after defining Int, because it also relies on it and will define it as a int(4byte) if it's not defined, instead of the pointer size(which might usually be 8 bytes);
#include "c_list.h"

#ifndef NULL
#define NULL 0
#endif

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
typedef List(Value) ValueList;
typedef List(Hash) HashList;
typedef List(char*) StringList;
typedef List(Int) IntList;
typedef List(char) CharList;

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

// <libraries header>
void init_std_type(VirtualMachine* vm);
void init_std_math(VirtualMachine* vm);
void init_std_condition(VirtualMachine* vm);
void init_std(VirtualMachine* vm);

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