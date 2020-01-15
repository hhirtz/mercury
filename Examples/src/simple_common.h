#pragma once

#include <mercury_bulk.h>
#include <mercury.h>
#include <mercury_macros.h>
#include <mercury_bulk.h>

int simple_hg_init(hg_class_t **c, hg_context_t **context,
                   const char *info_string, hg_bool_t listen);
void simple_hg_destroy(hg_class_t *c, hg_context_t *context);

void simple_hg_wait(hg_context_t *context);

hg_id_t simple_rpc_register(hg_class_t *c);
