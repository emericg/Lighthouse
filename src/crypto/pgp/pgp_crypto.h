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
 * Certification crypto for the "Pokemon Go Plus", central (app) side.
 *
 * The initial challenge the device offers is built in two layers:
 *
 * - the OUTER layer is encrypted with the device key, a 16 bytes secret burned into
 *   that particular device's OTP memory. It wraps an 80 bytes main_challenge_data.
 * - the INNER layer is encrypted with the SESSION key, which is itself carried inside
 *   that main_challenge_data. It wraps the 16 bytes challenge we have to echo back.
 *
 * So the device key gets us the session key, and the session key gets us the challenge
 * and every step that follows. Without the device key nothing can be opened at all,
 * which is the whole point of the scheme.
 *
 * The primitives are neither plain CTR nor a standard MAC, they are the CCM-ish
 * construction the device firmware uses. See the notes in pgp_crypto.cpp before
 * touching them, the counter numbering in particular is load bearing.
 *
 * Based on the work of  Yohanes Nugroho:
 * - https://github.com/yohanes/pgpemu/
 * - https://tinyhack.com/2018/11/21/reverse-engineering-pokemon-go-plus/
 *
 * And Jesus Bamford:
 * - https://github.com/Jesus805/Suota-Go-Plus
 * - https://coderjesus.com/blog/pgp-suota/
 */

#ifndef PGP_CRYPTO_H
#define PGP_CRYPTO_H
/* ************************************************************************** */

#include <stdint.h>

/* ************************************************************************** */

// These structures are sent and received as-is, so their layout is part of the protocol.
// #pragma pack is used rather than __attribute__((packed)), which MSVC ignores...
#pragma pack(push, 1)

/*!
 * The 80 bytes payload wrapped by the outer layer of the initial challenge.
 */
struct main_challenge_data {
    uint8_t bt_addr[6];             //!< the device's own address, byte reversed
    uint8_t key[16];                //!< THE SESSION KEY, everything later is keyed with it
    uint8_t nonce[16];              //!< nonce for the inner layer
    uint8_t encrypted_challenge[16];
    uint8_t encrypted_hash[16];
    uint8_t flash_data[10];         //!< device state, we do not use it
};

/*!
 * The 378 bytes initial challenge, read from SFIDA_TO_CENTRAL.
 */
struct challenge_data {
    uint8_t state[4];
    uint8_t nonce[16];                      //!< nonce for the outer layer
    uint8_t encrypted_main_challenge[80];   //!< a main_challenge_data, device key encrypted
    uint8_t encrypted_hash[16];
    uint8_t bt_addr[6];
    uint8_t blob[256];                      //!< the device's OTP blob, echoed as-is
};

/*!
 * The 52 bytes payload exchanged by both sides during the later steps.
 */
struct next_challenge {
    uint8_t state[4];
    uint8_t nonce[16];
    uint8_t encrypted_challenge[16];
    uint8_t encrypted_hash[16];
};

/*!
 * The 20 bytes answer we write back to CENTRAL_TO_SFIDA for the initial challenge.
 */
struct challenge_response {
    uint8_t state[4];
    uint8_t response[16];
};

#pragma pack(pop)

// The sizes are part of the protocol, a packing change must not pass silently
static_assert(sizeof(struct main_challenge_data) == 80, "main_challenge_data must be 80 bytes");
static_assert(sizeof(struct challenge_data) == 378, "challenge_data must be 378 bytes");
static_assert(sizeof(struct next_challenge) == 52, "next_challenge must be 52 bytes");
static_assert(sizeof(struct challenge_response) == 20, "challenge_response must be 20 bytes");

/* ************************************************************************** */

/*!
 * Central (app) side of the initial certification challenge.
 *
 * Takes the 378 bytes challenge_data read from SFIDA_TO_CENTRAL and peels the outer
 * layer with this device's key, recovering the 80 bytes main_challenge_data, which
 * carries the session key used by every later step.
 *
 * This is the inverse of generate_chal_0(), which is the device side.
 *
 * \param data: 378 bytes, a serialized struct challenge_data
 * \param device_key: 16 bytes, unique to the physical device (from its OTP memory)
 * \param output: receives the decrypted main challenge
 * \return 1 if the outer hash verifies, 0 if it does not (wrong key, or corrupt data)
 */
int app_decrypt_chal_0(const uint8_t *data,
                       const uint8_t *device_key,
                       struct main_challenge_data *output);

/*!
 * Recovers the plaintext challenge the device wants echoed back.
 *
 * \param main: the decrypted main challenge from app_decrypt_chal_0()
 * \param output: receives the 16 bytes challenge
 * \return 1 if the inner hash verifies, 0 if it does not
 */
int app_decrypt_challenge(const struct main_challenge_data *main, uint8_t *output);

/*!
 * Fills a 16 bytes nonce buffer.
 *
 * Only the first 13 bytes are ever used, the block formatting takes 13 of them and the
 * last two are the counter or length. The real app leaves the last three bytes zeroed,
 * so we do the same rather than filling all 16 with noise.
 *
 * \param nonce: receives 16 bytes
 */
void generate_nonce(uint8_t *nonce);

/*!
 * Builds one of the 52 bytes challenges we send during the later steps.
 *
 * \param data: 16 bytes to wrap, or nullptr to use the device's canned {0xaa, 0...}
 * \param key: the session key
 * \param nonce: 16 bytes, fresh per challenge
 * \param output: receives the challenge
 */
void generate_next_chal(const uint8_t *data, const uint8_t *key,
                        const uint8_t *nonce,
                        struct next_challenge *output);

/*!
 * Answers the shortened challenge a device sends when it already knows us.
 *
 * Not used yet: it belongs to the reconnect path, where a device holding a reconnect
 * key announces state 3 with a 32 bytes challenge instead of the full 378 bytes one.
 * Kept because that path still has to be implemented.
 *
 * \param key: the session key
 * \param challenge: 32 bytes
 * \param output: receives 16 bytes
 */
void generate_reconnect_response(const uint8_t *key,
                                 const uint8_t *challenge,
                                 uint8_t *output);

/*!
 * Opens one of the 52 bytes challenges the device sends us.
 *
 * \param data: 52 bytes, a serialized struct next_challenge
 * \param key: the session key
 * \param output: receives the 16 bytes payload
 * \return 1 if the hash verifies, 0 if it does not
 */
int decrypt_next(const uint8_t *data, const uint8_t *key, uint8_t *output);

/* ************************************************************************** */
#endif // PGP_CRYPTO_H
