/*!
 * This file is part of Lighthouse.
 * COPYRIGHT (C) 2026 Emeric Grange - All Rights Reserved
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * \date      2026
 * \author    Emeric Grange <emeric.grange@gmail.com>
 */

#include "pgp_crypto.h"

#include "aes.hpp"

#include <string.h>
#include <stdint.h>
#include <stdlib.h>

/* ************************************************************************** */

static void pgp_aes_encrypt(struct AES_ctx *ctx, const uint8_t *inp, uint8_t *out)
{
    memcpy(out, inp, 16);
    AES_ECB_encrypt(ctx, out);
}

static void aes_setkey(struct AES_ctx *ctx, const uint8_t *key)
{
    AES_init_ctx(ctx, key);
}

/*!
 * Formats the first block for the hash, CCM style: a flags byte, then only 13
 * of the 16 nonce bytes, then the payload length big endian in the last two.
 * The 57 (0x39) is what the device firmware uses, it is not derived from anything.
 */
static void init_nonce_hash(const uint8_t *inp_nonce, const int datalen, uint8_t *nonce_hash)
{
    memcpy(nonce_hash+1, inp_nonce, 13);

    nonce_hash[0] = 57;
    nonce_hash[14] = (datalen>>8) & 0xff;
    nonce_hash[15] = datalen & 0xff;
}

/*!
 * CBC-MAC over the payload, seeded with the formatted nonce block.
 *
 * This is not a standard MAC and it is not interchangeable with one. Note that
 * a trailing partial block is silently ignored (count/16), which is fine for the
 * fixed 16 and 80 bytes payloads used here but would quietly drop data otherwise.
 */
static void aes_hash(struct AES_ctx *ctx, const uint8_t *nonce,
                     const uint8_t *data, const int count,
                     uint8_t *output)
{
    uint8_t tmp[16];
    uint8_t tmp2[16];
    uint8_t nonce_hash[16];

    init_nonce_hash(nonce, count, nonce_hash);

    pgp_aes_encrypt(ctx, nonce_hash, tmp);  // encrypt nonce
    int blocks = count/16;
    const uint8_t *tmpdata = data;
    for (int i =0; i < blocks; i++)
    {
        for (int j = 0; j < 16; j++)
        {
            tmp[j] ^= tmpdata[j];           // xor with input
        }
        tmpdata += 16;
        memcpy(tmp2, tmp, 16);              // copy to temp
        pgp_aes_encrypt(ctx, tmp2, tmp);
    }
    memcpy(output, tmp, 16);
}

/*!
 * Formats the counter block, same CCM style layout as init_nonce_hash():
 * flags byte, 13 nonce bytes, then a 16 bit counter starting at zero.
 */
static void init_nonce_ctr(const uint8_t *inp_nonce, uint8_t *nonce_ctr)
{
    memcpy(nonce_ctr+1, inp_nonce, 13);
    nonce_ctr[0] = 1;
    nonce_ctr[14] = 0;
    nonce_ctr[15] = 0;
}

/*!
 * Masks a hash with the keystream of counter ZERO.
 *
 * Counter 0 is reserved for exactly this, which is why aes_ctr() below starts its
 * payload at counter 1. Do not "fix" that asymmetry, it is what makes the hashes verify.
 */
static void encrypt_block(struct AES_ctx *ctx, const uint8_t *nonce_iv, const uint8_t *nonce, uint8_t *output)
{
    uint8_t tmp[16];

    uint8_t nonce_ctr[16];

    init_nonce_ctr(nonce, nonce_ctr);

    pgp_aes_encrypt(ctx, nonce_ctr, tmp);

    for (int i = 0; i < 16; i++)
    {
        output[i] = tmp[i] ^ nonce_iv[i];
    }
}

static void inc_ctr(uint8_t * ctr)
{
    ctr[15]++;
    if (ctr[15] == 0)
    {
        ctr[14]++;
    }
}

/*!
 * CTR mode, and its own inverse, so the same call encrypts and decrypts.
 *
 * The counter is incremented BEFORE the first block, so payload data starts at counter
 * 1 and counter 0 stays reserved for encrypt_block()'s hash mask. As with aes_hash(),
 * a trailing partial block is silently ignored.
 */
