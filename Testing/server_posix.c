/*
 * Copyright (C) 2013 Argonne National Laboratory, Department of Energy,
 *                    UChicago Argonne, LLC and The HDF Group.
 * All rights reserved.
 *
 * The full copyright notice, including terms governing use, modification,
 * and redistribution, is contained in the COPYING file that can be
 * found at the root of the source code distribution tree.
 */

#include "test_posix.h"
#include "mercury_test.h"
#include "mercury_handler.h"
#include "mercury_bulk.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

unsigned int finalizing = 0;

int server_finalize(hg_handle_t handle)
{
    int ret = HG_SUCCESS;

    finalizing++;

    /* Free handle and send response back */
    ret = HG_Handler_start_output(handle, NULL);
    if (ret != HG_SUCCESS) {
        fprintf(stderr, "Could not respond\n");
        return ret;
    }

    return ret;
}

int server_posix_open(hg_handle_t handle)
{
    int hg_ret = HG_SUCCESS;

    open_in_t      open_in_struct;
    open_out_t     open_out_struct;

    const char *path;
    int flags;
    mode_t mode;
    int ret;

    /* Get input struct */
    ret = HG_Handler_get_input(handle, &open_in_struct);
    if (ret != HG_SUCCESS) {
        fprintf(stderr, "Could not get input struct\n");
        return ret;
    }

    path = open_in_struct.path;
    flags = open_in_struct.flags;
    mode = open_in_struct.mode;

    /* Call open */
    printf("Calling open with path: %s\n", path);
    ret = open(path, flags, mode);

    /* Fill output structure */
    open_out_struct.ret = ret;

    /* Free handle and send response back */
    ret = HG_Handler_start_output(handle, &open_out_struct);
    if (ret != HG_SUCCESS) {
        fprintf(stderr, "Could not respond\n");
        return ret;
    }

    return hg_ret;
}

int server_posix_close(hg_handle_t handle)
{
    int hg_ret = HG_SUCCESS;

    close_in_t     close_in_struct;
    close_out_t    close_out_struct;

    int fd;
    int ret;

    /* Get input struct */
    ret = HG_Handler_get_input(handle, &close_in_struct);
    if (ret != HG_SUCCESS) {
        fprintf(stderr, "Could not get input struct\n");
        return ret;
    }

    fd = close_in_struct.fd;

    /* Call close */
    printf("Calling close with fd: %d\n", fd);
    ret = close(fd);

    /* Fill output structure */
    close_out_struct.ret = ret;

    /* Free handle and send response back */
    ret = HG_Handler_start_output(handle, &close_out_struct);
    if (ret != HG_SUCCESS) {
        fprintf(stderr, "Could not respond\n");
        return ret;
    }

    return hg_ret;
}

int server_posix_write(hg_handle_t handle)
{
    int hg_ret = HG_SUCCESS;

    write_in_t     write_in_struct;
    write_out_t    write_out_struct;

    na_addr_t source = HG_Handler_get_addr(handle);
    hg_bulk_t bulk_handle = HG_BULK_NULL;
    hg_bulk_block_t bulk_block_handle = HG_BULK_BLOCK_NULL;

    int fd;
    void *buf;
    size_t count;
    ssize_t ret;

    /* for debug */
    int i;
    const int *buf_ptr;

    /* Get input struct */
    ret = HG_Handler_get_input(handle, &write_in_struct);
    if (ret != HG_SUCCESS) {
        fprintf(stderr, "Could not get input struct\n");
        return ret;
    }

    bulk_handle = write_in_struct.bulk_handle;
    fd = write_in_struct.fd;

    /* Read bulk data here and wait for the data to be here  */
    count = HG_Bulk_handle_get_size(bulk_handle);
    buf = malloc(count);

    HG_Bulk_block_handle_create(buf, count, HG_BULK_READWRITE, &bulk_block_handle);

    hg_ret = HG_Bulk_read(bulk_handle, source, bulk_block_handle);
    if (hg_ret != HG_SUCCESS) {
        fprintf(stderr, "Could not read bulk data\n");
        return hg_ret;
    }

    hg_ret = HG_Bulk_wait(bulk_block_handle, HG_BULK_MAX_IDLE_TIME);
    if (hg_ret != HG_SUCCESS) {
        fprintf(stderr, "Could not complete bulk data read\n");
        return hg_ret;
    }

    /* Check bulk buf */
    buf_ptr = buf;
    for (i = 0; i < (count / sizeof(int)); i++) {
        if (buf_ptr[i] != i) {
            printf("Error detected in bulk transfer, buf[%d] = %d, was expecting %d!\n", i, buf_ptr[i], i);
            break;
        }
    }

    printf("Calling write with fd: %d\n", fd);
    ret = write(fd, buf, count);

    /* Fill output structure */
    write_out_struct.ret = ret;

    /* Free handle and send response back */
    ret = HG_Handler_start_output(handle, &write_out_struct);
    if (ret != HG_SUCCESS) {
        fprintf(stderr, "Could not respond\n");
        return ret;
    }

    /* Free block handle */
    hg_ret = HG_Bulk_block_handle_free(bulk_block_handle);
    if (hg_ret != HG_SUCCESS) {
        fprintf(stderr, "Could not free block call\n");
        return hg_ret;
    }

    free(buf);

    return hg_ret;
}

