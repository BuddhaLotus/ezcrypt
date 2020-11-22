/*
 *  sha512.c
 *
 *  Written by Jari Ruusu, April 16 2001
 *
 *  Copyright 2001 by Jari Ruusu.
 *  Redistribution of this file is permitted under the GNU Public License.
 */

#include <string.h>
#include <sys/types.h>
#include "sha512.h"

/* Define one or more of these. If none is defined, you get all of them */
#if !defined(SHA256_NEEDED)&&!defined(SHA512_NEEDED)&&!defined(SHA384_NEEDED)
# define SHA256_NEEDED  1
# define SHA512_NEEDED  1
# define SHA384_NEEDED  1
#endif

#if defined(SHA256_NEEDED)
static const u_int32_t sha256_hashInit[8] = {
    0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c,
    0x1f83d9ab, 0x5be0cd19
};
static const u_int32_t sha256_K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
    0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
    0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
    0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
    0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
    0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};
#endif

#if defined(SHA512_NEEDED)
static const u_int64_t sha512_hashInit[8] = {
    0x6a09e667f3bcc908UL, 0xbb67ae8584caa73bUL, 0x3c6ef372fe94f82bUL,
    0xa54ff53a5f1d36f1UL, 0x510e527fade682d1UL, 0x9b05688c2b3e6c1fUL,
    0x1f83d9abfb41bd6bUL, 0x5be0cd19137e2179UL
};
#endif

#if defined(SHA384_NEEDED)
static const u_int64_t sha384_hashInit[8] = {
    0xcbbb9d5dc1059ed8UL, 0x629a292a367cd507UL, 0x9159015a3070dd17UL,
    0x152fecd8f70e5939UL, 0x67332667ffc00b31UL, 0x8eb44a8768581511UL,
    0xdb0c2e0d64f98fa7UL, 0x47b5481dbefa4fa4UL
};
#endif

#if defined(SHA512_NEEDED) || defined(SHA384_NEEDED)
static const u_int64_t sha512_K[80] = {
    0x428a2f98d728ae22UL, 0x7137449123ef65cdUL, 0xb5c0fbcfec4d3b2fUL,
    0xe9b5dba58189dbbcUL, 0x3956c25bf348b538UL, 0x59f111f1b605d019UL,
    0x923f82a4af194f9bUL, 0xab1c5ed5da6d8118UL, 0xd807aa98a3030242UL,
    0x12835b0145706fbeUL, 0x243185be4ee4b28cUL, 0x550c7dc3d5ffb4e2UL,
    0x72be5d74f27b896fUL, 0x80deb1fe3b1696b1UL, 0x9bdc06a725c71235UL,
    0xc19bf174cf692694UL, 0xe49b69c19ef14ad2UL, 0xefbe4786384f25e3UL,
    0x0fc19dc68b8cd5b5UL, 0x240ca1cc77ac9c65UL, 0x2de92c6f592b0275UL,
    0x4a7484aa6ea6e483UL, 0x5cb0a9dcbd41fbd4UL, 0x76f988da831153b5UL,
    0x983e5152ee66dfabUL, 0xa831c66d2db43210UL, 0xb00327c898fb213fUL,
    0xbf597fc7beef0ee4UL, 0xc6e00bf33da88fc2UL, 0xd5a79147930aa725UL,
    0x06ca6351e003826fUL, 0x142929670a0e6e70UL, 0x27b70a8546d22ffcUL,
    0x2e1b21385c26c926UL, 0x4d2c6dfc5ac42aedUL, 0x53380d139d95b3dfUL,
    0x650a73548baf63deUL, 0x766a0abb3c77b2a8UL, 0x81c2c92e47edaee6UL,
    0x92722c851482353bUL, 0xa2bfe8a14cf10364UL, 0xa81a664bbc423001UL,
    0xc24b8b70d0f89791UL, 0xc76c51a30654be30UL, 0xd192e819d6ef5218UL,
    0xd69906245565a910UL, 0xf40e35855771202aUL, 0x106aa07032bbd1b8UL,
    0x19a4c116b8d2d0c8UL, 0x1e376c085141ab53UL, 0x2748774cdf8eeb99UL,
    0x34b0bcb5e19b48a8UL, 0x391c0cb3c5c95a63UL, 0x4ed8aa4ae3418acbUL,
    0x5b9cca4f7763e373UL, 0x682e6ff3d6b2b8a3UL, 0x748f82ee5defb2fcUL,
    0x78a5636f43172f60UL, 0x84c87814a1f0ab72UL, 0x8cc702081a6439ecUL,
    0x90befffa23631e28UL, 0xa4506cebde82bde9UL, 0xbef9a3f7b2c67915UL,
    0xc67178f2e372532bUL, 0xca273eceea26619cUL, 0xd186b8c721c0c207UL,
    0xeada7dd6cde0eb1eUL, 0xf57d4f7fee6ed178UL, 0x06f067aa72176fbaUL,
    0x0a637dc5a2c898a6UL, 0x113f9804bef90daeUL, 0x1b710b35131c471bUL,
    0x28db77f523047d84UL, 0x32caab7b40c72493UL, 0x3c9ebe0a15c9bebcUL,
    0x431d67c49c100d4cUL, 0x4cc5d4becb3e42b6UL, 0x597f299cfc657e2aUL,
    0x5fcb6fab3ad6faecUL, 0x6c44198c4a475817UL
};
#endif

