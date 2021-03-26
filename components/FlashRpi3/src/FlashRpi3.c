/*
 * Copyright (C) 2019-2020, Hensoldt Cyber GmbH
 */

#include "OS_Dataport.h"

#include "lib_compiler/compiler.h"
#include "lib_debug/Debug.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <camkes.h>

static const OS_Dataport_t inPort  = OS_DATAPORT_ASSIGN(inputStorage_dp);
static const OS_Dataport_t outPort = OS_DATAPORT_ASSIGN(outputStorage_dp);

static bool
do_erase(off_t sz)
{
    OS_Error_t err;
    off_t wr;

    if ((err = outputStorage_rpc_erase(0, sz, &wr)) != OS_SUCCESS)
    {
        printf("outputStorage_rpc_erase() failed with %i", err);
        return false;
    }
    if (sz != wr)
    {
        printf(
            "wanted to erase %" PRIiMAX " bytes "
            "but did only %" PRIiMAX " bytes",
            sz,
            wr);

        return false;
    }

    return true;
}

static bool
do_flash(off_t sz)
{
    OS_Error_t err;
    size_t rd, wr;
    off_t flashed, bsz, left;

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
            printf(
                "wanted to read %" PRIiMAX " bytes but did only %zu bytes",
                sz,
                rd);

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
            printf(
                "wanted to write %" PRIiMAX " bytes but did only %zu bytes",
                sz,
                wr);

            return false;
        }

        flashed += bsz;
        left    -= bsz;
        bsz      = (left < bsz) ? left : bsz;
    }

    return true;
}

static bool
do_verify(off_t sz)
{
    OS_Error_t err;
    size_t rd;
    off_t verified, bsz, left;

    verified    = 0;
    left        = sz;
    bsz         = OS_Dataport_getSize(inPort);

    while (left > 0)
    {
        if ((err = inputStorage_rpc_read(verified, bsz, &rd)) != OS_SUCCESS)
        {
            printf("inputStorage_rpc_read() failed with %i", err);
            return false;
        }
        if (bsz != rd)
        {
            printf(
                "wanted to read %" PRIiMAX " bytes but did only %zu bytes",
                sz,
                rd);

            return false;
        }

        if ((err = outputStorage_rpc_read(verified, bsz, &rd)) != OS_SUCCESS)
        {
            printf("outputStorage_rpc_read() failed with %i", err);
            return false;
        }
        if (bsz != rd)
        {
            printf(
                "wanted to read %" PRIiMAX " bytes but did only %zu bytes",
                sz,
                rd);

            return false;
        }

        size_t differPos =
            memcmp(OS_Dataport_getBuf(outPort),
                    OS_Dataport_getBuf(inPort),
                    bsz);
        if (differPos)
        {
            printf("verification failed comparing bytes at offset %" PRIiMAX,
                verified + differPos);
            return false;
        }

        verified    += bsz;
        left        -= bsz;
        bsz         = (left < bsz) ? left : bsz;
    }

    return true;
}

void post_init()
{
    OS_Error_t err;
    off_t szIn = 0, szOut = 0;

    // Make sure dataport sizes match
    DECL_UNUSED_VAR(const bool arePortsSizesEqual) =
        (OS_Dataport_getSize(inPort) == OS_Dataport_getSize(outPort));
    Debug_ASSERT(arePortsSizesEqual);

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

    printf("Input storage reports size of %" PRIiMAX " bytes\n", szIn);
    printf("Output storage reports size of %" PRIiMAX " bytes\n", szOut);
    if (szIn > szOut)
    {
        printf("Input is too big.");
        return;
    }

    printf("Erasing: %s\n", do_erase(szIn) ? "OK" : "FAIL");
    printf("Flashing: %s\n", do_flash(szIn) ? "OK" : "FAIL");
    printf("Verifying: %s\n", do_verify(szIn) ? "OK" : "FAIL");
    printf("Done.\n");

    return;
}
