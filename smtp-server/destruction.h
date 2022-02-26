//
// Created by Karpukhin Aleksandr on 25.02.2022.
//
// Globally used resources' destruction pool.
// Store destructors for each global resource in program to
// guarantee its correct releasing in case of exit() call with
// error code (exit() must be called by program itself).
// This doesn't work in case of unexpected errors, such as segmentation fault,
// because it can't be caught correctly by execution flow.


#ifndef NETWORK_PROTOCOLS_CP_DESTRUCTION_H
#define NETWORK_PROTOCOLS_CP_DESTRUCTION_H

typedef size_t dtor_id_t;

typedef struct {
    void (*call)(void);
} void_dtor_t;

typedef struct {
    void (*call)(int);
    int arg;
} int_dtor_t;

typedef enum {
    VOID_DTOR,
    INT_DTOR
} dtor_te;

typedef struct {
    union {
        void_dtor_t void_dtor;
        int_dtor_t int_dtor;
    };
    dtor_te type;
} dtor_t;

dtor_id_t add_dtor(const dtor_t* dtor);

void delete_dtor(dtor_id_t dtor_id);

void exec_all_dtors(void);

#endif //NETWORK_PROTOCOLS_CP_DESTRUCTION_H
