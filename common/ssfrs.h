/* --------------------------------------------------------------------------------------------- */
/* Small System Framework                                                                        */
/*                                                                                               */
/* ssfrs.h                                                                                       */
/* Provides Reed-Solomon FEC encoder/decoder interface.                                          */
/*                                                                                               */
/* Limitations:                                                                                  */
/*     Decode interface does not support erasure corrections, only error corrections.            */
/*     Eraseure corrections are only useful if the location of an error is known.                */
/*                                                                                               */
/* Reed-Solomon algorithms and code inspired and adapted from:                                   */
/*     https://en.wikiversity.org/wiki/Reed-Solomon_codes_for_coders                             */
/*                                                                                               */
/* MOD255 macro algorithm inspired and adapted from:                                             */
/*     http://homepage.cs.uiowa.edu/~jones/bcd/mod.shtml                                         */
/*                                                                                               */
/* BSD-3-Clause License                                                                          */
/* Copyright 2021 Supurloop Software LLC                                                         */
/*                                                                                               */
/* Redistribution and use in source and binary forms, with or without modification, are          */
/* permitted provided that the following conditions are met:                                     */
/*                                                                                               */
/* 1. Redistributions of source code must retain the above copyright notice, this list of        */
/* conditions and the following disclaimer.                                                      */
/* 2. Redistributions in binary form must reproduce the above copyright notice, this list of     */
/* conditions and the following disclaimer in the documentation and/or other materials provided  */
/* with the distribution.                                                                        */
/* 3. Neither the name of the copyright holder nor the names of its contributors may be used to  */
/* endorse or promote products derived from this software without specific prior written         */
/* permission.                                                                                   */
/*                                                                                               */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS   */
/* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF               */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE    */
/* COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL      */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE */
/* GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED    */
/* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING     */
/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED  */
/* OF THE POSSIBILITY OF SUCH DAMAGE.                                                            */
/* --------------------------------------------------------------------------------------------- */
#ifndef SSF_RS_INCLUDE_H
#define SSF_RS_INCLUDE_H

#include <stdint.h>
#include <stdbool.h>

/* --------------------------------------------------------------------------------------------- */
/* Configure ssfrs's Reed-Solomon interface                                                      */
/* --------------------------------------------------------------------------------------------- */
/* The maximum total size in bytes of a message to be encoded or decoded */
#define FRAME_SSF_RS_MAX_MESSAGE_SIZE 1016
/* The maximum number of bytes that will be encoded with up to SSF_RS_MAX_SYMBOLS bytes */
#define FRAME_SSF_RS_MAX_CHUNK_SIZE 253
/* The maximum number of symbols in bytes that will encode up to SSF_RS_MAX_CHUNK_SIZE bytes */
/* Reed-Solomon can correct SSF_RS_MAX_SYMBOLS/2 bytes with errors in a message */
#define FRAME_SSF_RS_MAX_SYMBOLS 253
#define FRAME_SSF_RS_MIN_SYMBOLS 2
#define FRAME_SSF_RS_DEFAULT_SIZE FRAME_SSF_RS_MIN_SYMBOLS
/* For now we are limiting the total of chunk bytes and symbols to 255 max */
/* 2^8-1 , n^m-1 */
#define FRAME_SSF_RS_TOTAL_CHUNK_LIMIT 255
/* For now we are limiting the total encoded msg + ecc to 1024 bytes */
#define FRAME_SSF_RS_TOTAL_LIMIT 1024
/* Error codes */
#define FRAME_SSF_ERROR_MAX_MSG_SIZE 0x01
#define FRAME_SSF_ERROR_MAX_CHUNK_SIZE 0x02
#define FRAME_SSF_ERROR_MAX_RS_SIZE 0x04
#define FRAME_SSF_ERROR_CHUNK_LIMIT 0x08
#define FRAME_SSF_ERROR_TOTAL_LIMIT 0x10

