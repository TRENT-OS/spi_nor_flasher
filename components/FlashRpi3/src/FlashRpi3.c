/*
 * Copyright (C) 2019-2020, Hensoldt Cyber GmbH
 */

#include "OS_Dataport.h"

#include "LibDebug/Debug.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <camkes.h>

static const OS_Dataport_t inPort  = OS_DATAPORT_ASSIGN(inputStorage_dp);
static const OS_Dataport_t outPort = OS_DATAPORT_ASSIGN(outputStorage_dp);

static bool
do_erase(size_t sz)
{
    OS_Error_t err;
    size_t wr;

    if ((err = outputStorage_rpc_erase(0, sz, &wr)) != OS_SUCCESS)
    {
        printf("outputStorage_rpc_erase() failed with %i", err);
        return false;
    }
    if (sz != wr)
    {
        printf("wanted to erase %zu bytes but did only %zu bytes", sz, wr);
        return false;
    }

    return true;
}

static bool
do_flash(size_t sz)
{
    OS_Error_t err;
    size_t rd, wr, flashed, bsz, left;

    /*
     * Flash from input storage to output storage using the rpc calls and
     * dataports directly. Make sure we do only as much per copy as we can
     * fit into the dataports.
     */

    flashed = 0;
    left    = sz;
    bsz     = OS_Dataport_getSize(inPort);

    while (left > 0)
    {
        if ((err = inputStorage_rpc_read(flashed, bsz, &rd)) != OS_SUCCESS)
        {
            printf("inputStorage_rpc_read() failed with %i", err);
            return false;
        }
        if (bsz != rd)
        {
            printf("wanted to read %zu bytes but did only %zu bytes", sz, wr);
            return false;
        }

        memcpy(OS_Dataport_getBuf(outPort), OS_Dataport_getBuf(inPort), bsz);

        if ((err = outputStorage_rpc_write(flashed, bsz, &wr)) != OS_SUCCESS)
        {
            printf("outputStorage_rpc_write() failed with %i", err);
            return false;
        }
        if (bsz != wr)
        {
            printf("wanted to write %zu bytes but did only %zu bytes", sz, wr);
            return false;
        }

        flashed += bsz;
        left    -= bsz;
        bsz      = (left < bsz) ? left : bsz;
    }

    return true;
}

void post_init()
{
    OS_Error_t err;
    size_t szIn = 0, szOut = 0;

    // Make sure dataport sizes match
    Debug_ASSERT(OS_Dataport_getSize(inPort) == OS_Dataport_getSize(outPort));

    if ((err = outputStorage_rpc_getSize(&szOut)) != OS_SUCCESS)
    {
        printf("outputStorage_rpc_getSize() failed with %i", err);
        return;
    }
    if ((err = inputStorage_rpc_getSize(&szIn)) != OS_SUCCESS)
    {
        printf("inputStorage_rpc_getSize() failed with %i", err);
        return;
    }

    printf("Input storage reports size of %zu bytes\n", szIn);
    printf("Output storage reports size of %zu bytes\n", szOut);
    if (szIn > szOut)
    {
        printf("Input is too big.");
        return;
    }

    printf("Erasing: %s\n", do_erase(szIn) ? "OK" : "FAIL");
    printf("Flashing: %s\n", do_flash(szIn) ? "OK" : "FAIL");
    printf("Done.\n");

    return;
}