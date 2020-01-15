#include "simple_common.h"

struct lookup_cb_data {
    hg_int32_t rpc_arg;
    hg_id_t rpc_id;
    hg_context_t *context;
};

static hg_return_t
simple_rpc_cb(const struct hg_cb_info *rpc_info)
{
    hg_return_t ret = HG_SUCCESS;
    hg_int32_t output;
    struct lookup_cb_data *dat = (struct lookup_cb_data *) rpc_info->arg;

    ret = HG_Get_output(rpc_info->info.forward.handle, &output);
    if (ret != 0)
        goto done;

    printf(" * Result: %d\n", output);

done:
    HG_Free_output(rpc_info->info.forward.handle, &output);
    free(dat);
    return ret;
}

static hg_return_t
lookup_cb(const struct hg_cb_info *addr_info)
{
    hg_return_t ret = HG_SUCCESS;
    hg_handle_t handle;
    struct lookup_cb_data *dat = (struct lookup_cb_data *) addr_info->arg;

    if (addr_info->ret != 0) {
        fprintf(stderr, " ! Failed to lookup address\n");
        ret = 1;
        goto done;
    }

    printf(" * Sending RPC...\n");

    ret = HG_Create(dat->context, addr_info->info.lookup.addr, dat->rpc_id,
                    &handle);
    if (ret != HG_SUCCESS)
        goto done;

    ret = HG_Forward(handle, simple_rpc_cb, dat, &dat->rpc_arg);
    if (ret != HG_SUCCESS)
        goto done;

done:
    return ret;
}

static hg_return_t
simple_rpc_call(hg_context_t *context, hg_id_t id, hg_int32_t arg)
{
    hg_return_t ret;
    struct lookup_cb_data *dat = malloc(sizeof(struct lookup_cb_data));
    dat->rpc_arg = arg;
    dat->rpc_id = id;
    dat->context = context;

    printf(" * Looking up address...\n");

    ret = HG_Addr_lookup(context, lookup_cb, dat, "tcp://127.0.0.1:1234",
                         HG_OP_ID_IGNORE);
    if (ret != HG_SUCCESS) {
        free(dat);
        return ret;
    }

    return HG_SUCCESS;
}

int
main()
{
    hg_class_t *class;
    hg_context_t *context;
    hg_id_t rpc_id;

    simple_hg_init(&class, &context, "tcp", HG_FALSE);

    rpc_id = simple_rpc_register(class);
    simple_rpc_call(context, rpc_id, 2);

    simple_hg_wait(context);

    simple_hg_destroy(class, context);
    return 0;
}