#define Ch(x,y,z)   (((x) & (y)) ^ ((~(x)) & (z)))
#define Maj(x,y,z)  (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define R(x,y)      ((y) >> (x))

#if defined(SHA256_NEEDED)
void sha256_init(sha256_context *ctx)
{
    memcpy(&ctx->sha_H[0], &sha256_hashInit[0], sizeof(ctx->sha_H));
    ctx->sha_blocks = 0;
    ctx->sha_bufCnt = 0;
}

#define S(x,y)      (((y) >> (x)) | ((y) << (32 - (x))))
#define uSig0(x)    ((S(2,(x))) ^ (S(13,(x))) ^ (S(22,(x))))
#define uSig1(x)    ((S(6,(x))) ^ (S(11,(x))) ^ (S(25,(x))))
#define lSig0(x)    ((S(7,(x))) ^ (S(18,(x))) ^ (R(3,(x))))
#define lSig1(x)    ((S(17,(x))) ^ (S(19,(x))) ^ (R(10,(x))))

static void sha256_transform(sha256_context *ctx, unsigned char *datap)
{
    register int    j;
    u_int32_t       a, b, c, d, e, f, g, h;
    u_int32_t       T1, T2, W[64], Wm2, Wm15;

    /* read the data, big endian byte order */
    j = 0;
    do {
        W[j] = (((u_int32_t)(datap[0]))<<24) | (((u_int32_t)(datap[1]))<<16) |
               (((u_int32_t)(datap[2]))<<8 ) | ((u_int32_t)(datap[3]));
        datap += 4;
    } while(++j < 16);

    /* initialize variables a...h */
    a = ctx->sha_H[0];
    b = ctx->sha_H[1];
    c = ctx->sha_H[2];
    d = ctx->sha_H[3];
    e = ctx->sha_H[4];
    f = ctx->sha_H[5];
    g = ctx->sha_H[6];
    h = ctx->sha_H[7];

    /* apply compression function */
    j = 0;
    do {
        if(j >= 16) {
            Wm2 = W[j - 2];
            Wm15 = W[j - 15];
            W[j] = lSig1(Wm2) + W[j - 7] + lSig0(Wm15) + W[j - 16];
        }
        T1 = h + uSig1(e) + Ch(e,f,g) + sha256_K[j] + W[j];
        T2 = uSig0(a) + Maj(a,b,c);
        h = g; g = f; f = e;
        e = d + T1;
        d = c; c = b; b = a;
        a = T1 + T2;
    } while(++j < 64);

    /* compute intermediate hash value */
    ctx->sha_H[0] += a;
    ctx->sha_H[1] += b;
    ctx->sha_H[2] += c;
    ctx->sha_H[3] += d;
    ctx->sha_H[4] += e;
    ctx->sha_H[5] += f;
    ctx->sha_H[6] += g;
    ctx->sha_H[7] += h;

    ctx->sha_blocks++;
}

