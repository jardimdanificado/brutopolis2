// bruter 0.7.5b

// type agnostic,
// header only,
// easy to use,
// c list implementation
// by @jardimdanificado
// c_list.h

// example usage:
/*
    List(Int) *list = list_init(List(Int));
    list_push(*list, 1);
    list_unshift(*list, 2);
    list_insert(*list, 1, 3);
    list_remove(*list, 1);
    list_pop(*list);
    list_shift(*list);
    Int index = list_find(*list, 2);
    list_reverse(*list);
    list_set(*list, 0, 4);
    Int value = list_get(*list, 0);
    list_half(*list);
    list_double(*list);
    list_free(*list);
*/

// you might want to define Int before including this file if you want smaller or bigger lists;
#ifndef Int 
#define Int int
#endif

#ifndef C_LIST_H

#define List(T) struct \
{ \
    T *data; \
    Int size; \
    Int capacity; \
}

// malloc and initialize a new list
#define list_init(type) ({ \
    type *list = (type*)malloc(sizeof(type)); \
    list->data = NULL; \
    list->size = 0; \
    list->capacity = 0; \
    list; \
})

// increase the capacity of the stack
#define list_double(s) do { \
    (s).capacity = (s).capacity == 0 ? 1 : (s).capacity * 2; \
    (s).data = realloc((s).data, (s).capacity * sizeof(*(s).data)); \
} while (0)

// decrease the capacity of the stack
#define list_half(s) do { \
    (s).capacity /= 2; \
    (s).data = realloc((s).data, (s).capacity * sizeof(*(s).data)); \
    if ((s).size > (s).capacity) { \
        (s).size = (s).capacity; \
    } \
} while (0)

#define list_push(s, v) do { \
    if ((s).size == (s).capacity) { \
        list_double(s); \
    } \
    (s).data[(s).size++] = (v); \
} while (0)

#define list_unshift(s, v) do { \
    if ((s).size == (s).capacity) { \
        list_double(s); \
    } \
    for (Int i = (s).size; i > 0; i--) { \
        (s).data[i] = (s).data[i - 1]; \
    } \
    (s).data[0] = (v); \
    (s).size++; \
} while (0)

#define list_pop(s) ((s).data[--(s).size])

#define list_shift(s) ({ \
    typeof((s).data[0]) ret = (s).data[0]; \
    for (Int i = 0; i < (s).size - 1; i++) { \
        (s).data[i] = (s).data[i + 1]; \
    } \
    (s).size--; \
    ret; \
})

#define list_free(s) ({free((s).data);free(&s);})

//swap elements from index i1 to index i2
#define list_swap(s, i1, i2) do { \
    typeof((s).data[i1]) tmp = (s).data[i1]; \
    (s).data[i1] = (s).data[i2]; \
    (s).data[i2] = tmp; \
} while (0)

//insert element v at index i
#define list_insert(s, i, v) do { \
    if ((s).size == (s).capacity) { \
        list_double(s); \
    } \
    for (Int j = (s).size; j > i; j--) { \
        (s).data[j] = (s).data[j - 1]; \
    } \
    (s).data[i] = (v); \
    (s).size++; \
} while (0)

//remove element at index i and return it
#define list_remove(s, i) ({ \
    typeof((s).data[i]) ret = (s).data[i]; \
    for (Int j = i; j < (s).size - 1; j++) { \
        (s).data[j] = (s).data[j + 1]; \
    } \
    (s).size--; \
    ret; \
})

//same as remove but does a swap and pop, faster but the order of the elements will change
#define list_fast_remove(s, i) ({ \
    typeof((s).data[i]) ret = (s).data[i]; \
    list_swap(s, i, (s).size - 1); \
    list_pop(s); \
    ret; \
})

#define list_find(s, v) ({ \
    Int i = 0; \
    while (i < (s).size && (s).data[i] != (v)) { \
        i++; \
    } \
    i == (s).size ? -1 : i; \
})

#define list_reverse(s) do { \
    for (Int i = 0; i < (s).size / 2; i++) { \
        list_swap((s), i, (s).size - i - 1); \
    } \
} while (0)

#define list_set(s, i, v) ((s).data[i] = (v))

#define list_get(s, i) ((s).data[i])

#endif