/*
 * Copyright (c) 2018, [Ribose Inc](https://www.ribose.com).
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1.  Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 * 2.  Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include <rnp/rnp_types.h>
#include "pgp-key.h"
#include "crypto.h"
#include <string.h>

int
sign_highlevel()
{
    rnp_t        rnp = {};
    rnp_params_t params = {};
    rnp_ctx_t    ctx = {};
    pgp_key_t *  key = NULL;
    int          result = 1;
    const char * message = "RNP signing sample message";
    char         signed_message[10000] = {0};
    FILE *       sfile = NULL;
    size_t       signed_len = 0;

    /* setup rnp parameters */
    rnp_params_init(&params);
    params.pubpath = strdup("pubring.pgp");
    params.secpath = strdup("secring.pgp");
    params.ks_pub_format = RNP_KEYSTORE_GPG;
    params.ks_sec_format = RNP_KEYSTORE_GPG;
    params.password_provider = {.callback = rnp_password_provider_string,
                                .userdata = (void *) "password"};

    /* initialize rnp structure */
    if (rnp_init(&rnp, &params) != RNP_SUCCESS) {
        fprintf(stdout, "failed to init rnp");
        goto finish;
    }

    /* load keys - you should have keys, generated by the generate example */
    if (!rnp_key_store_load_keys(&rnp, true)) {
        fprintf(stdout, "failed to load keyrings: run ./generate example first\n");
        goto finish;
    }

    /* initalize context with some data from rnp_t */
    if (rnp_ctx_init(&ctx, &rnp) != RNP_SUCCESS) {
        fprintf(stdout, "failed to initialize context\n");
        goto finish;
    }

    /* do we need armored output? */
    ctx.armor = true;
    /* signed file name and modification date */
    ctx.filename = strdup("source.txt");
    ctx.filemtime = time(NULL);
    /* compression algorithm and level (1-9), PGP_C_NONE to not use compression */
    ctx.zalg = PGP_C_ZIP;
    ctx.zlevel = 6;
    /* hash algorithm used for signature calculation */
    ctx.halg = PGP_HASH_SHA256;
    /* signature properties */
    ctx.sigcreate = time(NULL);
    ctx.sigexpire = 0;
    /* do we need cleartext signature? */
    ctx.clearsign = false;
    /* do we need detached signature? */
    ctx.detached = false;

    /* find and add signer's secret key. Key may be searched by userid, keyid, fingerprint or
     * grip. You may add as many keys as needed. */
    key = rnp_key_store_get_key_by_name(rnp.secring, "25519@key", NULL);
    if (!key) {
        fprintf(stdout, "key 25519@key not found, did you run generate example?\n");
        goto finish;
    }
    if (!list_append(&ctx.signers, &key, sizeof(key))) {
        fprintf(stdout, "failed to add key to signers list\n");
        goto finish;
    }

    /* within this call you should pass large enough output buffer, otherwise operation will
     * run twice */
    if (rnp_protect_mem(&ctx,
                        message,
                        strlen(message),
                        signed_message,
                        sizeof(signed_message),
                        &signed_len) != RNP_SUCCESS) {
        fprintf(stdout, "signing failed\n");
        goto finish;
    }

    fprintf(stdout, "%.*s", (int) signed_len, signed_message);

    if (!(sfile = fopen("signed.asc", "w+"))) {
        fprintf(stdout, "failed to write signed data to file\n");
        goto finish;
    }
    fwrite(signed_message, 1, signed_len, sfile);
    fclose(sfile);

    result = 0;
finish:
    rnp_ctx_free(&ctx);
    rnp_params_free(&params);
    rnp_end(&rnp);
    return result;
}

int
main(int argc, char **argv)
{
    return sign_highlevel();
}