int server_posix_read(hg_handle_t handle)
{
    int hg_ret = HG_SUCCESS;

    read_in_t     read_in_struct;
    read_out_t    read_out_struct;

    na_addr_t dest = HG_Handler_get_addr(handle);
    hg_bulk_t bulk_handle = HG_BULK_NULL;
    hg_bulk_block_t bulk_block_handle = HG_BULK_BLOCK_NULL;

    int fd;
    void *buf;
    size_t count;
    ssize_t ret;

    /* for debug */
    int i;
    const int *buf_ptr;

    /* Get input struct */
    ret = HG_Handler_get_input(handle, &read_in_struct);
    if (ret != HG_SUCCESS) {
        fprintf(stderr, "Could not get input struct\n");
        return ret;
    }

    bulk_handle = read_in_struct.bulk_handle;
    fd = read_in_struct.fd;

    /* Call read */
    count = HG_Bulk_handle_get_size(bulk_handle);
    buf = malloc(count);

    printf("Calling read with fd: %d\n", fd);
    ret = read(fd, buf, count);

    /* Check bulk buf */
    buf_ptr = buf;
    for (i = 0; i < (count / sizeof(int)); i++) {
        if (buf_ptr[i] != i) {
            printf("Error detected after read, buf[%d] = %d, was expecting %d!\n", i, buf_ptr[i], i);
            break;
        }
    }

    /* Create a new block handle to write the data */
    HG_Bulk_block_handle_create(buf, ret, HG_BULK_READ_ONLY, &bulk_block_handle);

    /* Write bulk data here and wait for the data to be there  */
    hg_ret = HG_Bulk_write(bulk_handle, dest, bulk_block_handle);
    if (hg_ret != HG_SUCCESS) {
        fprintf(stderr, "Could not write bulk data\n");
        return hg_ret;
    }

    hg_ret = HG_Bulk_wait(bulk_block_handle, HG_BULK_MAX_IDLE_TIME);
    if (hg_ret != HG_SUCCESS) {
        fprintf(stderr, "Could not complete bulk data write\n");
        return hg_ret;
    }

    /* Fill output structure */
    read_out_struct.ret = ret;

    /* Free handle and send response back */
    ret = HG_Handler_start_output(handle, &read_out_struct);
    if (ret != HG_SUCCESS) {
        fprintf(stderr, "Could not respond\n");
        return ret;
    }

    /* Free block handle */
    hg_ret = HG_Bulk_block_handle_free(bulk_block_handle);
    if (hg_ret != HG_SUCCESS) {
        fprintf(stderr, "Could not free block call\n");
        return hg_ret;
    }

    free(buf);

    return hg_ret;
}

/******************************************************************************/
int main(int argc, char *argv[])
{
    na_class_t *network_class = NULL;
    unsigned int number_of_peers;
    int hg_ret;

    /* Used by Test Driver */
    printf("Waiting for client...\n");
    fflush(stdout);

    /* Initialize the interface */
    network_class = HG_Test_server_init(argc, argv, &number_of_peers);

    hg_ret = HG_Handler_init(network_class);
    if (hg_ret != HG_SUCCESS) {
        fprintf(stderr, "Could not initialize function shipper handler\n");
        return EXIT_FAILURE;
    }

    hg_ret = HG_Bulk_init(network_class);
    if (hg_ret != HG_SUCCESS) {
        fprintf(stderr, "Could not initialize bulk data shipper\n");
        return EXIT_FAILURE;
    }

    /* Register routine */
    MERCURY_HANDLER_REGISTER("open", server_posix_open, open_in_t, open_out_t);
    MERCURY_HANDLER_REGISTER("write", server_posix_write, write_in_t, write_out_t);
    MERCURY_HANDLER_REGISTER("read", server_posix_read, read_in_t, read_out_t);
    MERCURY_HANDLER_REGISTER("close", server_posix_close, close_in_t, close_out_t);
    MERCURY_HANDLER_REGISTER_FINALIZE(server_finalize);

    while (finalizing != number_of_peers) {
        /* Receive new function calls */
        hg_ret = HG_Handler_process(HG_HANDLER_MAX_IDLE_TIME);
        if (hg_ret != HG_SUCCESS) {
            fprintf(stderr, "Could not receive function call\n");
            return EXIT_FAILURE;
        }
    }

    printf("Finalizing...\n");

    /* Finalize the interface */
    hg_ret = HG_Bulk_finalize();
    if (hg_ret != HG_SUCCESS) {
        fprintf(stderr, "Could not finalize bulk data shipper\n");
        return EXIT_FAILURE;
    }

    hg_ret = HG_Handler_finalize();
    if (hg_ret != HG_SUCCESS) {
        fprintf(stderr, "Could not finalize function shipper handler\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}