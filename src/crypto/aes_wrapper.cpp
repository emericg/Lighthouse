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

#include "aes_wrapper.h"

#if defined(ENABLE_MBEDTLS)
#include "mbedtls/aes.h"
#include "mbedtls/ccm.h"
#else
#include "tinyAES/aes.hpp"
#endif

#include <cstring>

/* ************************************************************************** */

#if defined(ENABLE_MBEDTLS)
typedef mbedtls_aes_context aes_backend_context;
#else
typedef struct AES_ctx aes_backend_context;
#endif

static_assert(sizeof(aes_backend_context) <= sizeof(aes128_context),
              "aes128_context storage is too small for the selected AES backend");

static aes_backend_context *backend(aes128_context *ctx)
{
    return reinterpret_cast<aes_backend_context *>(ctx->opaque);
}

/* ************************************************************************** */

void aes128_init(aes128_context *ctx, const uint8_t *key)
{
#if defined(ENABLE_MBEDTLS)
    mbedtls_aes_init(backend(ctx));
    mbedtls_aes_setkey_enc(backend(ctx), key, AES128_KEY_SIZE * 8);
#else
    AES_init_ctx(backend(ctx), key);
#endif
}

void aes128_free(aes128_context *ctx)
{
#if defined(ENABLE_MBEDTLS)
    mbedtls_aes_free(backend(ctx));
#else
    memset(ctx->opaque, 0, sizeof(ctx->opaque));
#endif
}

void aes128_encrypt_block(aes128_context *ctx, const uint8_t *input, uint8_t *output)
{
#if defined(ENABLE_MBEDTLS)
    mbedtls_aes_crypt_ecb(backend(ctx), MBEDTLS_AES_ENCRYPT, input, output);
#else
    // tinyAES works in place
    if (output != input) memcpy(output, input, AES_BLOCK_SIZE);
    AES_ECB_encrypt(backend(ctx), output);
#endif
}

/* ************************************************************************** */

int aes128_ccm_star_decrypt(const uint8_t *key,
                            const uint8_t *nonce, const size_t nonce_len,
                            const uint8_t *input, const size_t input_len,
                            uint8_t *output)
{
    if (nonce_len < 7 || nonce_len > 13) return -1;

#if defined(ENABLE_MBEDTLS)

    mbedtls_ccm_context ctx;
    mbedtls_ccm_init(&ctx);

    int status = mbedtls_ccm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, key, AES128_KEY_SIZE * 8);
    if (status == 0)
    {
        // no associated data, and a zero length tag: CCM*, not CCM
        status = mbedtls_ccm_star_auth_decrypt(&ctx, input_len,
                                               nonce, nonce_len,
                                               nullptr, 0,
                                               input, output,
                                               nullptr, 0);
    }

    mbedtls_ccm_free(&ctx);

    return status;

#else

    // With an empty tag there is no MAC left to verify, and what remains of CCM* is plain
    // CTR mode over the CCM counter blocks A_i = [flags][nonce][counter], counter being
    // big endian over the last q bytes (RFC 3610 §2.3).
    // The payload starts at A_1, because A_0 is reserved for masking the tag we do not have.
    const size_t q = 15 - nonce_len;

    aes128_context ctx;
    aes128_init(&ctx, key);

    uint8_t counter_block[AES_BLOCK_SIZE];
    uint8_t keystream[AES_BLOCK_SIZE];

    memset(counter_block, 0, sizeof(counter_block));
    counter_block[0] = static_cast<uint8_t>(q - 1);
    memcpy(counter_block + 1, nonce, nonce_len);

    size_t counter = 1;
    for (size_t offset = 0; offset < input_len; offset += AES_BLOCK_SIZE)
    {
        for (size_t i = 0; i < q; i++)
        {
            counter_block[15 - i] = static_cast<uint8_t>((counter >> (8 * i)) & 0xff);
        }

        aes128_encrypt_block(&ctx, counter_block, keystream);

        const size_t blocklen = (input_len - offset < AES_BLOCK_SIZE) ? input_len - offset
                                                                      : AES_BLOCK_SIZE;
        for (size_t i = 0; i < blocklen; i++)
        {
            output[offset + i] = input[offset + i] ^ keystream[i];
        }

        counter++;
    }

    aes128_free(&ctx);
    memset(keystream, 0, sizeof(keystream));

    return 0;

#endif // ENABLE_MBEDTLS
}

/* ************************************************************************** */