void sha256_write(sha256_context *ctx, unsigned char *datap, int length)
{
    while(length > 0) {
        if(!ctx->sha_bufCnt) {
            while(length >= sizeof(ctx->sha_out)) {
                sha256_transform(ctx, datap);
                datap += sizeof(ctx->sha_out);
                length -= sizeof(ctx->sha_out);
            }
            if(!length) return;
        }
        ctx->sha_out[ctx->sha_bufCnt] = *datap++;
        length--;
        if(++ctx->sha_bufCnt == sizeof(ctx->sha_out)) {
            sha256_transform(ctx, &ctx->sha_out[0]);
            ctx->sha_bufCnt = 0;
        }
    }
}

void sha256_final(sha256_context *ctx)
{
    register int    j;
    u_int64_t       bitLength;
    u_int32_t       i;
    unsigned char   padByte, *datap;

    bitLength = (ctx->sha_blocks << 9) | (ctx->sha_bufCnt << 3);
    padByte = 0x80;
    sha256_write(ctx, &padByte, 1);

    /* pad extra space with zeroes */
    padByte = 0;
    while(ctx->sha_bufCnt != 56) {
        sha256_write(ctx, &padByte, 1);
    }

    /* write bit length, big endian byte order */
    ctx->sha_out[56] = (unsigned char)(bitLength >> 56);
    ctx->sha_out[57] = (unsigned char)(bitLength >> 48);
    ctx->sha_out[58] = (unsigned char)(bitLength >> 40);
    ctx->sha_out[59] = (unsigned char)(bitLength >> 32);
    ctx->sha_out[60] = (unsigned char)(bitLength >> 24);
    ctx->sha_out[61] = (unsigned char)(bitLength >> 16);
    ctx->sha_out[62] = (unsigned char)(bitLength >> 8);
    ctx->sha_out[63] = (unsigned char)(bitLength);
    sha256_transform(ctx, &ctx->sha_out[0]);

    /* return results in ctx->sha_out[0...31] */
    datap = &ctx->sha_out[0];
    j = 0;
    do {
        i = ctx->sha_H[j];
        datap[0] = (unsigned char)(i >> 24);
        datap[1] = (unsigned char)(i >> 16);
        datap[2] = (unsigned char)(i >> 8);
        datap[3] = (unsigned char)(i);
        datap += 4;
    } while(++j < 8);

    /* clear sensitive information */
    memset(&ctx->sha_out[32], 0, sizeof(sha256_context) - 32);
}

void sha256_hash_buffer(unsigned char *ib, int ile, unsigned char *ob, int ole)
{
    sha256_context ctx;

    if(ole < 1) return;
    memset(ob, 0, ole);
    if(ole > 32) ole = 32;
    sha256_init(&ctx);
    sha256_write(&ctx, ib, ile);
    sha256_final(&ctx);
    memcpy(ob, &ctx.sha_out[0], ole);
    memset(&ctx, 0, sizeof(ctx));
}

#endif

#if defined(SHA512_NEEDED)
void sha512_init(sha512_context *ctx)
{
    memcpy(&ctx->sha_H[0], &sha512_hashInit[0], sizeof(ctx->sha_H));
    ctx->sha_blocks = 0;
    ctx->sha_blocksMSB = 0;
    ctx->sha_bufCnt = 0;
}
#endif

