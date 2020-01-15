#include "simple_common.h"

int
simple_hg_init(hg_class_t **c, hg_context_t **context,
               const char *info_string, hg_bool_t listen)
{
    *c = HG_Init(info_string, listen);
    if (!c)
        return -1;

    *context = HG_Context_create(*c);
    if (!*context) {
        HG_Finalize(*c);
        return -1;
    }

    return 0;
}

void
simple_hg_destroy(hg_class_t *c, hg_context_t *context)
{
    HG_Context_destroy(context);
    HG_Finalize(c);
}

void
simple_hg_wait(hg_context_t *context)
{
    for (;;) {
        unsigned int unsigned_int;
        hg_return_t ret;

        do {
            ret = HG_Trigger(context, 0, 1, &unsigned_int);
        } while (ret == HG_SUCCESS && unsigned_int);

        HG_Progress(context, 100);
    }
}

static hg_return_t
respond_cb(const struct hg_cb_info *info)
{
    (void) info;
    printf(" * Sent result\n");
    return HG_SUCCESS;
}

static hg_return_t
simple_rpc(hg_handle_t handle)
{
    hg_return_t ret;
    hg_int32_t arg;

    if ((ret = HG_Get_input(handle, &arg)) != HG_SUCCESS)
        return ret;

    printf(" * call to simple_rpc(%d)\n", arg);

    arg++;
    if ((ret = HG_Respond(handle, respond_cb, 0, &arg)) != HG_SUCCESS)
        return ret;

    return HG_SUCCESS;
}

static hg_return_t
ser_args(hg_proc_t proc, void *data)
{
    return hg_proc_int32_t(proc, (hg_int32_t *) data);
}

static hg_return_t
deser_args(hg_proc_t proc, void *data)
{
    return hg_proc_int32_t(proc, (hg_int32_t *) data);
}

hg_id_t
simple_rpc_register(hg_class_t *c)
{
    return HG_Register_name(c, "simple_rpc", ser_args, deser_args, simple_rpc);
}
