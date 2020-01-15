#include "simple_common.h"

int
main()
{
    hg_class_t *class;
    hg_context_t *context;
    hg_id_t rpc_id;

    simple_hg_init(&class, &context, "tcp://127.0.0.1:1234", HG_TRUE);

    simple_rpc_register(class);

    simple_hg_wait(context);

    simple_hg_destroy(class, context);
    return 0;
}