#if defined(SHA512_NEEDED) || defined(SHA384_NEEDED)
#undef S
#undef uSig0
#undef uSig1
#undef lSig0
#undef lSig1
#define S(x,y)      (((y) >> (x)) | ((y) << (64 - (x))))
#define uSig0(x)    ((S(28,(x))) ^ (S(34,(x))) ^ (S(39,(x))))
#define uSig1(x)    ((S(14,(x))) ^ (S(18,(x))) ^ (S(41,(x))))
#define lSig0(x)    ((S(1,(x))) ^ (S(8,(x))) ^ (R(7,(x))))
#define lSig1(x)    ((S(19,(x))) ^ (S(61,(x))) ^ (R(6,(x))))

static void sha512_transform(sha512_context *ctx, unsigned char *datap)
{
    register int    j;
    u_int64_t       a, b, c, d, e, f, g, h;
    u_int64_t       T1, T2, W[80], Wm2, Wm15;

    /* read the data, big endian byte order */
    j = 0;
    do {
        W[j] = (((u_int64_t)(datap[0]))<<56) | (((u_int64_t)(datap[1]))<<48) |
               (((u_int64_t)(datap[2]))<<40) | (((u_int64_t)(datap[3]))<<32) |
               (((u_int64_t)(datap[4]))<<24) | (((u_int64_t)(datap[5]))<<16) |
               (((u_int64_t)(datap[6]))<<8 ) | ((u_int64_t)(datap[7]));
        datap += 8;
    } while(++j < 16);

    /* initialize variables a...h */
    a = ctx->sha_H[0];
    b = ctx->sha_H[1];
    c = ctx->sha_H[2];
    d = ctx->sha_H[3];
    e = ctx->sha_H[4];
    f = ctx->sha_H[5];
    g = ctx->sha_H[6];
    h = ctx->sha_H[7];

    /* apply compression function */
    j = 0;
    do {
        if(j >= 16) {
            Wm2 = W[j - 2];
            Wm15 = W[j - 15];
            W[j] = lSig1(Wm2) + W[j - 7] + lSig0(Wm15) + W[j - 16];
        }
        T1 = h + uSig1(e) + Ch(e,f,g) + sha512_K[j] + W[j];
        T2 = uSig0(a) + Maj(a,b,c);
        h = g; g = f; f = e;
        e = d + T1;
        d = c; c = b; b = a;
        a = T1 + T2;
    } while(++j < 80);

    /* compute intermediate hash value */
    ctx->sha_H[0] += a;
    ctx->sha_H[1] += b;
    ctx->sha_H[2] += c;
    ctx->sha_H[3] += d;
    ctx->sha_H[4] += e;
    ctx->sha_H[5] += f;
    ctx->sha_H[6] += g;
    ctx->sha_H[7] += h;

    ctx->sha_blocks++;
    if(!ctx->sha_blocks) ctx->sha_blocksMSB++;
}

void sha512_write(sha512_context *ctx, unsigned char *datap, int length)
{
    while(length > 0) {
        if(!ctx->sha_bufCnt) {
            while(length >= sizeof(ctx->sha_out)) {
                sha512_transform(ctx, datap);
                datap += sizeof(ctx->sha_out);
                length -= sizeof(ctx->sha_out);
            }
            if(!length) return;
        }
        ctx->sha_out[ctx->sha_bufCnt] = *datap++;
        length--;
        if(++ctx->sha_bufCnt == sizeof(ctx->sha_out)) {
            sha512_transform(ctx, &ctx->sha_out[0]);
            ctx->sha_bufCnt = 0;
        }
    }
}