/* 1 to enable Reed-Solomon encoding interface, else 0. */
#define SSF_RS_ENABLE_ENCODING (1u)

/* 1 to enable Reed-Solomon decoding interface, else 0. */
#define SSF_RS_ENABLE_DECODING (1u)

/* The maximum total size in bytes of a message to be encoded or decoded */
#define SSF_RS_MAX_MESSAGE_SIZE (223)
#if SSF_RS_MAX_MESSAGE_SIZE > 2048
#error SSFRS invalid SSF_RS_MAX_MESSAGE_SIZE.
#endif

/* The maximum number of bytes that will be encoded with up to SSF_RS_MAX_SYMBOLS bytes */
#define SSF_RS_MAX_CHUNK_SIZE (231u)
#if SSF_RS_MAX_CHUNK_SIZE > 253
#error SSFRS invalid SSF_RS_MAX_CHUNK_SIZE.
#endif

/* The maximum number of chunks that a message will be broken up into for encoding and decoding */
#if SSF_RS_MAX_MESSAGE_SIZE % SSF_RS_MAX_CHUNK_SIZE == 0
#define SSF_RS_MAX_CHUNKS (SSF_RS_MAX_MESSAGE_SIZE / SSF_RS_MAX_CHUNK_SIZE)
#else
#define SSF_RS_MAX_CHUNKS ((SSF_RS_MAX_MESSAGE_SIZE / SSF_RS_MAX_CHUNK_SIZE) + 1)
#endif

/* The maximum number of symbols in bytes that will encode up to SSF_RS_MAX_CHUNK_SIZE bytes */
/* Reed-Solomon can correct SSF_RS_MAX_SYMBOLS/2 bytes with errors in a message */
#define SSF_RS_MAX_SYMBOLS (24ul)
#if (SSF_RS_MAX_SYMBOLS < 2) || (SSF_RS_MAX_SYMBOLS > 254)
#error SSFRS Invalid SSF_RS_MAX_SYMBOLS.
#endif

/* For now we are limiting the total of chunk bytes and symbols to 255 max */
#if SSF_RS_MAX_CHUNK_SIZE + SSF_RS_MAX_SYMBOLS > 255
#error SSFRS total of SSF_RS_MAX_CHUNK_SIZE + SSF_RS_MAX_SYMBOLS not supported.
#endif

/* For now we are limiting the total of chunk bytes and symbols to 255 max */
#if FRAME_SSF_RS_TOTAL_CHUNK_LIMIT > 255
#error SSFRS total size of chunk over limit, for GF(2^8) is 255 max block size
#endif

/* For now we are limiting the total encoded msg + ecc to 61440 (60KiB) */
#if (SSF_RS_MAX_SYMBOLS * SSF_RS_MAX_CHUNKS) + SSF_RS_MAX_MESSAGE_SIZE > 61440ul
#error SSFRS total of SSF_RS_MAX_CHUNK_SIZE + SSF_RS_MAX_SYMBOLS not supported.
#endif

/* --------------------------------------------------------------------------------------------- */
/* External interface                                                                            */
/* --------------------------------------------------------------------------------------------- */
void SSFRS_Encode(const uint8_t *msg, uint16_t msgLen, uint8_t* eccBuf, uint16_t eccBufSize,
                 uint16_t *eccBufLen, uint8_t eccNumBytes, uint8_t chunkSize);
bool SSFRS_Decode(uint8_t *msg, uint16_t msgSize, uint16_t *msgLen, uint8_t chunkSyms,
                 uint8_t chunkSize);
uint8_t SSFRS_Init(uint16_t max_msg_size, uint16_t max_chunk_size, uint16_t max_rs_symbols);
uint16_t SSFRS_GetMsgSize(void);
uint16_t SSFRS_GetRSSize(void);
uint16_t SSFRS_GetMaxChunks(void);
uint16_t SSFRS_GetChunkSize(void);

#endif /* SSF_RS_INCLUDE_H */
