//
// Created by Karpukhin Aleksandr on 25.02.2022.
//

#include <stdlib.h>
#include <assert.h>

#include "destruction.h"
#include "defines.h"
#include "error.h"

static dtor_t* dtor_pool = NULL;
static ssize_t next_pos = 0;         // position for next new dtor to add
static ssize_t last_free_pos = -1;   // position of last deleted dtor (can be reused in next add_dtor() call)
static ssize_t dtors_capacity = 0;   // current dtor pool capacity (size of memory allocated)

dtor_id_t add_dtor(const dtor_t* dtor) {
    assert(dtor != NULL);

    dtor_id_t dtor_id;

    if (last_free_pos != -1) {
        dtor_pool[last_free_pos] = *dtor;
        dtor_id = last_free_pos;
        last_free_pos = -1;
    } else {
        if (next_pos == dtors_capacity) {
            dtors_capacity += POOL_CHUNK_ALLOC;
            if ((dtor_pool = (dtor_t *) realloc(dtor_pool, sizeof(dtor_t) * dtors_capacity)) == NULL) {
                exit_with_error_message(ERR_NOT_ALLOCATED, "can't reallocate memory for dtor pool");
            }
        }

        dtor_pool[next_pos] = *dtor;
        dtor_id = next_pos++;
    }

    return dtor_id;
}

void delete_dtor(dtor_id_t dtor_id) {
    assert(0 <= dtor_id && dtor_id < next_pos);
    last_free_pos = dtor_id;
}

void exec_all_dtors(void) {
    if (dtor_pool == NULL) return;

    for (ssize_t i = 0; i < next_pos && i != last_free_pos; i++) {
        switch (dtor_pool[i].type) {
            case VOID_DTOR:
                dtor_pool[i].void_dtor.call();
                break;
            case INT_DTOR:
                dtor_pool[i].int_dtor.call(dtor_pool[i].int_dtor.arg);
                break;
            case REGEX_DTOR:
                dtor_pool[i].regex_dtor.call(dtor_pool[i].regex_dtor.regexes, dtor_pool[i].regex_dtor.cnt);
                break;
        }
    }

    free(dtor_pool);
}

