
## About Pokemon GO Plus

* The Nintendo [Pokemon GO Plus](https://www.pokemon.com/us/pokemon-video-games/pokemon-go-plus) (PGP) is a wearable accessory for the Pokemon GO mobile game
* Codenamed "Sfida" (Italian for [challenge](https://en.wiktionary.org/wiki/sfida)) in the firmware and in the BLE protocol
* Notifies the wearer with a RGB LED and a vibration motor, and takes a single button press as input
* Uses Bluetooth Low Energy (BLE) and has a limited range
* A CR2032 coin cell battery is used as power source

## Hardware

* CPU: Dialog Semiconductor DA14580 (Bluetooth 4.0)
* 8 intensity vibration motor
* 4 bit RGB LED (12 bit color)
* CR2032 coin cell

## Features

* RGB LED and vibration notifications
* Button press notification
* Battery level

## Protocol

The device uses BLE GATT for communication, but almost nothing is available on connection. 
Every characteristic outside of the generic services answers reads and writes with `Insufficient Authorization`
until the central has completed a certification handshake against the `CERTIFICATE_SERVICE`.  
That handshake requires a key unique to each physical device, and cannot be derived: it has to be extracted from the device itself.  

Once certification starts, the device will vibrate strongly once.  
If the certification starts but fails, the device will blink red and vibrate strongly three times.  
The device also drops the connection on its own after a few seconds if the central does not certify.  

Once certification completes, the LED, the vibration motor, the button notifications and the battery level all become accessible for the duration of the connection.

### BLE & GATT

The basic technologies behind the communication are [Bluetooth Low Energy (BLE)](https://en.wikipedia.org/wiki/Bluetooth_Low_Energy) and [GATT](https://www.bluetooth.com/specifications/gatt).
They allow the devices and the app to share data in a defined manner and define the way you can discover the devices and their services.
In general you have to know about services and characteristics to talk to a BLE device.

<img src="endianness.png" width="400px" alt="Endianness" align="right" />

### Data structure

Bluetooth payload data typically uses little-endian byte order.
This means that the data is represented with the least significant byte first.

The certification payloads are an exception: their multi byte fields are opaque blocks of AES output,
and the 4 bytes state field that prefixes each of them is written most significant byte last (ex `01 00 00 00` for state 1).

## Advertisement data

Once the device is powered on by clicking its button, it will advertise for about 10 seconds.

It broadcast one `0x00` byte over `service data` with 16 bits service UUID `0x0000`.

## Services and characteristics

The name advertised by the device is `Pokemon GO Plus`, and its MAC address belongs to the `Nintendo Co. Ltd.` range.

##### Generic access (UUID 00001800-0000-1000-8000-00805f9b34fb)

| Characteristic UUID                  | Access      | Description                         |
| ------------------------------------ | ----------- | ----------------------------------- |
| 00002a00-0000-1000-8000-00805f9b34fb | read        | device name                         |
| 00002a01-0000-1000-8000-00805f9b34fb | read        | appearance                          |
| 00002a02-0000-1000-8000-00805f9b34fb | read/write  | peripheral privacy flag             |
| 00002a04-0000-1000-8000-00805f9b34fb | read        | preferred connection parameters     |

##### Generic attribute (UUID 00001801-0000-1000-8000-00805f9b34fb)

| Characteristic UUID                  | Access      | Description                         |
| ------------------------------------ | ----------- | ----------------------------------- |
| 00002a05-0000-1000-8000-00805f9b34fb | read/indic. | service changed                     |

##### Battery service (UUID 0000180f-0000-1000-8000-00805f9b34fb)

| Characteristic UUID                  | Access      | Description                         |
| ------------------------------------ | ----------- | ----------------------------------- |
| 00002a19-0000-1000-8000-00805f9b34fb | read        | battery level                       |

##### Device control service (UUID 21c50462-67cb-63a3-5c4c-82b5b9939aeb)

| Characteristic UUID                  | Access      | Description                         |
| ------------------------------------ | ----------- | ----------------------------------- |
| 21c50462-67cb-63a3-5c4c-82b5b9939aec | write       | LED and vibration control           |
| 21c50462-67cb-63a3-5c4c-82b5b9939aed | notify      | button press notification           |
| 21c50462-67cb-63a3-5c4c-82b5b9939aef | write       | firmware update request             |
| 21c50462-67cb-63a3-5c4c-82b5b9939af0 | read        | firmware version                    |

##### Certificate service (UUID bbe87709-5b89-4433-ab7f-8b8eef0d8e37)

| Characteristic UUID                  | Access      | Description                              |
| ------------------------------------ | ----------- | ---------------------------------------- |
| bbe87709-5b89-4433-ab7f-8b8eef0d8e38 | write       | central to device (`CENTRAL_TO_SFIDA`)   |
| bbe87709-5b89-4433-ab7f-8b8eef0d8e39 | notify      | device commands (`SFIDA_COMMANDS`)       |
| bbe87709-5b89-4433-ab7f-8b8eef0d8e3a | read        | device to central (`SFIDA_TO_CENTRAL`)   |

##### Firmware update service (UUID 0000fef5-0000-1000-8000-00805f9b34fb)

The standard Dialog Semiconductor SUOTA service, used to flash the DA14580. Out of scope for this document.

## Certification process

The device offers an encrypted challenge, the central has to answer it using a 16 bytes device key unique to that physical device,
and both sides then prove to each other that they hold the session key carried inside the challenge.
Until this completes, every other characteristic answers with `Insufficient Authorization`.

The device key cannot be derived. It lives in the DA14580 OTP memory and has to be dumped from the device, see the [SUOTA Go+](https://github.com/Jesus805/Suota-Go-Plus) tool.  

#### Prerequisites

Two things are needed before the handshake can even start.

The link must be **encrypted**: the certificate service characteristics require authentication, so the central has to be bonded with the device.
Writing the `SFIDA_COMMANDS` notification descriptor on an unencrypted link is refused with `Insufficient Authorization`, and the handshake never begins.

The central must hold the **device key**, 16 bytes unique to that physical device.
A key dump also contains a 256 bytes `blob` taken from the same OTP area,
which the device sends inside its first challenge but which the central never has to verify:
the blob is only needed when *emulating* a device, not when talking to one.

#### Cryptographic primitives

Everything is built on AES-128 in ECB mode, used as a block function by a CCM-like construction.
It is neither plain CTR nor a standard MAC, and none of the pieces are interchangeable with their standard counterparts.

Nonces are generated as 16 bytes buffers, but only the first 13 are ever consumed by the block formatting below.
The official application leaves the last three bytes zeroed rather than randomizing them.

##### Counter block

| Position | 00        | 01-13         | 14-15                                  |
| -------- | --------- | ------------- | -------------------------------------- |
| Value    | `0x01`    | nonce[0:12]   | uint16_be counter, starting at 0       |

The keystream block for counter `n` is `AES(key, counter_block(n))`.

##### Hash block

| Position | 00        | 01-13         | 14-15                                  |
| -------- | --------- | ------------- | -------------------------------------- |
| Value    | `0x39`    | nonce[0:12]   | uint16_be payload length, in bytes     |

The `0x39` (57) flags byte is what the device firmware uses, it is not derived from anything.

##### Operations

| Operation         | Description                                               |
| ----------------- | --------------------------------------------------------- |
| `aes_ctr()`       | CTR mode over whole 16 bytes blocks. The counter is incremented **before** the first block, so the payload starts at counter 1 and counter 0 stays reserved. Being its own inverse, the same operation encrypts and decrypts. |
| `aes_hash()`      | CBC-MAC over the payload, seeded with `AES(key, hash_block)`. Returns the last 16 bytes block. |
| `encrypt_block()` | Masks a 16 bytes hash with the keystream of counter **0**, `hash ^ AES(key, counter_block(0))`. Also its own inverse. |

Both `aes_ctr()` and `aes_hash()` silently ignore a trailing partial block, which is harmless here because every payload they are used on is a multiple of 16 bytes (16 or 80).

So a payload is always protected the same way: the plaintext is CTR encrypted, hashed with `aes_hash()`, and the hash is masked with `encrypt_block()`.
Verifying means CTR decrypting, recomputing the hash over the plaintext, and comparing it with the unmasked one.

#### Payload structures

All the certification payloads start with the same 4 bytes state field, and it is the *only* multi byte field in this protocol that is not opaque AES output.
It carries the step number in its first byte, and the remaining three bytes are zero (`01 00 00 00` for state 1).

##### challenge_data, 378 bytes

The first challenge, read from `SFIDA_TO_CENTRAL`. Encrypted with the **device key**.

| Bytes   | Field                     | Description                                                     |
| ------- | ------------------------- | --------------------------------------------------------------- |
| 000-003 | state                     | `00 00 00 00`                                                   |
| 004-019 | nonce                     | nonce of the outer layer                                        |
| 020-099 | encrypted_main_challenge  | a `main_challenge_data`, CTR encrypted with the device key      |
| 100-115 | encrypted_hash            | masked hash of the plaintext `main_challenge_data`              |
| 116-121 | bt_addr                   | the device address, byte reversed                               |
| 122-377 | blob                      | 256 bytes read from the device OTP memory, sent in the clear    |

##### main_challenge_data, 80 bytes

The plaintext hidden by the outer layer above. Its own payload is encrypted with the **session key** it carries.

| Bytes | Field               | Description                                                             |
| ----- | ------------------- | ----------------------------------------------------------------------- |
| 00-05 | bt_addr             | the device address, byte reversed                                       |
| 06-21 | key                 | **the session key**, everything after this point is keyed with it       |
| 22-37 | nonce               | nonce of the inner layer                                                |
| 38-53 | encrypted_challenge | the 16 bytes challenge to echo back, CTR encrypted with the session key |
| 54-69 | encrypted_hash      | masked hash of the plaintext challenge                                  |
| 70-79 | flash_data          | device state, of no use to the central                                  |

The device address is a useful sanity check: if the reversed `bt_addr` matches the address that was connected to,
the decryption is right in substance and not merely self consistent.

##### challenge_response, 20 bytes

The answer to the first challenge, written to `CENTRAL_TO_SFIDA`. Sent **in the clear**, the point being to prove that the 16 bytes could be recovered at all.

| Bytes | Field    | Description                              |
| ----- | -------- | ---------------------------------------- |
| 00-03 | state    | `01 00 00 00`                            |
| 04-19 | response | the decrypted 16 bytes challenge         |

##### next_challenge, 52 bytes

Used by both sides for every step after the first one, always keyed with the **session key**.

| Bytes | Field               | Description                                     |
| ----- | ------------------- | ----------------------------------------------- |
| 00-03 | state               | step number                                     |
| 04-19 | nonce               | fresh for each challenge                        |
| 20-35 | encrypted_challenge | 16 bytes payload, CTR encrypted                 |
| 36-51 | encrypted_hash      | masked hash of the plaintext payload            |

#### Message flow

The device drives the whole exchange through `SFIDA_COMMANDS` notifications.
Each notification carries 4 bytes, the first one being the state the device just moved to, and the payload that goes with it is read from `SFIDA_TO_CENTRAL`.

The central answers by writing to `CENTRAL_TO_SFIDA`, and the state it stamps on its answer is the state it drives the device to.
Every step is therefore one notification, one read and one write.

| Step | Notification  | Read from `SFIDA_TO_CENTRAL` | Write to `CENTRAL_TO_SFIDA`     |
| ---- | ------------- | ---------------------------- | ------------------------------- |
| 0    | `00 00 00 00` | 378 bytes `challenge_data`   | 20 bytes `challenge_response`   |
| 1    | `01 00 00 00` | 52 bytes `next_challenge`    | 52 bytes `next_challenge`       |
| 2    | `02 00 00 00` | 20 bytes echo                | 52 bytes `next_challenge`       |
| 3    | `04 00 23 00` | -                            | -                               |

#### Step by step

##### Step 0, the device challenges the central

Subscribing to `SFIDA_COMMANDS`, by writing `0x0100` to its client characteristic configuration descriptor, is what makes the device offer its first challenge.
It answers with a `00 00 00 00` notification and loads 378 bytes into `SFIDA_TO_CENTRAL`.
This read is longer than the default MTU, so it goes over several ATT `Read Blob` transactions.

The central peels the two layers:

* CTR decrypt `encrypted_main_challenge` with the **device key**, then recompute and compare the outer hash.
  A mismatch means the key does not belong to this device, and there is nothing to be done about it.
* CTR decrypt `encrypted_challenge` from the recovered `main_challenge_data` with the **session key** it carries,
  then recompute and compare the inner hash.

The 16 bytes challenge is then written back as-is in a `challenge_response` stamped with state `01`.
This authenticates the central: only a holder of the device key could have reached those bytes.

##### Step 1, mutual challenge

The device answers with a `01 00 00 00` notification and a 52 bytes `next_challenge`, which the central verifies with the session key.
Its content is not meaningful, the emulated device sends a fixed `AA 00 ... 00` filler there, so verifying the hash is the whole point of the step.

The central then sends its own 52 bytes `next_challenge`, stamped with state `02`, wrapping 16 **random** bytes and using a fresh nonce.
These are the bytes the device will have to hand back.

##### Step 2, the device answers

The device notifies `02 00 00 00` and puts 20 bytes into `SFIDA_TO_CENTRAL`:
the 4 bytes state followed by the 16 bytes the central just sent, decrypted and in the clear.

Comparing them with what was sent is what authenticates the *device*, the earlier steps only authenticated the central.

##### Step 3, the final challenge

The last message wraps a fixed 16 bytes marker, the ASCII string `PokemonGoooooooo`, in a `next_challenge` keyed with the session key.
Its state field goes back to `00` rather than continuing to count up.

| Position | 00 | 01 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 10 | 11 | 12 | 13 | 14 | 15 |
| -------- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- |
| Value    | 50 | 6f | 6b | 65 | 6d | 6f | 6e | 47 | 6f | 6f | 6f | 6f | 6f | 6f | 6f | 6f |

The device acknowledges with a `04 00 23 00` notification, and certification is complete.
This is the one step that is not always announced beforehand, so a central that waits passively for a notification here
can hang: reading `SFIDA_TO_CENTRAL` on its own after a short delay is more reliable.

From that point on, the battery, LED, vibration and button characteristics answer normally.
Services discovered *before* certification completed may have been cached as empty,
so it is worth running the discovery of the battery and device control services only once the handshake is done.

Nothing survives the connection: the session key is generated by the device for that connection only, and the whole handshake runs again on the next one.

#### Reconnection

A device that already knows the central takes a shorter path.
Instead of the 378 bytes challenge, subscribing to `SFIDA_COMMANDS` produces a `03 00 00 00` notification
and a 36 bytes payload, 4 bytes of state followed by a 32 bytes reconnect challenge.

The central answers it with 16 bytes computed as `AES(session_key, challenge[0:15]) ^ challenge[16:31]`, and the exchange
then runs through states `04 00 01 00`, `05` and `04 00 02 00` before the device considers the central certified.

How the reconnect key itself is established and stored is not documented here, and Lighthouse does not implement this path:
it runs the full handshake on every connection, which the device always accepts.

## Using the device

#### Device name

A read request to the `device name` characteristic will return 15 bytes of data, for example `0x506f6b656d6f6e20474f20506c7573`.

| Position | 00 | 01 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 10 | 11 | 12 | 13 | 14 |
| -------- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- |
| Value    | 50 | 6f | 6b | 65 | 6d | 6f | 6e | 20 | 47 | 4f | 20 | 50 | 6c | 75 | 73 |

| Bytes | Type          | Value             | Description                       |
| ----- | ------------- | ----------------- | --------------------------------- |
| all   | ASCII text    | Pokemon GO Plus   | device name                       |

#### Preferred connection parameters

A read request to the `preferred connection parameters` characteristic will return 8 bytes of data, for example `0x0800100000006400`.

| Position | 00 | 01 | 02 | 03 | 04 | 05 | 06 | 07 |
| -------- | -- | -- | -- | -- | -- | -- | -- | -- |
| Value    | 08 | 00 | 10 | 00 | 00 | 00 | 64 | 00 |

| Bytes | Type       | Value | Description                                      |
| ----- | ---------- | ----- | ------------------------------------------------ |
| 00-01 | uint16_le  | 8     | minimum connection interval, in 1.25 ms          |
| 02-03 | uint16_le  | 16    | maximum connection interval, in 1.25 ms          |
| 04-05 | uint16_le  | 0     | slave latency                                    |
| 06-07 | uint16_le  | 100   | supervision timeout, in 10 ms                    |

The device asks for a 10 to 20 ms connection interval and a 1 second supervision timeout.

#### Battery level

> Using the `Battery` service requires a successful certification process.

A read request to the `battery level` characteristic will return 1 byte of data.

| Bytes | Type       | Value | Description                                      |
| ----- | ---------- | ----- | ------------------------------------------------ |
| 00    | uint8      | 0-100 | battery level in %                               |

This read fails with `Insufficient Authorization` until certification has completed.

#### LED and vibration

> Using the `Device control` service requires a successful certification process.

Writing to the `LED and vibration control` characteristic plays a pattern sequence.
The payload is a 4 bytes header followed by 0 to 31 patterns of 3 bytes each, sent in a single write.

The whole sequence is transmitted at once and played back immediately, as long as it is valid.
The write must be a *Write Request*, not a *Write Command*: the device answers it, and the answer is how you know whether the sequence was accepted.

Two things make a sequence invalid and get it silently ignored:

* the `patterns` count in the header does not match the number of pattern packets actually sent
* a sequence is already playing and its priority is higher than the one being sent

The bits in the following tables are numbered from high to low.

##### Header

| Field         | Byte Offset   | Width | Description                           |
| ------------- | ------------- | ----- | ------------------------------------- |
| reserved0     | 0             | [7:0] | Set to zero. Was thought to be input delay in m50 format. |
| reserved1     | 1             | [7:0] | Set to zero.                          |
| reserved2     | 2             | [7:0] | Set to zero.                          |
| priority      | 3             | [7:5] | The message priority, in `0..7`.      |
| patterns      | 3             | [4:0] | The number of patterns, in `0..31`. It must match the number of pattern packets that follow, or the whole write is ignored. |

A new sequence is only played if its `priority` is greater than or equal to the priority of the sequence currently playing. Every sequence sent by the official game uses priority 0.

To cancel a sequence in progress, send a header with `patterns` set to 0, no pattern packet at all,
and a priority greater than or equal to the one of the sequence being cancelled.

##### Pattern

| Field         | Byte Offset   | Width | Description                           |
| ------------- | ------------- | ----- | ------------------------------------- |
| duration      | 0             | [7:0] | The period of the pattern, in `m50`   |
| g             | 1             | [7:4] | The green level, in `0..15`           |
| r             | 1             | [3:0] | The red level, in `0..15`             |
| interpolate   | 2             | [7:7] | Interpolate the LED color from the previous pattern, using 9 bit PWM. Applies to the color only, never to the vibration. `0`: hard switch, `1`: fade in from the previous pattern. |
| vibration     | 2             | [6:4] | Vibration, in `0..7`. The motor is on/off only and the intensity is *not* adjustable: any non-zero value turns it on for the duration of the pattern. |
| b             | 2             | [3:0] | The blue level, in `0..15`            |

A level of `0` turns a color channel off, `15` is the maximum intensity.

###### duration formatting

Durations are formatted in a scheme called `m50`, which is a byte value representing 50-millisecond intervals.
This gives a maximum duration of `50 * 255 = 12750 ms` per pattern.

##### Example

A short sequence captured from the official application, `0x000000010a0000`:

| Position | 00 | 01 | 02 | 03 | 04 | 05 | 06 |
| -------- | -- | -- | -- | -- | -- | -- | -- |
| Value    | 00 | 00 | 00 | 01 | 0a | 00 | 00 |

| Bytes | Type      | Value     | Description                                   |
| ----- | --------- | --------- | --------------------------------------------- |
| 00-02 | ?         | 0         | reserved                                      |
| 03    | bits      | 0 / 1     | priority 0, one pattern follows               |
| 04    | m50       | 10        | pattern duration, 500 ms                      |
| 05    | nibble    | 0 / 0     | green 0, red 0                                |
| 06    | bits      | 0         | no interpolation, no vibration, blue 0        |

A pattern with everything off is how the game keeps the device quiet for a while.

Another single pattern sequence, `0x0000000114 0f17`, this one actually visible:

| Position | 00 | 01 | 02 | 03 | 04 | 05 | 06 |
| -------- | -- | -- | -- | -- | -- | -- | -- |
| Value    | 00 | 00 | 00 | 01 | 14 | 0f | 17 |

| Bytes | Type      | Value     | Description                                   |
| ----- | --------- | --------- | --------------------------------------------- |
| 00-02 | ?         | 0         | reserved                                      |
| 03    | bits      | 0 / 1     | priority 0, one pattern follows               |
| 04    | m50       | 20        | pattern duration, 1000 ms                     |
| 05    | nibble    | 0 / 15    | green 0, red 15                               |
| 06    | bits      | 0/1/7     | no interpolation, vibration on, blue 7        |

One second of pink LED with the motor running.

A longer one, `0x0000000f10f0f00800001 0f0f008...`, declares 15 patterns and totals 30 seconds.

#### Button notification

> Subscribing to the `button press notification` characteristic requires a successful certification process.

The device only notifies **while a sequence is playing**.  
Outside of a sequence, the button state is simply not reported, which is why the official game keeps a long,
mostly invisible sequence running whenever it wants to catch a button press.  

While a sequence is in progress, the button is sampled every 50 ms and the device sends 2 bytes every 500 ms,
carrying the last ten samples right-aligned and zero-padded.

| Field       | Byte Offset | Width | Description                                     |
| ----------- | ----------- | ----- | ----------------------------------------------- |
| reserved    | 0           | [7:2] | All zeros so far, but a GO+ 2 could use them    |
| sample 1    | 0           | [1:1] | Oldest sample                                   |
| sample 2    | 0           | [0:0] |                                                 |
| samples 3-9 | 1           | [7:1] |                                                 |
| sample 10   | 1           | [0:0] | Newest sample                                   |

Each sample is `0` when the button was not pressed and `1` when it was.
Sample 1 is the oldest of the batch, sample 10 the most recent.

Pressing the button does **not** cancel the sequence in progress. If that is the wanted behavior,
the central has to cancel the sequence itself, see the header description above.

## Reference

[1] https://github.com/fbrzlarosa/porygon  
[2] https://web.archive.org/web/20170119170928/https://github.com/numinit/porygon/wiki/protocol  
[3] https://www.ifixit.com/Teardown/Pokemon+Go+Plus+Teardown/67462  
[4] https://tinyhack.com/2018/11/21/reverse-engineering-pokemon-go-plus/  
[5] https://github.com/yohanes/pgpemu/  
[6] https://coderjesus.com/blog/pgp-suota/  
[7] https://github.com/Jesus805/Suota-Go-Plus  
[8] https://www.reddit.com/r/pokemongodev/comments/7cba9t/controlling_the_pokemon_go_plus_led_and_button/  
[9] https://www.fortinet.com/blog/threat-research/pokemon-go-plus-preview-through-reverse-engineering  

## License

This documentation is licensed under the MIT license.

> Copyright (c) 2026 Emeric Grange <emeric.grange@gmail.com>