static void aes_ctr(struct AES_ctx *ctx, const uint8_t *nonce,
                    const uint8_t *data, int count,
                    uint8_t *output)
{
    uint8_t ctr[16];
    uint8_t ectr[16];

    init_nonce_ctr(nonce, ctr);

    int blocks = count/16;
    const uint8_t *tmpdata = data;
    uint8_t *outptr = output;

    for (int i = 0; i < blocks; i++)
    {
        inc_ctr(ctr);
        pgp_aes_encrypt(ctx, ctr, ectr);

        for (int j = 0; j < 16; j++)
        {
            *outptr = ectr[j] ^ *tmpdata;
            ++outptr;
            ++tmpdata;
        }
    }
}

/* ************************************************************************** */

void generate_nonce(uint8_t *nonce)
{
    // Only 13 bytes are consumed by the block formatting, and a capture of the real app
    // shows it zeroing the last three rather than randomizing them
    memset(nonce, 0, 16);
    for (int i = 0; i < 13; i++)
    {
        nonce[i] = rand() & 0xff; // random quality is not important here
    }
}

int app_decrypt_chal_0(const uint8_t *data, const uint8_t *device_key,
                       struct main_challenge_data *output)
{
    struct AES_ctx ctx;
    const struct challenge_data *chal = (const struct challenge_data *)data;
    uint8_t tmp_hash[16];
    uint8_t expected_hash[16];

    aes_setkey(&ctx, device_key);

    // CTR mode is its own inverse, so this is the same call the device used to encrypt
    aes_ctr(&ctx, chal->nonce, chal->encrypted_main_challenge, 80, (uint8_t *)output);

    // Recompute the outer hash over the plaintext and compare with the one we got.
    // A mismatch means the device key is wrong for this device.
    aes_hash(&ctx, chal->nonce, (const uint8_t *)output, 80, tmp_hash);
    encrypt_block(&ctx, tmp_hash, chal->nonce, expected_hash);

    return memcmp(expected_hash, chal->encrypted_hash, 16) == 0;
}

int app_decrypt_challenge(const struct main_challenge_data *main, uint8_t *output)
{
    struct AES_ctx ctx;
    uint8_t tmp_hash[16];
    uint8_t expected_hash[16];

    // The inner layer is keyed with the session key carried by the main challenge
    aes_setkey(&ctx, main->key);
    aes_ctr(&ctx, main->nonce, main->encrypted_challenge, 16, output);

    aes_hash(&ctx, main->nonce, output, 16, tmp_hash);
    encrypt_block(&ctx, tmp_hash, main->nonce, expected_hash);

    return memcmp(expected_hash, main->encrypted_hash, 16) == 0;
}

void generate_next_chal(const uint8_t *indata, const uint8_t *key, const uint8_t *nonce, struct next_challenge *output)
{
    struct AES_ctx ctx;
    uint8_t data[16];
    uint8_t tmp_hash[16];

    if (indata)
    {
        memcpy(data, indata, 16);
    } else
    {
        memset(data, 0, 16);
        data[0] = 0xaa;
    }

    memcpy(output->nonce, nonce, 16);

    aes_setkey(&ctx, key);
    aes_ctr(&ctx, output->nonce, data, 16, output->encrypted_challenge);

    aes_hash(&ctx, output->nonce, data, 16, tmp_hash);
    encrypt_block(&ctx, tmp_hash, output->nonce, output->encrypted_hash);
}

int decrypt_next(const uint8_t *data, const uint8_t *key, uint8_t *output)
{
    struct AES_ctx ctx;
    const struct next_challenge *chal = (const struct next_challenge *)data;

    aes_setkey(&ctx, key);
    aes_ctr(&ctx, chal->nonce, chal->encrypted_challenge, 16, output);

    // Unmask the hash we were sent, recompute it over the plaintext, and compare
    uint8_t enc_nonce[16];
    encrypt_block(&ctx, chal->encrypted_hash, chal->nonce, enc_nonce);

    uint8_t hash_1[16];
    aes_hash(&ctx, chal->nonce, output, 16, hash_1);

    return memcmp(hash_1, enc_nonce, 16) == 0;
}

void generate_reconnect_response(const uint8_t *key,
                                 const uint8_t *challenge,
                                 uint8_t *output)
{
    struct AES_ctx ctx;
    aes_setkey(&ctx, key);
    pgp_aes_encrypt(&ctx, challenge, output);
    for (int i = 0; i < 16; i++)
    {
        output[i] ^= challenge[i+16];
    }
}

/* ************************************************************************** */
