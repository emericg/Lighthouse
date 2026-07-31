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

/*!
 * Thin AES-128 abstraction over the two crypto backends Lighthouse can be built with.
 *
 * - mbedTLS, when ENABLE_MBEDTLS is set. Optional, and NOT shipped with the sources.
 * - tinyAES otherwise. Always available, it is the fallback and is bundled.
 *
 * Every device that needs AES goes through this header, so no device code has to
 * know which backend is in use, and no device is limited to one of them.
 *
 * Neither backend header is included here on purpose: tinyAES defines bare ECB / CBC / CTR
 * macros that would leak into every consumer, so the key schedule is kept opaque.
 */

#ifndef AES_WRAPPER_H
#define AES_WRAPPER_H
/* ************************************************************************** */

#include <cstdint>
#include <cstddef>

/* ************************************************************************** */

#define AES128_KEY_SIZE     16      //!< AES-128 only, both backends are configured for a 16 bytes key
#define AES_BLOCK_SIZE      16      //!< AES is 128 bits block only

/*!
 * \brief An AES-128 key schedule.
 *
 * The storage is sized for the largest backend context, and checked against the actual
 * one by a static_assert in aes_wrapper.cpp, so this header stays backend agnostic.
 * uint64_t rather than uint8_t so the buffer is suitably aligned for whatever goes in it.
 */
typedef struct aes128_context {
    uint64_t opaque[40];
} aes128_context;

/*!
 * \brief Schedules an AES-128 key for encryption.
 * \param ctx: receives the key schedule, to be released with aes128_free()
 * \param key: AES128_KEY_SIZE bytes
 *
 * Only the encryption direction is set up: the modes used here (ECB single block, CTR, CCM*)
 * never call the block cipher backward.
 */
void aes128_init(aes128_context *ctx, const uint8_t *key);

/*!
 * \brief Releases a key schedule, wiping the key material it holds.
 * \param ctx: a context from aes128_init()
 */
void aes128_free(aes128_context *ctx);

/*!
 * \brief Encrypts one block, ECB style.
 * \param ctx: a context from aes128_init()
 * \param input: AES_BLOCK_SIZE bytes
 * \param output: receives AES_BLOCK_SIZE bytes, may alias input
 *
 * This is the raw block cipher, the building block the modes are made of.
 * It is not a mode of operation, and must not be used to encrypt more than one block of real data.
 */
void aes128_encrypt_block(aes128_context *ctx, const uint8_t *input, uint8_t *output);

/*!
 * \brief AES-128 CCM* decryption with an empty authentication tag.
 * \param key: AES128_KEY_SIZE bytes
 * \param nonce: 7 to 13 bytes
 * \param nonce_len: length of nonce
 * \param input: the ciphertext, any length
 * \param input_len: length of input
 * \param output: receives input_len bytes, may alias input
 * \return 0 on success, non zero if the parameters are out of range
 *
 * CCM* is the CCM variant that allows a zero length tag, which is what the Xiaomi BLE advertisement encryption uses.
 * With no tag there is nothing to verify, so this only recovers the plaintext:
 * the caller gets no authenticity guarantee whatsoever, and a wrong key yields garbage rather than an error.
 *
 * Associated data is not accepted, because with an empty tag it could not influence the result anyway.
 */
int aes128_ccm_star_decrypt(const uint8_t *key,
                            const uint8_t *nonce, const size_t nonce_len,
                            const uint8_t *input, const size_t input_len,
                            uint8_t *output);

/* ************************************************************************** */
#endif // AES_WRAPPER_H