void sha512_final(sha512_context *ctx)
{
    register int    j;
    u_int64_t       bitLength, bitLengthMSB;
    u_int64_t       i;
    unsigned char   padByte, *datap;

    bitLength = (ctx->sha_blocks << 10) | (ctx->sha_bufCnt << 3);
    bitLengthMSB = (ctx->sha_blocksMSB << 10) | (ctx->sha_blocks >> 54);
    padByte = 0x80;
    sha512_write(ctx, &padByte, 1);

    /* pad extra space with zeroes */
    padByte = 0;
    while(ctx->sha_bufCnt != 112) {
        sha512_write(ctx, &padByte, 1);
    }

    /* write bit length, big endian byte order */
    ctx->sha_out[112] = (unsigned char)(bitLengthMSB >> 56);
    ctx->sha_out[113] = (unsigned char)(bitLengthMSB >> 48);
    ctx->sha_out[114] = (unsigned char)(bitLengthMSB >> 40);
    ctx->sha_out[115] = (unsigned char)(bitLengthMSB >> 32);
    ctx->sha_out[116] = (unsigned char)(bitLengthMSB >> 24);
    ctx->sha_out[117] = (unsigned char)(bitLengthMSB >> 16);
    ctx->sha_out[118] = (unsigned char)(bitLengthMSB >> 8);
    ctx->sha_out[119] = (unsigned char)(bitLengthMSB);
    ctx->sha_out[120] = (unsigned char)(bitLength >> 56);
    ctx->sha_out[121] = (unsigned char)(bitLength >> 48);
    ctx->sha_out[122] = (unsigned char)(bitLength >> 40);
    ctx->sha_out[123] = (unsigned char)(bitLength >> 32);
    ctx->sha_out[124] = (unsigned char)(bitLength >> 24);
    ctx->sha_out[125] = (unsigned char)(bitLength >> 16);
    ctx->sha_out[126] = (unsigned char)(bitLength >> 8);
    ctx->sha_out[127] = (unsigned char)(bitLength);
    sha512_transform(ctx, &ctx->sha_out[0]);

    /* return results in ctx->sha_out[0...63] */
    datap = &ctx->sha_out[0];
    j = 0;
    do {
        i = ctx->sha_H[j];
        datap[0] = (unsigned char)(i >> 56);
        datap[1] = (unsigned char)(i >> 48);
        datap[2] = (unsigned char)(i >> 40);
        datap[3] = (unsigned char)(i >> 32);
        datap[4] = (unsigned char)(i >> 24);
        datap[5] = (unsigned char)(i >> 16);
        datap[6] = (unsigned char)(i >> 8);
        datap[7] = (unsigned char)(i);
        datap += 8;
    } while(++j < 8);

    /* clear sensitive information */
    memset(&ctx->sha_out[64], 0, sizeof(sha512_context) - 64);
}

void sha512_hash_buffer(unsigned char *ib, int ile, unsigned char *ob, int ole)
{
    sha512_context ctx;

    if(ole < 1) return;
    memset(ob, 0, ole);
    if(ole > 64) ole = 64;
    sha512_init(&ctx);
    sha512_write(&ctx, ib, ile);
    sha512_final(&ctx);
    memcpy(ob, &ctx.sha_out[0], ole);
    memset(&ctx, 0, sizeof(ctx));
}
#endif

#if defined(SHA384_NEEDED)
void sha384_init(sha512_context *ctx)
{
    memcpy(&ctx->sha_H[0], &sha384_hashInit[0], sizeof(ctx->sha_H));
    ctx->sha_blocks = 0;
    ctx->sha_blocksMSB = 0;
    ctx->sha_bufCnt = 0;
}

void sha384_hash_buffer(unsigned char *ib, int ile, unsigned char *ob, int ole)
{
    sha512_context ctx;

    if(ole < 1) return;
    memset(ob, 0, ole);
    if(ole > 48) ole = 48;
    sha384_init(&ctx);
    sha512_write(&ctx, ib, ile);
    sha512_final(&ctx);
    memcpy(ob, &ctx.sha_out[0], ole);
    memset(&ctx, 0, sizeof(ctx));
}
#endif
