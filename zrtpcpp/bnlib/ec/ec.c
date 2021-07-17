/*
 * Copyright 2006 - 2018, Werner Dittmann
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <bn.h>
#include <bnprint.h>

#include <ec/ec.h>

static BigNum _mpiZero;
static BigNum _mpiOne;
static BigNum _mpiTwo;
static BigNum _mpiThree;
static BigNum _mpiFour;
static BigNum _mpiEight;

static BigNum* mpiZero  = &_mpiZero;
static BigNum* mpiOne   = &_mpiOne;
static BigNum* mpiTwo   = &_mpiTwo;
static BigNum* mpiThree = &_mpiThree;
static BigNum* mpiFour  = &_mpiFour;
static BigNum* mpiEight = &_mpiEight;
static int initialized = 0;

void ecDestroyStatic(void) {
	if (initialized) {
		initialized = 0;
		bnEnd(&_mpiZero);
		bnEnd(&_mpiOne);
		bnEnd(&_mpiTwo);
		bnEnd(&_mpiThree);
		bnEnd(&_mpiFour);
		bnEnd(&_mpiEight);
	}
}


/* The following parameters are given:
 - The prime modulus p
 - The order n
 - The 160-bit input seed SEED to the SHA-1 based algorithm (i.e., the domain parameter seed)
 - The output c of the SHA-1 based algorithm
 - The coefficient b (satisfying b2 c ≡ –27 (mod p))
 - The base point x coordinate Gx
 - The base point y coordinate Gy
*/

typedef struct _curveData {
    char *p;
    char *n;
    char *SEED;
    char *c;
    char *b;
    char *Gx;
    char *Gy;
} curveData;

static curveData nist192 = {
    "6277101735386680763835789423207666416083908700390324961279",
    "6277101735386680763835789423176059013767194773182842284081",
    "3045ae6fc8422f64ed579528d38120eae12196d5",
    "3099d2bbbfcb2538542dcd5fb078b6ef5f3d6fe2c745de65",
    "64210519e59c80e70fa7e9ab72243049feb8deecc146b9b1",
    "188da80eb03090f67cbf20eb43a18800f4ff0afd82ff1012",
    "07192b95ffc8da78631011ed6b24cdd573f977a11e794811",
};

static curveData nist224 = {
    "26959946667150639794667015087019630673557916260026308143510066298881",
    "26959946667150639794667015087019625940457807714424391721682722368061",
    "bd71344799d5c7fcdc45b59fa3b9ab8f6a948bc5",
    "5b056c7e11dd68f40469ee7f3c7a7d74f7d121116506d031218291fb",
    "b4050a850c04b3abf54132565044b0b7d7bfd8ba270b39432355ffb4",
    "b70e0cbd6bb4bf7f321390b94a03c1d356c21122343280d6115c1d21",
    "bd376388b5f723fb4c22dfe6cd4375a05a07476444d5819985007e34",
};

static curveData nist256 = {
    "115792089210356248762697446949407573530086143415290314195533631308867097853951",
    "115792089210356248762697446949407573529996955224135760342422259061068512044369",
    "c49d360886e704936a6678e1139d26b7819f7e90",
    "7efba1662985be9403cb055c75d4f7e0ce8d84a9c5114abcaf3177680104fa0d",
    "5ac635d8aa3a93e7b3ebbd55769886bc651d06b0cc53b0f63bce3c3e27d2604b",
    "6b17d1f2e12c4247f8bce6e563a440f277037d812deb33a0f4a13945d898c296",
    "4fe342e2fe1a7f9b8ee7eb4a7c0f9e162bce33576b315ececbb6406837bf51f5",
};

static curveData nist384 = {
    "39402006196394479212279040100143613805079739270465446667948293404245721771496870329047266088258938001861606973112319",
    "39402006196394479212279040100143613805079739270465446667946905279627659399113263569398956308152294913554433653942643",
    "a335926aa319a27a1d00896a6773a4827acdac73",
    "79d1e655f868f02fff48dcdee14151ddb80643c1406d0ca10dfe6fc52009540a495e8042ea5f744f6e184667cc722483",
    "b3312fa7e23ee7e4988e056be3f82d19181d9c6efe8141120314088f5013875ac656398d8a2ed19d2a85c8edd3ec2aef",
    "aa87ca22be8b05378eb1c71ef320ad746e1d3b628ba79b9859f741e082542a385502f25dbf55296c3a545e3872760ab7",
    "3617de4a96262c6f5d9e98bf9292dc29f8f41dbd289a147ce9da3113b5f0b8c00a60b1ce1d7e819d7a431d7c90ea0e5f",
};

static curveData nist521 = {
    "6864797660130609714981900799081393217269435300143305409394463459185543183397656052122559640661454554977296311391480858037121987999716643812574028291115057151",
    "6864797660130609714981900799081393217269435300143305409394463459185543183397655394245057746333217197532963996371363321113864768612440380340372808892707005449",
    "d09e8800291cb85396cc6717393284aaa0da64ba",
        "0b48bfa5f420a34949539d2bdfc264eeeeb077688e44fbf0ad8f6d0edb37bd6b533281000518e19f1b9ffbe0fe9ed8a3c2200b8f875e523868c70c1e5bf55bad637",
        "051953eb9618e1c9a1f929a21a0b68540eea2da725b99b315f3b8b489918ef109e156193951ec7e937b1652c0bd3bb1bf073573df883d2c34f1ef451fd46b503f00",
         "c6858e06b70404e9cd9e3ecb662395b4429c648139053fb521f828af606b4d3dbaa14b5e77efe75928fe1dc127a2ffa8de3348b3c1856a429bf97e7e31c2e5bd66",
        "11839296a789a3bc0045c8a5fb42c7d1bd998f54449579b446817afbd17273e662c97ee72995ef42640c550b9013fad0761353c7086a272c24088be94769fd16650",
};


/*
 * The data for curve3617 copied from:
 * http://safecurves.cr.yp.to/field.html
 * http://safecurves.cr.yp.to/base.html
 */
static curveData curve3617 = {
    "3fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef",  /* Prime */
    "7ffffffffffffffffffffffffffffffffffffffffffffffffffeb3cc92414cf706022b36f1c0338ad63cf181b0e71a5e106af79",   /* order */
    "",                                                                                                          /* SEED */
    "",                                                                                                          /* c */
    "",                                                                                                          /* b */
    "1a334905141443300218c0631c326e5fcd46369f44c03ec7f57ff35498a4ab4d6d6ba111301a73faa8537c64c4fd3812f3cbc595",  /* Gx*/
    "22",                                                                                                        /* Gy (radix 16) */
};

/*
 * The data for curve25519 copied from:
 * http://safecurves.cr.yp.to/field.html
 * http://safecurves.cr.yp.to/base.html
 * 
 * Note: 
 * The data for Curve25519 is here for the sake of completeness and to have the same
 * set of initialization. One exception if the base point X coordinate (Gx) that we use to
 * compute the DH public value, refer to function ecdhGeneratePublic(...) in ecdh.c.
 * 
 * Otherwise the functions use EcCurve structure only to get the pointers to the Curve25519
 * wrapper functions.
 * 
 */
static curveData curve25519 = {
    "7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffed",   /* Prime */
    "1000000000000000000000000000000014def9dea2f79cd65812631a5cf5d3ed",   /* order */
    "",                                                                   /* SEED */
    "",                                                                   /* c */
    "",                                                                   /* b */
    "9",                                                                  /* Gx */
    "20ae19a1b8a086b4e01edd2c7748d14c923d4d7e6d7c61b229e9c5a27eced3d9",   /* Gy */
};

/*============================================================================*/
/*    Bignum Shorthand Functions                                              */
/*============================================================================*/

int bnAddMod_ (struct BigNum *rslt, struct BigNum *n1, struct BigNum *mod)
{
    bnAdd (rslt, n1);
    if (bnCmp (rslt, mod) >= 0) {
        bnSub (rslt, mod);
    }
    return 0;
}

int bnAddQMod_ (struct BigNum *rslt, unsigned n1, struct BigNum *mod)
{
    bnAddQ (rslt, n1);
    if (bnCmp (rslt, mod) >= 0) {
        bnSub (rslt, mod);
    }
    return 0;
}

int bnSubMod_ (struct BigNum *rslt, struct BigNum *n1, struct BigNum *mod)
{
    if (bnCmp (rslt, n1) < 0) {
        bnAdd (rslt, mod);
    }
    bnSub (rslt, n1);
    return 0;
}

int bnSubQMod_ (struct BigNum *rslt, unsigned n1, struct BigNum *mod)
{
    if (bnCmpQ (rslt, n1) < 0) {
        bnAdd (rslt, mod);
    }
    bnSubQ (rslt, n1);
    return 0;
}

int bnMulMod_ (struct BigNum *rslt, struct BigNum *n1, struct BigNum *n2, struct BigNum *mod, const EcCurve *curve)
{
    bnMul (rslt, n1, n2);
    if (curve)
        curve->modOp(rslt, rslt, mod);
    else
        bnMod(rslt, rslt, mod);
    return 0;
}

int bnMulQMod_ (struct BigNum *rslt, struct BigNum *n1, unsigned n2, struct BigNum *mod, const EcCurve *curve)
{
    bnMulQ (rslt, n1, n2);
    if (curve)
        curve->modOp(rslt, rslt, mod);
    else
        bnMod(rslt, rslt, mod);
   return 0;
}

int bnSquareMod_ (struct BigNum *rslt, struct BigNum *n1, struct BigNum *mod, const EcCurve *curve)
{
    bnSquare (rslt, n1);
    if (curve)
        curve->modOp(rslt, rslt, mod);
    else
        bnMod(rslt, rslt, mod);
    return 0;
}

/*
 * Note on the Curve25519 functions and usage of BigNumber:
 * In most cases the functions to compute Curve25519 data are small wrapper functions
 * that implement the same API as for the other curve functions. The wrapper functions
 * then call the very specific, high-efficient function in curve25519-donna.c .
 * 
 * For Curve25519 we don't have a real implementation for point add, point doubling, modulo
 * and check public key. Please refer to the actual implementations below.
 */

static int ecGetAffineNist(const EcCurve *curve, EcPoint *R, const EcPoint *P);
static int ecGetAffineEd(const EcCurve *curve, EcPoint *R, const EcPoint *P);
static int ecGetAffine25519(const EcCurve *curve, EcPoint *R, const EcPoint *P);

static int ecDoublePointNist(const EcCurve *curve, EcPoint *R, const EcPoint *P);
static int ecDoublePointEd(const EcCurve *curve, EcPoint *R, const EcPoint *P);
static int ecDoublePoint25519(const EcCurve *curve, EcPoint *R, const EcPoint *P);

static int ecAddPointNist(const EcCurve *curve, EcPoint *R, const EcPoint *P, const EcPoint *Q);
static int ecAddPointEd(const EcCurve *curve, EcPoint *R, const EcPoint *P, const EcPoint *Q);
static int ecAddPoint25519(const EcCurve *curve, EcPoint *R, const EcPoint *P, const EcPoint *Q);

static int ecCheckPubKeyNist(const EcCurve *curve, const EcPoint *pub);
static int ecCheckPubKey3617(const EcCurve *curve, const EcPoint *pub);
static int ecCheckPubKey25519(const EcCurve *curve, const EcPoint *pub);

static int ecGenerateRandomNumberNist(const EcCurve *curve, BigNum *d);
static int ecGenerateRandomNumber3617(const EcCurve *curve, BigNum *d);
static int ecGenerateRandomNumber25519(const EcCurve *curve, BigNum *d);

static int ecMulPointScalarNormal(const EcCurve *curve, EcPoint *R, const EcPoint *P, const BigNum *scalar);
static int ecMulPointScalar25519(const EcCurve *curve, EcPoint *R, const EcPoint *P, const BigNum *scalar);

/* Forward declaration of new modulo functions for the EC curves */
static int newMod192(BigNum *r, const BigNum *a, const BigNum *modulo);
static int newMod256(BigNum *r, const BigNum *a, const BigNum *modulo);
static int newMod384(BigNum *r, const BigNum *a, const BigNum *modulo);
static int newMod521(BigNum *r, const BigNum *a, const BigNum *modulo);

static int mod3617(BigNum *r, const BigNum *a, const BigNum *modulo);
static int mod25519(BigNum *r, const BigNum *a, const BigNum *modulo);

static void commonInit(void)
{
    bnBegin(mpiZero); bnSetQ(mpiZero, 0);
    bnBegin(mpiOne); bnSetQ(mpiOne, 1);
    bnBegin(mpiTwo); bnSetQ(mpiTwo, 2);
    bnBegin(mpiThree); bnSetQ(mpiThree, 3);
    bnBegin(mpiFour); bnSetQ(mpiFour, 4);
    bnBegin(mpiEight); bnSetQ(mpiEight, 8);
}

static void curveCommonInit(EcCurve *curve)
{
    /* Initialize scratchpad variables and their pointers */
    bnBegin(&curve->_S1); curve->S1 = &curve->_S1;
    bnBegin(&curve->_U1); curve->U1 = &curve->_U1;
    bnBegin(&curve->_H);  curve->H = &curve->_H;
    bnBegin(&curve->_R);  curve->R = &curve->_R;
    bnBegin(&curve->_t0); curve->t0 = &curve->_t0;
    bnBegin(&curve->_t1); curve->t1 = &curve->_t1;
    bnBegin(&curve->_t2); curve->t2 = &curve->_t2;
    bnBegin(&curve->_t3); curve->t3 = &curve->_t3;
}

static void curveCommonPrealloc(EcCurve *curve)
{
    size_t maxBits;

    /* variables must be able to hold p^2, plus one nimb (min. 15 bits) for overflow */
    maxBits = bnBits(curve->p) * 2 + 15;

    /* The set_bit allocates enough memory to hold maximum values */
    /* Initialize scratchpad variables before use */
    bnPrealloc(curve->S1, maxBits);
    bnPrealloc(curve->U1, maxBits);
    bnPrealloc(curve->H, maxBits);
    bnPrealloc(curve->R, maxBits);
    bnPrealloc(curve->S1, maxBits);
    bnPrealloc(curve->t1, maxBits);
    bnPrealloc(curve->t2, maxBits);
    bnPrealloc(curve->t3, maxBits);
}

int ecGetCurveNistECp(Curves curveId, EcCurve *curve)
{
    curveData *cd;

    if (curveId >= Curve25519 && curveId <= Curve3617)
        return ecGetCurvesCurve(curveId, curve);

    if (!initialized) {
        commonInit();
        initialized = 1;
    }
    if (curve == NULL)
        return -2;

    bnBegin(&curve->_p);    curve->p = &curve->_p;
    bnBegin(&curve->_n);    curve->n = &curve->_n;
    bnBegin(&curve->_SEED); curve->SEED = &curve->_SEED;
    bnBegin(&curve->_c);    curve->c = &curve->_c;
    bnBegin(&curve->_a);    curve->a = &curve->_a;
    bnBegin(&curve->_b);    curve->b = &curve->_b;
    bnBegin(&curve->_Gx);   curve->Gx = &curve->_Gx;
    bnBegin(&curve->_Gy);   curve->Gy = &curve->_Gy;

    curveCommonInit(curve);

    switch (curveId) {
    case NIST192P:
        cd = &nist192;
        curve->modOp = newMod192;
        break;

    case NIST224P:
        cd = &nist224;
        curve->modOp = bnMod;
        break;

    case NIST256P:
        cd = &nist256;
        curve->modOp = bnMod;
        break;

    case NIST384P:
        cd = &nist384;
        curve->modOp = newMod384;
        break;

    case NIST521P:
        cd = &nist521;
        curve->modOp = newMod521;
        break;

    default:
        return -2;
    }

    curve->affineOp = ecGetAffineNist;
    curve->doubleOp = ecDoublePointNist;
    curve->addOp = ecAddPointNist;
    curve->checkPubOp = ecCheckPubKeyNist;
    curve->randomOp = ecGenerateRandomNumberNist;
    curve->mulScalar = ecMulPointScalarNormal;

    bnReadAscii(curve->p, cd->p, 10);
    bnReadAscii(curve->n, cd->n, 10);
    bnReadAscii(curve->SEED, cd->SEED, 16);
    bnReadAscii(curve->c, cd->c, 16);
    bnCopy(curve->a, curve->p);
    bnSub(curve->a, mpiThree);
    bnReadAscii(curve->b, cd->b, 16);
    bnReadAscii(curve->Gx, cd->Gx, 16);
    bnReadAscii(curve->Gy, cd->Gy, 16);

    curveCommonPrealloc(curve);
    curve->id = curveId;

    return 0;
}

int ecGetCurvesCurve(Curves curveId, EcCurve *curve)
{
    curveData *cd;

    if (!initialized) {
        commonInit();
        initialized = 1;
    }
    if (curve == NULL)
        return -2;

    /* set-up all bignum structures, simplifies "free" handling */
    bnBegin(&curve->_p);    curve->p = &curve->_p;
    bnBegin(&curve->_n);    curve->n = &curve->_n;
    bnBegin(&curve->_SEED); curve->SEED = &curve->_SEED;
    bnBegin(&curve->_c);    curve->c = &curve->_c;
    bnBegin(&curve->_a);    curve->a = &curve->_a;
    bnBegin(&curve->_b);    curve->b = &curve->_b;
    bnBegin(&curve->_Gx);   curve->Gx = &curve->_Gx;
    bnBegin(&curve->_Gy);   curve->Gy = &curve->_Gy;

    curveCommonInit(curve);

    switch (curveId) {
    case Curve3617:
        cd = &curve3617;
        curve->modOp = mod3617;
        curve->affineOp = ecGetAffineEd;
        curve->doubleOp = ecDoublePointEd;
        curve->addOp = ecAddPointEd;
        curve->checkPubOp = ecCheckPubKey3617;
        curve->randomOp = ecGenerateRandomNumber3617;
        curve->mulScalar = ecMulPointScalarNormal;

        bnReadAscii(curve->a, "3617", 10);
        break;

    case Curve25519:
        cd = &curve25519;
        curve->modOp = mod25519;
        curve->affineOp = ecGetAffine25519;
        curve->doubleOp = ecDoublePoint25519;
        curve->addOp = ecAddPoint25519;
        curve->checkPubOp = ecCheckPubKey25519;
        curve->randomOp = ecGenerateRandomNumber25519;
        curve->mulScalar = ecMulPointScalar25519;

        bnReadAscii(curve->a, "486662", 10);
        break;

    default:
        return -2;
    }
    bnReadAscii(curve->p, cd->p, 16);
    bnReadAscii(curve->n, cd->n, 16);

    bnReadAscii(curve->Gx, cd->Gx, 16);
    bnReadAscii(curve->Gy, cd->Gy, 16);

    curveCommonPrealloc(curve);
    curve->id = curveId;
    return 0;
}

void ecFreeCurveNistECp(EcCurve *curve) 
{
    if (curve == NULL)
        return;

    bnEnd(curve->p);
    bnEnd(curve->n);
    bnEnd(curve->SEED);
    bnEnd(curve->c);
    bnEnd(curve->a);
	bnEnd(curve->b);
    bnEnd(curve->Gx);
    bnEnd(curve->Gy);

    bnEnd(curve->S1);
    bnEnd(curve->U1);
    bnEnd(curve->H);
    bnEnd(curve->R);
    bnEnd(curve->t0);
    bnEnd(curve->t1);
    bnEnd(curve->t2);
    bnEnd(curve->t3);
}

/*
 * EC point helper functions
 */

void ecInitPoint(EcPoint *P)
{
    INIT_EC_POINT(P);
}

void ecFreePoint(EcPoint *P)
{
    FREE_EC_POINT(P);
}

void ecSetBasePoint(EcCurve *C, EcPoint *P)
{
    SET_EC_BASE_POINT(C, P);
}

void ecFreeCurvesCurve(EcCurve *curve)
{
    ecFreeCurveNistECp(curve);
}

/*============================================================================*/
/*    Elliptic Curve arithmetic                                               */
/*============================================================================*/

int ecGetAffine(const EcCurve *curve, EcPoint *R, const EcPoint *P)
{
    return curve->affineOp(curve, R, P);
}

static int ecGetAffineNist(const EcCurve *curve, EcPoint *R, const EcPoint *P)
{
    int ret = 0;

    struct BigNum z_1, z_2;

    bnBegin(&z_1);
    bnBegin(&z_2);

    /* affine x = X / Z^2 */
    bnInv (&z_1, P->z, curve->p);                 /* z_1 = Z^(-1) */
    bnMulMod_(&z_2, &z_1, &z_1, curve->p, curve); /* z_2 = Z^(-2) */
    bnMulMod_(R->x, P->x, &z_2, curve->p, curve);

    /* affine y = Y / Z^3 */
    bnMulMod_(&z_2, &z_2, &z_1, curve->p, curve); /* z_2 = Z^(-3) */
    bnMulMod_(R->y, P->y, &z_2, curve->p, curve);

    bnSetQ(R->z, 1);

    bnEnd(&z_1);
    bnEnd(&z_2);
    return ret;
}

static int ecGetAffineEd(const EcCurve *curve, EcPoint *R, const EcPoint *P)
{
    int ret = 0;

    struct BigNum z_1;

    bnBegin(&z_1);

    /* affine x = X / Z */
    bnInv (&z_1, P->z, curve->p);                 /* z_1 = Z^(-1) */
    bnMulMod_(R->x, P->x, &z_1, curve->p, curve);

    /* affine y = Y / Z */
    bnMulMod_(R->y, P->y, &z_1, curve->p, curve);

    bnSetQ(R->z, 1);

    bnEnd(&z_1);
    return ret;

}

/* 
 * If the arguments do not point to the same EcPoint then copy P to result.
 * Curve25519 has no specific GetAffine function, it's all inside curve25519-donna
 */
static int ecGetAffine25519(const EcCurve *curve, EcPoint *R, const EcPoint *P)
{
    if (R != P) {
        bnCopy(R->x, P->x);
        bnCopy(R->y, P->y);
        bnCopy(R->z, P->z);
    }
    return 0;
}

int ecDoublePoint(const EcCurve *curve, EcPoint *R, const EcPoint *P)
{
    return curve->doubleOp(curve, R, P);
}

static int ecDoublePointNist(const EcCurve *curve, EcPoint *R, const EcPoint *P)
{
    int ret = 0;

    EcPoint tP;
    const EcPoint *ptP = 0;

    if (!bnCmp(P->y, mpiZero) || !bnCmp(P->z, mpiZero)) {
        bnSetQ(R->x, 1);
        bnSetQ(R->y, 1);
        bnSetQ(R->z, 0);
        return 0;
    }

    /* Check for overlapping arguments, copy if necessary and set pointer */
    if (P == R) {
        INIT_EC_POINT(&tP);
        ptP = &tP;
        bnCopy(tP.x, P->x);
        bnCopy(tP.y, P->y);
        bnCopy(tP.z, P->z);
    }
    else 
        ptP = P;

    /* S = 4*X*Y^2, save Y^2 in t1 for later use */
    bnMulMod_(curve->t1, ptP->y, ptP->y, curve->p, curve);       /* t1 = Y^2 */
    bnMulMod_(curve->t0, ptP->x, mpiFour, curve->p, curve);      /* t0 = 4 * X */
    bnMulMod_(curve->S1, curve->t0, curve->t1, curve->p, curve); /* S1 = t0 * t1 */

    /* M = 3*(X + Z^2)*(X - Z^2), use scratch variable U1 to store M value */
    bnMulMod_(curve->t2, ptP->z, ptP->z, curve->p, curve);       /* t2 = Z^2 */
    bnCopy(curve->t0, ptP->x);
    bnAddMod_(curve->t0, curve->t2, curve->p);                   /* t0 = X + t2  */
    bnMulMod_(curve->t3, curve->t0, mpiThree, curve->p, curve);  /* t3 = 3 * t0 */
    bnCopy(curve->t0, ptP->x);
    bnSubMod_(curve->t0, curve->t2, curve->p);                   /* t0 = X - t2 */
    bnMulMod_(curve->U1, curve->t3, curve->t0, curve->p, curve); /* M = t3 * t0 */
    
    /* X' = M^2 - 2*S */
    bnMulMod_(curve->t2, curve->U1, curve->U1, curve->p, curve); /* t2 = M^2 */
    bnMulMod_(curve->t0, curve->S1, mpiTwo, curve->p, curve);    /* t0 = S * 2 */
    bnCopy(R->x, curve->t2);
    bnSubMod_(R->x, curve->t0, curve->p);                        /* X' = t2 - t0 */

    /* Y' = M*(S - X') - 8*Y^4 */
    bnMulMod_(curve->t3, curve->t1, curve->t1, curve->p, curve); /* t3 = Y^4 (t1 saved above) */
    bnMulMod_(curve->t2, curve->t3, mpiEight, curve->p, curve);  /* t2 = t3 * 8 */
    bnCopy(curve->t3, curve->S1);
    bnSubMod_(curve->t3, R->x, curve->p);                        /* t3 = S - X' */
    bnMulMod_(curve->t0, curve->U1, curve->t3, curve->p, curve); /* t0 = M * t3 */
    bnCopy(R->y, curve->t0);
    bnSubMod_(R->y, curve->t2, curve->p);                        /* Y' = t0 - t2 */

    /* Z' = 2*Y*Z */
    bnMulMod_(curve->t0, ptP->y, mpiTwo, curve->p, curve);       /* t0 = 2 * Y */
    bnMulMod_(R->z, curve->t0, ptP->z, curve->p, curve);         /* Z' = to * Z */

    if (P == R)
        FREE_EC_POINT(&tP);

    return ret;
}

static int ecDoublePointEd(const EcCurve *curve, EcPoint *R, const EcPoint *P)
{
    EcPoint tP;
    const EcPoint *ptP = 0;

    /* Check for overlapping arguments, copy if necessary and set pointer */
    if (P == R) {
        INIT_EC_POINT(&tP);
        ptP = &tP;
        bnCopy(tP.x, P->x);
        bnCopy(tP.y, P->y);
        bnCopy(tP.z, P->z);
    }
    else 
        ptP = P;

    /* Compute B, C, D, H, E */
    bnCopy(curve->t1, ptP->x);
    bnAddMod_(curve->t1, ptP->y, curve->p);
    bnSquareMod_(curve->t0, curve->t1, curve->p, curve);       /* t0 -> B */

    bnSquareMod_(R->x, ptP->x, curve->p, curve);               /* Rx -> C */

    bnSquareMod_(R->y, ptP->y, curve->p, curve);               /* Ry -> D */

    bnSquareMod_(R->z, ptP->z, curve->p, curve);               /* Rz -> H */
    bnAddMod_(R->z, R->z, curve->p);                           /* Rz -> 2H */

    bnCopy(curve->t1, R->x);
    bnAddMod_(curve->t1, R->y, curve->p);                      /* t1 -> E */

    /* Compute Ry */
    bnCopy(curve->t2, R->x);
    bnSubMod_(curve->t2, R->y, curve->p);                      /* C - D */
    bnMulMod_(R->y, curve->t1, curve->t2, curve->p, curve);    /* E * t2; Ry */

    /* Compute Rx */
    bnSubMod_(curve->t0, curve->t1, curve->p);                 /* B - E; sub result */
    bnCopy(curve->t2, curve->t1);
    bnSubMod_(curve->t2, R->z, curve->p);                      /* t2 -> J; (E - 2H) */
    bnMulMod_(R->x, curve->t2, curve->t0, curve->p, curve);    /* J * t0 */

    /* Compute Rz */
    bnMulMod_(R->z, curve->t2, curve->t1, curve->p, curve);    /* J * E */

    if (P == R)
        FREE_EC_POINT(&tP);

    return 0;
}

/* 
 * Curve25519 has no specific Double Point function, all inside curve25519-donna
 */
static int ecDoublePoint25519(const EcCurve *curve, EcPoint *R, const EcPoint *P)
{
    return -2;
}

/* Add two elliptic curve points. Any of them may be the same object. */
int ecAddPoint(const EcCurve *curve, EcPoint *R, const EcPoint *P, const EcPoint *Q)
{
    return curve->addOp(curve, R, P, Q);
}

static int ecAddPointNist(const EcCurve *curve, EcPoint *R, const EcPoint *P, const EcPoint *Q)
{
    int ret = 0;

    EcPoint tP, tQ;
    const EcPoint *ptP = 0;
    const EcPoint *ptQ = 0;


    /* Fast check if application called add(R, P, P) */
    if (!bnCmp(P->x, Q->x) && !bnCmp(P->y, Q->y) && !bnCmp(P->z, Q->z)) {
        return ecDoublePoint(curve, R, P);
    }

    /* if P is (@,@), R = Q */
    if (!bnCmp(P->z, mpiZero)) {
        bnCopy(R->x, Q->x);
        bnCopy(R->y, Q->y);
        bnCopy(R->z, Q->z);
        return 0;
    }

    /* if Q is (@,@), R = P */
    if (!bnCmp(Q->z, mpiZero)) {
        bnCopy(R->x, P->x);
        bnCopy(R->y, P->y);
        bnCopy(R->z, P->z);
        return 0;
    }

    /* Check for overlapping arguments, copy if necessary and set pointers */
    if (P == R) {
        INIT_EC_POINT(&tP);
        ptP = &tP;
        bnCopy(tP.x, P->x);
        bnCopy(tP.y, P->y);
        bnCopy(tP.z, P->z);
    }
    else 
        ptP = P;

    if (Q == R) {
        INIT_EC_POINT(&tQ);
        ptQ = &tQ;
        bnCopy(tQ.x, Q->x);
        bnCopy(tQ.y, Q->y);
        bnCopy(tQ.z, Q->z);
    }
    else
        ptQ = Q;

    /* U1 = X1*Z2^2, where X1: P->x, Z2: Q->z */
    bnMulMod_(curve->t1, ptQ->z, ptQ->z, curve->p, curve);    /* t1 = Z2^2 */
    bnMulMod_(curve->U1, ptP->x, curve->t1, curve->p, curve); /* U1 = X1 * z_2 */

    /* S1 = Y1*Z2^3, where Y1: P->y */
    bnMulMod_(curve->t1, curve->t1, ptQ->z, curve->p, curve); /* t1 = Z2^3 */
    bnMulMod_(curve->S1, ptP->y, curve->t1, curve->p, curve); /* S1 = Y1 * z_2 */

    /* U2 = X2*Z1^2, where X2: Q->x, Z1: P->z */
    bnMulMod_(curve->t1, ptP->z, ptP->z, curve->p, curve);    /* t1 = Z1^2 */
    bnMulMod_(curve->H, ptQ->x, curve->t1, curve->p, curve);  /* H = X2 * t1 (store U2 in H) */

    /* H = U2 - U1 */
    bnSubMod_(curve->H, curve->U1, curve->p);

    /* S2 = Y2*Z1^3, where Y2: Q->y */
    bnMulMod_(curve->t1, curve->t1, ptP->z, curve->p, curve); /* t1 = Z1^3 */
    bnMulMod_(curve->R, ptQ->y, curve->t1, curve->p, curve);  /* R = Y2 * t1 (store S2 in R) */

    /* R = S2 - S1 */
    bnSubMod_(curve->R, curve->S1, curve->p);

    /* if (U1 == U2), i.e H is zero */
    if (!bnCmp(curve->H, mpiZero)) {

        /* if (S1 != S2), i.e. R is _not_ zero: return infinity*/
        if (bnCmp(curve->R, mpiZero)) {
            bnSetQ(R->x, 1);
            bnSetQ(R->y, 1);
            bnSetQ(R->z, 0);
            return 0;
        }
        return ecDoublePoint(curve, R, P);
    }
    /* X3 = R^2 - H^3 - 2*U1*H^2, where X3: R->x */
    bnMulMod_(curve->t0, curve->H, curve->H, curve->p, curve);   /* t0 = H^2 */
    bnMulMod_(curve->t1, curve->U1, curve->t0, curve->p, curve); /* t1 = U1 * t0, (hold t1) */
    bnMulMod_(curve->t0, curve->t0, curve->H, curve->p, curve);  /* t0 = H^3, (hold t0) */
    bnMulMod_(curve->t2, curve->R, curve->R, curve->p, curve);   /* t2 = R^2 */
    bnCopy(curve->t3, curve->t2);
    bnSubMod_(curve->t3, curve->t0, curve->p);                   /* t3 = t2 - t0, (-H^3)*/
    bnMulMod_(curve->t2, mpiTwo, curve->t1, curve->p, curve);    /* t2 = 2 * t1 */
    bnCopy(R->x, curve->t3);
    bnSubMod_(R->x, curve->t2, curve->p);                        /* X3 = t3 - t2 */

    /* Y3 = R*(U1*H^2 - X3) - S1*H^3, where Y3: R->y */
    bnSubMod_(curve->t1, R->x, curve->p);                        /* t1 = t1 - X3, overwrites t1 now */
    bnMulMod_(curve->t2, curve->R, curve->t1, curve->p, curve);  /* t2 = R * z_2 */
    bnMulMod_(curve->S1, curve->S1, curve->t0, curve->p, curve); /* S1 = S1 * t0, (t0 has H^3) */
    bnCopy(R->y, curve->t2);
    bnSubMod_(R->y, curve->S1, curve->p);                        /* Y3 = t2 - S1 */

    /* Z3 = H*Z1*Z2, where Z1: P->z, Z2: Q->z, Z3: R->z */
    bnMulMod_(curve->t2, curve->H, P->z, curve->p, curve);       /* t2 = H * Z1 */
    bnMulMod_(R->z, curve->t2, Q->z, curve->p, curve);           /* Z3 = t2 * Z2 */

    if (P == R)
        FREE_EC_POINT(&tP);
    if (Q == R)
        FREE_EC_POINT(&tQ);
    return ret;
}

/*
 * Refer to the document: Faster addition and doubling on elliptic curves; Daniel J. Bernstein and Tanja Lange
 * section 4.
 *
 * This function is a variant of the 'addition'. The function returns the result in an own curve point
 * and does not overwrite its input parameters.
 */
static int ecAddPointEd(const EcCurve *curve, EcPoint *R, const EcPoint *P, const EcPoint *Q)
{
    EcPoint tP, tQ;
    const EcPoint *ptP = 0;
    const EcPoint *ptQ = 0;

    /* if P is (@,@), R = Q */
    if (!bnCmp(P->z, mpiZero)) {
        bnCopy(R->x, Q->x);
        bnCopy(R->y, Q->y);
        bnCopy(R->z, Q->z);
        return 0;
    }

    /* if Q is (@,@), R = P */
    if (!bnCmp(Q->z, mpiZero)) {
        bnCopy(R->x, P->x);
        bnCopy(R->y, P->y);
        bnCopy(R->z, P->z);
        return 0;
    }

    /* Check for overlapping arguments, copy if necessary and set pointers */
    if (P == R) {
        INIT_EC_POINT(&tP);
        ptP = &tP;
        bnCopy(tP.x, P->x);
        bnCopy(tP.y, P->y);
        bnCopy(tP.z, P->z);
    }
    else 
        ptP = P;

    if (Q == R) {
        INIT_EC_POINT(&tQ);
        ptQ = &tQ;
        bnCopy(tQ.x, Q->x);
        bnCopy(tQ.y, Q->y);
        bnCopy(tQ.z, Q->z);
    }
    else
        ptQ = Q;

    /* Compute A, C, D first */
    bnMulMod_(R->z, ptP->z, ptQ->z, curve->p, curve);            /* Rz -> A; (Z1 * Z2); Rz becomes R3 */
    bnMulMod_(R->x, ptP->x, ptQ->x, curve->p, curve);            /* Rx -> C; (X1 * X2); Rx becomes R1 */
    bnMulMod_(R->y, ptP->y, ptQ->y, curve->p, curve);            /* Ry -> D; (Y1 * Y2); Ry becomes R2 */

    /* Compute large parts of X3 equation, sub result in t0 */
    bnCopy(curve->t0, ptP->x);
    bnAddMod_(curve->t0, ptP->y, curve->p);                      /* t0 -> X1 + Y1 */
    bnCopy(curve->t1, ptQ->x);
    bnAddMod_(curve->t1, ptQ->y, curve->p);                      /* t1 -> X2 + Y2 */
    bnMulMod_(curve->t2, curve->t0, curve->t1, curve->p, curve); /* t2 = t0 * t1 */
    bnSubMod_(curve->t2, R->x, curve->p);                        /* t2 - C */
    bnSubMod_(curve->t2, R->y, curve->p);                        /* t2 - D */
    bnMulMod_(curve->t0, curve->t2, R->z, curve->p, curve);      /* t0 -> R7; (t2 * A); sub result */

    /* Compute E */
    bnMulMod_(curve->t2, R->x, R->y, curve->p, curve);           /* t2 = C * D */
    bnMulMod_(curve->t1, curve->t2, curve->a, curve->p, curve);  /* t1 -> E; t1 new R8 */

    /* Compute part of Y3 equation, sub result in t2 */
    bnSubMod_(R->y, R->x, curve->p);                             /* Ry = D - C; sub result */
    bnMulMod_(curve->t2, R->y, R->z, curve->p, curve);           /* t2 = Ry * A; sub result */

    /* Compute B */
    bnSquareMod_(R->z, R->z, curve->p, curve);                   /* Rz -> B; (A^2) */

    /* Compute F */
    bnCopy(curve->t3, R->z);
    bnSubMod_(curve->t3, curve->t1, curve->p);                   /* t3 -> F; (B - E) */

    /* Compute G */
    bnAddMod_(R->z, curve->t1, curve->p);                        /* Rz -> G; (B + E) */

    /* Compute, X, Y, Z results */
    bnMulMod_(R->x, curve->t3, curve->t0, curve->p, curve);      /* Rx = F * t0 */
    bnMulMod_(R->y, curve->t2, R->z, curve->p, curve);           /* Ry = t2 * G */
    bnMulMod_(R->z, curve->t3, R->z, curve->p, curve);           /* Rz = F * G */

    if (P == R)
        FREE_EC_POINT(&tP);
    if (Q == R)
        FREE_EC_POINT(&tQ);

    return 0;
}

/* 
 * Curve25519 has no specific Add Point function, all inside curve25519-donna
 */
static int ecAddPoint25519(const EcCurve *curve, EcPoint *R, const EcPoint *P, const EcPoint *Q)
{
    return -2;
}

int ecMulPointScalar(const EcCurve *curve, EcPoint *R, const EcPoint *P, const BigNum *scalar)
{
    return curve->mulScalar(curve, R, P, scalar);
}

static int ecMulPointScalarNormal(const EcCurve *curve, EcPoint *R, const EcPoint *P, const BigNum *scalar)
{
    int ret = 0;
    int i;
    int bits = bnBits(scalar);
    EcPoint n;

    INIT_EC_POINT(&n);
    bnCopy(n.x, P->x);
    bnCopy(n.y, P->y);
    bnCopy(n.z, P->z);

    bnSetQ(R->x, 0);
    bnSetQ(R->y, 0);
    bnSetQ(R->z, 0);

    for (i = 0; i < bits; i++) {
        if (bnReadBit(scalar, i))
            ecAddPoint(curve, R, R, &n);

        /*        ecAddPoint(curve, &n, &n, &n); */
        ecDoublePoint(curve, &n, &n);
    }
    FREE_EC_POINT(&n);
    return ret;
}

/* 
 * This function uses BigNumber only as containers to transport the 32 byte data.
 * This makes it compliant to the other functions and thus higher-level API does not change.
 * 
 * curve25519_donna function uses data in little endian format.
 */
static int ecMulPointScalar25519(const EcCurve *curve, EcPoint *R, const EcPoint *P, const BigNum *scalar)
{
    uint8_t basepoint[32], secret[32], result[32];

    bnExtractLittleBytes(P->x, basepoint, 0, 32);  /* 25519 function requires the X coordinate only (compressed) */
    bnExtractLittleBytes(scalar, secret, 0, 32);
    curve25519_donna(result, secret, basepoint);
    bnInsertLittleBytes(R->x, result, 0, 32);
    return 0;
}

#ifdef WEAKRANDOM
#include <fcntl.h>

/*
 * A standard random number generator that uses the portable random() system function.
 *
 * This should be enhanced to use a better random generator
 */
static int _random(unsigned char *output, size_t len)
{
    size_t num = 0;

    int rnd = open("/dev/urandom", O_RDONLY);
    if (rnd >= 0) {
        num = read(rnd, output, len);
        close(rnd);
    }
    else
        return num;

    return( 0 );
}
#else
#include <cryptcommon/ZrtpRandom.h>
static int _random(unsigned char *output, size_t len)
{
    return zrtp_getRandomData(output, len);
}
#endif

int ecGenerateRandomNumber(const EcCurve *curve, BigNum *d)
{
    return curve->randomOp(curve, d);
}

#define MAX_RANDOM_BYTES  128   // Curve 521 (our biggest curve) requires 66 (+8) bytes of random data

static int ecGenerateRandomNumberNist(const EcCurve *curve, BigNum *d)
{
    BigNum c, nMinusOne;
    uint8_t ran[MAX_RANDOM_BYTES];
    size_t randomBytes = ((bnBits(curve->n) + 64) + 7) / 8;

    if (randomBytes > MAX_RANDOM_BYTES)
        return -1;

    bnBegin(&c);
    bnBegin(&nMinusOne);

    bnCopy(&nMinusOne, curve->n);
    bnSubMod_(&nMinusOne, mpiOne, curve->p);

    bnSetQ(d, 0);

    while (!bnCmpQ(d, 0)) {
        /* use _random function */
        _random(ran, randomBytes);
        bnInsertBigBytes(&c, ran, 0, randomBytes);
        bnMod(d, &c, &nMinusOne);
        bnAddMod_(d, mpiOne, curve->p);
    }

    bnEnd(&c);
    bnEnd(&nMinusOne);

    return 0;
}

static int ecGenerateRandomNumber3617(const EcCurve *curve, BigNum *d)
{
    unsigned char random[52];
    _random(random, 52);

    /* prepare the secret random data: clear bottom 3 bits. Clearing top 2 bits
     * makes is a 414 bit value
     */
    random[51] &= ~0x7;
    random[0] &= 0x3f;
    /* convert the random data into big numbers */
    bnInsertBigBytes(d, random, 0, 52);
    return 0;
}

static int ecGenerateRandomNumber25519(const EcCurve *curve, BigNum *d)
{
    unsigned char random[32];
    _random(random, 32);

    // Same as in curve25519_donna, thus a no-op there if this function generates the secret.
    random[0] &= 248;
    random[31] &= 127;
    random[31] |= 64;

    /* No specific preparation. The curve25519_donna functions prepares the data.
     *
     * convert the random data into big numbers. the bigNumber is a container only.
     * we don not use the big number for any arithmetic
     */
    bnInsertLittleBytes(d, random, 0, 32);
    return 0;

}

int ecCheckPubKey(const EcCurve *curve, const EcPoint *pub)
{
    return curve->checkPubOp(curve, pub);
}

static int ecCheckPubKeyNist(const NistECpCurve *curve, const EcPoint *pub)
{
    /* Represent point at infinity by (0, 0), make sure it's not that */
    if (bnCmpQ(pub->x, 0) == 0 && bnCmpQ(pub->y, 0) == 0) {
        return 0;
    }
    /* Check that coordinates are within range */
    if (bnCmpQ(pub->x, 0) < 0 || bnCmp(pub->x, curve->p) >= 0) {
        return 0;
    }
    if (bnCmpQ(pub->y, 0) < 0 || bnCmp(pub->y, curve->p) >= 0) {
        return 0;
    }
    /* Check that point satisfies EC equation y^2 = x^3 - 3x + b, mod P */
    bnSquareMod_(curve->t1, pub->y, curve->p, curve);
    bnSquareMod_(curve->t2, pub->x, curve->p, curve);
    bnSubQMod_(curve->t2, 3, curve->p);
    bnMulMod_(curve->t2, curve->t2, pub->x, curve->p, curve);
    bnAddMod_(curve->t2, curve->b, curve->p);
    if (bnCmp (curve->t1, curve->t2) != 0) {
        return 0;
    }
    return 1;

}

static int ecCheckPubKey3617(const EcCurve *curve, const EcPoint *pub)
{
    /* Represent point at infinity by (0, 0), make sure it's not that */
    if (bnCmpQ(pub->x, 0) == 0 && bnCmpQ(pub->y, 0) == 0) {
        return 0;
    }
    /* Check that coordinates are within range */
    if (bnCmpQ(pub->x, 0) < 0 || bnCmp(pub->x, curve->p) >= 0) {
        return 0;
    }
    if (bnCmpQ(pub->y, 0) < 0 || bnCmp(pub->y, curve->p) >= 0) {
        return 0;
    }
    /* Check that point satisfies EC equation x^2+y^2 = 1+3617x^2y^2, mod P */
    bnSquareMod_(curve->t1, pub->y, curve->p, curve);
    bnSquareMod_(curve->t2, pub->x, curve->p, curve);
    bnCopy(curve->t3, curve->t1);                                /* Load t3 */
    bnAddMod_(curve->t3, curve->t2, curve->p);                   /* t3 = t1 + t2, (x^2+y^2)*/

    bnMulMod_(curve->t0, curve->a, curve->t1, curve->p, curve);  /* t0 = a * t1,  (3617 * y^2) */
    bnMulMod_(curve->t0, curve->t0, curve->t2, curve->p, curve); /* t0 = t0 * t2, (3617 * x^2 * y^2) */
    bnAddMod_(curve->t0, mpiOne, curve->p);                      /* t0 = t0 + 1,  (3617 * x^2 * y^2 + 1) */

    if (bnCmp (curve->t0, curve->t3) != 0) {
        return 0;
    }
    return 1;
}

/**
 * According to http://cr.yp.to/ecdh.html#validate no validation is required if used for Diffie-Hellman
 * thus always return success.
 */
static int ecCheckPubKey25519(const EcCurve *curve, const EcPoint *pub)
{
    return 1;
}

static int mod3617(BigNum *r, const BigNum *a, const BigNum *modulo)
{
    unsigned char buffer[52] = {0};
    int cmp;
    BigNum tmp;

    bnBegin(&tmp);
    cmp = bnCmp(modulo, a);
    if (cmp == 0) {             /* a is equal modulo, set resul to zero */
        bnSetQ(r, 0);
        return 0;
    }
    if (cmp > 0) {              /* modulo is greater than a - copy a to r and return it */
        bnCopy(r, a);
        return 0;
    }
    bnExtractLittleBytes(a, buffer, 0, 52);
    buffer[51] &= 0x3f;

    bnCopy(&tmp, a);
    bnRShift(&tmp, 414);
    bnCopy(r, &tmp);
    bnLShift(r, 4);
    bnAdd(r, &tmp);

    bnInsertLittleBytes(&tmp, buffer, 0, 52);

    bnAdd(r, &tmp);
    while (bnCmp(r, modulo) >= 0) {
        bnSub(r, modulo);
    }
    bnEnd(&tmp);
    return 0;
}

/* 
 * Curve25519 has no specific modulo function, all inside curve25519-donna
 */
static int mod25519(BigNum *r, const BigNum *a, const BigNum *modulo)
{
    return -2;
}

/*
 * Beware: Here are the dragons.
 *
 * The modulo implementations for the NIST curves. For more detailled information see
 * FIPS 186-3, chapter D.2 and other papers about Generailzed Mersenne numbers.
 *
 * I use byte operations to perfom the additions with carry. On a little endian machine
 * this saves conversion from/to big endian format if I would use integers for example. Also
 * using byte addition into a short carry accumulator works on every word size and avoids
 * complex testing and handling of wordsizes and big/little endian stuff.
 *
 */

/* new modulo for 192bit curve */
static int newMod192(BigNum *r, const BigNum *a, const BigNum *modulo)
{
    unsigned char buffer[200] = {0};
    unsigned char *pt;
    unsigned char *ps1;
    unsigned char *ps2;
    unsigned char *ps3;
    short ac;
    int cmp;

    /* Binary big number representation in PolarSSL is always big endian
     *
     * the least significant 64bit large word starts at byte offset 40,
     * the least significant 32bit word starts at byte offset 44
     * the least significant byte starts at byte offset 47
     *
     *           S3    S2   S1          T
     *                            /-----^------\
     *           A5    A4   A3    A2    A1    A0
     * 64bit  0     1     2     3     4     5
     *        |--+--|--+--|--+--|--+--|--+--|--+--|
     * 32bit  0  1  2  3  4  5  6  7  8  9 10 11
     *
     * perform T + S1 + S2 + S3 mod p

     * where T  = (A2 || A1 || A0)
     *     + S1 = ( 0 || A3 || A3)
     *     + S2 = (A4 || A4 ||  0)
     *     + S3 = (A5 || A5 || A5)
     *
     * TODO: error check if input variable is > modulo^2 (do normal mpi_mod_mpi),
     */

    /* TODO: check if a is > modulo^2 */
    cmp = bnCmp(modulo, a);
    if (cmp == 0) {             /* a is equal modulo, set resul to zero */
        bnSetQ(r, 0);
        return 0;
    }
    if (cmp > 0) {              /* modulo is greater than a - copy a to r and return it */
        bnCopy(r, a);
        return 0;
    }
    bnExtractBigBytes(a, buffer, 0, bnBytes(modulo)*2);

    /* 6 'A' words, each word is 8 byte. Compute offset to least significant byte of word X */
#define A(X) buffer + (((6-X)*8)-1)

    ac = 0;

    pt = A(0);      /* pt points to least significant byte of A0  */

    /* Add up first 8 byte word, no need to add ps2 */
    ps1 = A(3);        /* ps1 points to least significant byte of S1 (A3) */
    ps3 = A(5);        /* ps3 points to least significant byte of S3 (A5)*/

    /* Each block processes one 32 bit word, big endian, using byte operations */
    ac += *pt + *ps1--; ac += *ps3--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps1--; ac += *ps3--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps1--; ac += *ps3--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps1--; ac += *ps3--; *pt-- = ac; ac >>= 8;

    ac += *pt + *ps1--; ac += *ps3--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps1--; ac += *ps3--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps1--; ac += *ps3--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps1--; ac += *ps3--; *pt-- = ac; ac >>= 8;

    /* Add up second 8 byte word, all three S words are used here */
    ps1 = A(3); ps2 = A(4); ps3 = A(5);

    ac += *pt + *ps1--; ac += *ps2--; ac += *ps3--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps1--; ac += *ps2--; ac += *ps3--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps1--; ac += *ps2--; ac += *ps3--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps1--; ac += *ps2--; ac += *ps3--; *pt-- = ac; ac >>= 8;

    ac += *pt + *ps1--; ac += *ps2--; ac += *ps3--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps1--; ac += *ps2--; ac += *ps3--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps1--; ac += *ps2--; ac += *ps3--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps1--; ac += *ps2--; ac += *ps3--; *pt-- = ac; ac >>= 8;

    /* Add up third 8 byte word, no need to add S1 word */
    ps2 = A(4); ps3 = A(5);

    ac += *pt + *ps2--; ac += *ps3--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps2--; ac += *ps3--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps2--; ac += *ps3--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps2--; ac += *ps3--; *pt-- = ac; ac >>= 8;

    ac += *pt + *ps2--; ac += *ps3--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps2--; ac += *ps3--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps2--; ac += *ps3--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps2--; ac += *ps3--; *pt-- = ac; ac >>= 8;

    /* In this function we cannot have a negative carry and at most a carry of 2
     * thus just subtract the modulo until we are less than modulo
     */
    bnSetQ(r, 0);

    *(A(3)) = ac;      /* Store the carry */
    bnInsertBigBytes(r, A(3), 0, 25);  /* 25: 3 * 8 byte words + 1 carry byte */
    while (bnCmp(r, modulo) >= 0) {
        bnSub(r, modulo);
    }
    return 0;
}
#undef A

/* new modulo for 256bit curve */
static int newMod256(BigNum *r, const BigNum *a, const BigNum *modulo)
{
    unsigned char buffer[200] = {0};
    unsigned char *pt;
    unsigned char *ps1;
    unsigned char *ps2;
    unsigned char *ps3;
    unsigned char *ps4;

    unsigned char *pd1;
    unsigned char *pd2;
    unsigned char *pd3;
    unsigned char *pd4;
    short ac;
    int cmp;

    /* Binary big number representation in PolarSSL is always big endian
     *
     * the least significant byte starts at byte offset 63
     *
     *                                                                    T
     *                                                  /-----------------^------------------\
     *          A15  A14  A13  A12  A11  A10  A9   A8   A7   A6   A5   A4   A3   A2   A1   A0
     *        |----+----|----+----|----+----|----+----|----+----|----+----|----+----|----+----|
     * offset 0    4    8   12   16   20   24   28   32   36   40   44   48   52   56   60    64
     *
     * T  = (  A7 ||  A6 ||  A5 ||  A4 ||  A3 ||  A2 ||  A1 ||  A0 )
     *
     * S1 = ( A15 || A14 || A13 || A12 || A11 ||  00 ||  00 ||  00 )
     * S2 = (  00 || A15 || A14 || A13 || A12 ||  00 ||  00 ||  00 )
     * S3 = ( A15 || A14 ||  00 ||  00 ||  00 || A10 ||  A9 ||  A8 )
     * S4 = (  A8 || A13 || A15 || A14 || A13 || A11 || A10 ||  A9 )
     * D1 = ( A10 ||  A8 ||  00 ||  00 ||  00 || A13 || A12 || A11 )
     * D2 = ( A11 ||  A9 ||  00 ||  00 || A15 || A14 || A13 || A12 )
     * D3 = ( A12 ||  00 || A10 ||  A9 ||  A8 || A15 || A14 || A13 )
     * D4 = ( A13 ||  00 || A11 || A10 ||  A9 ||  00 || A15 || A14 )
     *
     * perform B = T + 2*S1 + 2*S2 + S3 + S4 - D1 - D2 - D3 - D4 mod p
     *
     * TODO: error check if input variable is > modulo^2 (do normal mpi_mod_mpi),
     */

    cmp = bnCmp(modulo, a);
    if (cmp == 0) {             /* a is equal modulo, set resul to zero */
        bnSetQ(r, 0);
        return 0;
    }
    if (cmp > 0) {              /* modulo is greater than a - copya to r and return it */
        bnCopy(r, a);
        return 0;
    }
    bnExtractBigBytes(a, buffer, 0, bnBytes(modulo)*2);

    /* 16 'A' words, each word is 4 byte. Compute offset to least significant byte of word X */
#define A(X) buffer + (((16-X)*4)-1)

    ac = 0;

    pt = A(0);          /* pt points to least significant byte of A0  */

    /* Set up to add up data that goes into A0 (right-most column abover); S1, S2 not used */
    ps3 = A(8);         /* ps3 points to least significant byte of S3 */
    ps4 = A(9);         /* ps4 points to least significant byte of S4 */
    pd1 = A(11);        /* pd1 points to least significant byte of D1 */
    pd2 = A(12);        /* pd2 points to least significant byte of D2 */
    pd3 = A(13);        /* pd3 points to least significant byte of D3 */
    pd4 = A(14);        /* pd4 points to least significant byte of D4 */

    /* Each block processes one 32 bit word, big endian, using byte operations */
    ac += *pt + *ps3--; ac += *ps4--; ac -= *pd1--; ac -= *pd2--; ac -= *pd3--; ac -= *pd4--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps3--; ac += *ps4--; ac -= *pd1--; ac -= *pd2--; ac -= *pd3--; ac -= *pd4--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps3--; ac += *ps4--; ac -= *pd1--; ac -= *pd2--; ac -= *pd3--; ac -= *pd4--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps3--; ac += *ps4--; ac -= *pd1--; ac -= *pd2--; ac -= *pd3--; ac -= *pd4--; *pt-- = ac; ac >>= 8;

    /* Set up to add up data that goes into A1; S1, S2 not used */
    ps3 = A(9);  ps4 = A(10); pd1 = A(12); pd2 = A(13); pd3 = A(14); pd4 = A(15);
    ac += *pt + *ps3--; ac += *ps4--; ac -= *pd1--; ac -= *pd2--; ac -= *pd3--; ac -= *pd4--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps3--; ac += *ps4--; ac -= *pd1--; ac -= *pd2--; ac -= *pd3--; ac -= *pd4--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps3--; ac += *ps4--; ac -= *pd1--; ac -= *pd2--; ac -= *pd3--; ac -= *pd4--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps3--; ac += *ps4--; ac -= *pd1--; ac -= *pd2--; ac -= *pd3--; ac -= *pd4--; *pt-- = ac; ac >>= 8;

    /* Set up to add up data that goes into A2; S1, S2, D4 not used */
    ps3 = A(10); ps4 = A(11); pd1 = A(13); pd2 = A(14); pd3 = A(15);
    ac += *pt + *ps3--; ac += *ps4--; ac -= *pd1--; ac -= *pd2--; ac -= *pd3--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps3--; ac += *ps4--; ac -= *pd1--; ac -= *pd2--; ac -= *pd3--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps3--; ac += *ps4--; ac -= *pd1--; ac -= *pd2--; ac -= *pd3--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps3--; ac += *ps4--; ac -= *pd1--; ac -= *pd2--; ac -= *pd3--; *pt-- = ac; ac >>= 8;

    /* Set up to add up data that goes into A3; S3, D1 not used */
    ps1 = A(11); ps2 = A(12); ps4 = A(13); pd2 = A(15); pd3 = A(8); pd4 = A(9);
    ac += *pt + *ps1;ac += *ps1--;  ac += *ps2;ac += *ps2--;  ac += *ps4--; ac -= *pd2--; ac -= *pd3--; ac -= *pd4--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps1;ac += *ps1--;  ac += *ps2;ac += *ps2--;  ac += *ps4--; ac -= *pd2--; ac -= *pd3--; ac -= *pd4--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps1;ac += *ps1--;  ac += *ps2;ac += *ps2--;  ac += *ps4--; ac -= *pd2--; ac -= *pd3--; ac -= *pd4--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps1;ac += *ps1--;  ac += *ps2;ac += *ps2--;  ac += *ps4--; ac -= *pd2--; ac -= *pd3--; ac -= *pd4--; *pt-- = ac; ac >>= 8;

    /* Set up to add up data that goes into A4; S3, D1, D2 not used */
    ps1 = A(12); ps2 = A(13); ps4 = A(14); pd3 = A(9); pd4 = A(10);
    ac += *pt + *ps1;ac += *ps1--;  ac += *ps2;ac += *ps2--;  ac += *ps4--; ac -= *pd3--; ac -= *pd4--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps1;ac += *ps1--;  ac += *ps2;ac += *ps2--;  ac += *ps4--; ac -= *pd3--; ac -= *pd4--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps1;ac += *ps1--;  ac += *ps2;ac += *ps2--;  ac += *ps4--; ac -= *pd3--; ac -= *pd4--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps1;ac += *ps1--;  ac += *ps2;ac += *ps2--;  ac += *ps4--; ac -= *pd3--; ac -= *pd4--; *pt-- = ac; ac >>= 8;

    /* Set up to add up data that goes into A5; S3, D1, D2 not used */
    ps1 = A(13); ps2 = A(14); ps4 = A(15); pd3 = A(10); pd4 = A(11);
    ac += *pt + *ps1;ac += *ps1--;  ac += *ps2;ac += *ps2--;  ac += *ps4--; ac -= *pd3--; ac -= *pd4--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps1;ac += *ps1--;  ac += *ps2;ac += *ps2--;  ac += *ps4--; ac -= *pd3--; ac -= *pd4--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps1;ac += *ps1--;  ac += *ps2;ac += *ps2--;  ac += *ps4--; ac -= *pd3--; ac -= *pd4--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps1;ac += *ps1--;  ac += *ps2;ac += *ps2--;  ac += *ps4--; ac -= *pd3--; ac -= *pd4--; *pt-- = ac; ac >>= 8;

    /* Set up to add up data that goes into A6; D3, D4 not used */
    ps1 = A(14); ps2 = A(15); ps3 = A(14); ps4 = A(13); pd1 = A(8); pd2 = A(9);
    ac += *pt + *ps1;ac += *ps1--;  ac += *ps2;ac += *ps2--;  ac += *ps3--; ac += *ps4--; ac -= *pd1--; ac -= *pd2--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps1;ac += *ps1--;  ac += *ps2;ac += *ps2--;  ac += *ps3--; ac += *ps4--; ac -= *pd1--; ac -= *pd2--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps1;ac += *ps1--;  ac += *ps2;ac += *ps2--;  ac += *ps3--; ac += *ps4--; ac -= *pd1--; ac -= *pd2--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps1;ac += *ps1--;  ac += *ps2;ac += *ps2--;  ac += *ps3--; ac += *ps4--; ac -= *pd1--; ac -= *pd2--; *pt-- = ac; ac >>= 8;

    /* Set up to add up data that goes into A7; S2 not used */
    ps1 = A(15); ps3 = A(15); ps4 = A(8); pd1 = A(10); pd2 = A(11); pd3 = A(12); pd4 = A(13);
    ac += *pt + *ps1;ac += *ps1--;  ac += *ps3--; ac += *ps4--; ac -= *pd1--; ac -= *pd2--; ac -= *pd3--; ac -= *pd4--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps1;ac += *ps1--;  ac += *ps3--; ac += *ps4--; ac -= *pd1--; ac -= *pd2--; ac -= *pd3--; ac -= *pd4--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps1;ac += *ps1--;  ac += *ps3--; ac += *ps4--; ac -= *pd1--; ac -= *pd2--; ac -= *pd3--; ac -= *pd4--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps1;ac += *ps1--;  ac += *ps3--; ac += *ps4--; ac -= *pd1--; ac -= *pd2--; ac -= *pd3--; ac -= *pd4--; *pt-- = ac; ac >>= 8;

    bnSetQ(r, 0);
    if (ac > 0) {
        *(A(8)) = ac;      /* Store the carry */
        bnInsertBigBytes(r, A(8), 0, 33);  /* 33: 8 * 4 byte words + 1 carry byte */
    }
    /* Negative carry requires that we add the modulo (carry * -1) times to make
     * the result positive. Then get the result mod(256).
     */
    else if (ac < 0) {
        int msb, maxMsb;

        *(A(8)) = 0;
        bnInsertBigBytes(r, A(8), 0, 33);  /* 33: 8 * 4 byte words + 1 carry byte */
        ac *= -1;
        while (ac--) {
            bnAdd(r, modulo);
        }
        maxMsb =  bnBits(modulo);
        msb = bnBits(r) - maxMsb;
        /* clear all bits above bit length of modulo. This length is 256 here, thus
         * we effectiviely doing a mod(256)
         */
        if (msb > 0) {
            BigNum tmp;
            bnBegin(&tmp);
            bnSetQ (&tmp, 1);
            bnLShift (&tmp, maxMsb);
            bnMod(r, r, &tmp);
            bnEnd(&tmp);
        }
    }
    else {
        *(A(8)) = 0;
        bnInsertBigBytes(r, A(8), 0, 33);  /* 33: 8 * 4 byte words + 1 carry byte */
    }
    while (bnCmp(r, modulo) >= 0) {
        bnSub(r, modulo);
    }
    return 0;
}
#undef A


/* new modulo for 384bit curve */
static int newMod384(BigNum *r, const BigNum *a, const BigNum *modulo)
{
    unsigned char buffer[200] = {0};
    unsigned char *pt;
    unsigned char *ps1;
    unsigned char *ps2;
    unsigned char *ps3;
    unsigned char *ps4;
    unsigned char *ps5;
    unsigned char *ps6;

    unsigned char *pd1;
    unsigned char *pd2;
    unsigned char *pd3;
    short ac;
    int cmp;

    /*
     *
     * the least significant byte starts at byte offset 97
     *
     *                                                                    T
     *                                        /---------------------------^----------------------------\
     *      A23 ......... A15  A14  A13  A12  A11  A10  A9   A8   A7   A6   A5   A4   A3   A2   A1   A0
     *    |----+ ...... |----+----|----+----|----+----|----+----|----+----|----+----|----+----|----+----|
     *
     * T  = (A11 || A10 ||  A9 ||  A8 ||  A7 ||  A6 ||  A5 ||  A4 ||  A3 ||  A2 ||  A1 ||  A0)

     * S1 = ( 00 ||  00 ||  00 ||  00 ||  00 || A23 || A22 || A21 ||  00 ||  00 ||  00 ||  00)
     * S2 = (A23 || A22 || A21 || A20 || A19 || A18 || A17 || A16 || A15 || A14 || A13 || A12)
     * S3 = (A20 || A19 || A18 || A17 || A16 || A15 || A14 || A13 || A12 || A23 || A22 || A21)
     * S4 = (A19 || A18 || A17 || A16 || A15 || A14 || A13 || A12 || A20 ||  00 || A23 ||  00)
     * S5 = ( 00 ||  00 ||  00 ||  00 || A23 || A22 || A21 || A20 ||  00 ||  00 ||  00 ||  00)
     * S6 = ( 00 ||  00 ||  00 ||  00 ||  00 ||  00 || A23 || A22 || A21 ||  00 ||  00 || A20)
     * D1 = (A22 || A21 || A20 || A19 || A18 || A17 || A16 || A15 || A14 || A13 || A12 || A23)
     * D2 = ( 00 ||  00 ||  00 ||  00 ||  00 ||  00 ||  00 || A23 || A22 || A21 || A20 ||  00)
     * D3 = ( 00 ||  00 ||  00 ||  00 ||  00 ||  00 ||  00 || A23 || A23 ||  00 ||  00 ||  00)
     *
     * perform B =  T + 2S1 + S2 + S3 + S4 + S5 + S6 – D1 – D2 – D3 mod p
     *
     * TODO: error check if input variable is > modulo^2 (do normal mpi_mod_mpi),
     *       optimize if input is already < modulo  (just copy over in this case).
     */

    cmp = bnCmp(modulo, a);
    if (cmp == 0) {             /* a is equal modulo, set resul to zero */
        bnSetQ(r, 0);
        return 0;
    }
    if (cmp > 0) {              /* modulo is greater than a - copy a to r and return it */
        bnCopy(r, a);
        return 0;
    }

    bnExtractBigBytes(a, buffer, 0, bnBytes(modulo)*2);

    /* 24 'A' words, each word is 4 byte. Compute offset to least significant byte of word X */
#define A(X) buffer + (((24-X)*4)-1)

    ac = 0;

    pt = A(0);      /* pt points to least significant byte of A0  */

    /* Set up to add data that goes into A0; S1, S4, S5, D2, D3 not used */
    ps2 = A(12); ps3 = A(21); ps6 = A(20); pd1 = A(23);

    /* Each block processes one 32 bit word, big endian, using byte operations */
    ac += *pt + *ps2--; ac += *ps3--; ac += *ps6--; ac -= *pd1--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps2--; ac += *ps3--; ac += *ps6--; ac -= *pd1--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps2--; ac += *ps3--; ac += *ps6--; ac -= *pd1--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps2--; ac += *ps3--; ac += *ps6--; ac -= *pd1--; *pt-- = ac; ac >>= 8;

    /* Set up to add data that goes into A1; S1, S5, S6, D3 not used */
    ps2 = A(13); ps3 = A(22); ps4 = A(23); pd1= A(12); pd2 = A(20);
    ac += *pt + *ps2--; ac += *ps3--; ac += *ps4--; ac -= *pd1--;  ac -= *pd2--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps2--; ac += *ps3--; ac += *ps4--; ac -= *pd1--;  ac -= *pd2--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps2--; ac += *ps3--; ac += *ps4--; ac -= *pd1--;  ac -= *pd2--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps2--; ac += *ps3--; ac += *ps4--; ac -= *pd1--;  ac -= *pd2--; *pt-- = ac; ac >>= 8;

    /* Set up to add data that goes into A2; S1, S4, S5, S6, D3 not used */
    ps2 = A(14); ps3 = A(23); pd1 = A(13); pd2 = A(21);
    ac += *pt + *ps2--; ac += *ps3--; ac -= *pd1--;  ac -= *pd2--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps2--; ac += *ps3--; ac -= *pd1--;  ac -= *pd2--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps2--; ac += *ps3--; ac -= *pd1--;  ac -= *pd2--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps2--; ac += *ps3--; ac -= *pd1--;  ac -= *pd2--; *pt-- = ac; ac >>= 8;

    /* Set up to add data that goes into A3; S1, S5, S6 not used */
    ps2 = A(15); ps3 = A(12); ps4 = A(20); ps6 = A(21); pd1 = A(14); pd2 = A(22); pd3 = A(23);
    ac += *pt + *ps2--; ac += *ps3--; ac += *ps4--; ac += *ps6--; ac -= *pd1--;  ac -= *pd2--; ac -= *pd3--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps2--; ac += *ps3--; ac += *ps4--; ac += *ps6--; ac -= *pd1--;  ac -= *pd2--; ac -= *pd3--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps2--; ac += *ps3--; ac += *ps4--; ac += *ps6--; ac -= *pd1--;  ac -= *pd2--; ac -= *pd3--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps2--; ac += *ps3--; ac += *ps4--; ac += *ps6--; ac -= *pd1--;  ac -= *pd2--; ac -= *pd3--; *pt-- = ac; ac >>= 8;

    /* Set up to add data that goes into A4 */
    ps1 = A(21); ps2 = A(16); ps3 = A(13); ps4 = A(12); ps5 = A(20); ps6 = A(22); pd1 = A(15); pd2 = A(23), pd3 = A(23);
    ac += *pt + *ps1;ac += *ps1--; ac += *ps2--; ac += *ps3--; ac += *ps4--; ac += *ps5--; ac += *ps6--; ac -= *pd1--;  ac -= *pd2--; ac -= *pd3--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps1;ac += *ps1--; ac += *ps2--; ac += *ps3--; ac += *ps4--; ac += *ps5--; ac += *ps6--; ac -= *pd1--;  ac -= *pd2--; ac -= *pd3--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps1;ac += *ps1--; ac += *ps2--; ac += *ps3--; ac += *ps4--; ac += *ps5--; ac += *ps6--; ac -= *pd1--;  ac -= *pd2--; ac -= *pd3--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps1;ac += *ps1--; ac += *ps2--; ac += *ps3--; ac += *ps4--; ac += *ps5--; ac += *ps6--; ac -= *pd1--;  ac -= *pd2--; ac -= *pd3--; *pt-- = ac; ac >>= 8;

    /* Set up to add data that goes into A5; D2, D3 not used */
    ps1 = A(22); ps2 = A(17); ps3 = A(14); ps4 = A(13); ps5 = A(21); ps6 = A(23); pd1 = A(16);
    ac += *pt + *ps1;ac += *ps1--; ac += *ps2--; ac += *ps3--; ac += *ps4--; ac += *ps5--; ac += *ps6--; ac -= *pd1--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps1;ac += *ps1--; ac += *ps2--; ac += *ps3--; ac += *ps4--; ac += *ps5--; ac += *ps6--; ac -= *pd1--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps1;ac += *ps1--; ac += *ps2--; ac += *ps3--; ac += *ps4--; ac += *ps5--; ac += *ps6--; ac -= *pd1--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps1;ac += *ps1--; ac += *ps2--; ac += *ps3--; ac += *ps4--; ac += *ps5--; ac += *ps6--; ac -= *pd1--; *pt-- = ac; ac >>= 8;

    /* Set up to add data that goes into A6; S6, D2, D3 not used */
    ps1 = A(23); ps2 = A(18); ps3 = A(15); ps4 = A(14); ps5 = A(22); pd1 = A(17);
    ac += *pt + *ps1;ac += *ps1--; ac += *ps2--; ac += *ps3--; ac += *ps4--; ac += *ps5--; ac -= *pd1--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps1;ac += *ps1--; ac += *ps2--; ac += *ps3--; ac += *ps4--; ac += *ps5--; ac -= *pd1--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps1;ac += *ps1--; ac += *ps2--; ac += *ps3--; ac += *ps4--; ac += *ps5--; ac -= *pd1--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps1;ac += *ps1--; ac += *ps2--; ac += *ps3--; ac += *ps4--; ac += *ps5--; ac -= *pd1--; *pt-- = ac; ac >>= 8;

    /* Set up to add data that goes into A7; S1, S6, D2, D3 not used */
    ps2 = A(19); ps3 = A(16); ps4 = A(15); ps5 = A(23); pd1 = A(18);
    ac += *pt + *ps2--; ac += *ps3--; ac += *ps4--; ac += *ps5--; ac -= *pd1--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps2--; ac += *ps3--; ac += *ps4--; ac += *ps5--; ac -= *pd1--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps2--; ac += *ps3--; ac += *ps4--; ac += *ps5--; ac -= *pd1--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps2--; ac += *ps3--; ac += *ps4--; ac += *ps5--; ac -= *pd1--; *pt-- = ac; ac >>= 8;

    /* Set up to add data that goes into A8; S1, S5, S6, D2, D3 not used */
    ps2 = A(20); ps3 = A(17); ps4 = A(16); pd1 = A(19);
    ac += *pt + *ps2--; ac += *ps3--; ac += *ps4--; ac -= *pd1--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps2--; ac += *ps3--; ac += *ps4--; ac -= *pd1--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps2--; ac += *ps3--; ac += *ps4--; ac -= *pd1--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps2--; ac += *ps3--; ac += *ps4--; ac -= *pd1--; *pt-- = ac; ac >>= 8;

    /* Set up to add data that goes into A9; S1, S5, S6, D2, D3 not used */
    ps2 = A(21); ps3 = A(18); ps4 = A(17); pd1 = A(20);
    ac += *pt + *ps2--; ac += *ps3--; ac += *ps4--; ac -= *pd1--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps2--; ac += *ps3--; ac += *ps4--; ac -= *pd1--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps2--; ac += *ps3--; ac += *ps4--; ac -= *pd1--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps2--; ac += *ps3--; ac += *ps4--; ac -= *pd1--; *pt-- = ac; ac >>= 8;

    /* Set up to add data that goes into A10; S1, S5, S6, D2, D3 not used */
    ps2 = A(22); ps3 = A(19); ps4 = A(18); pd1 = A(21);
    ac += *pt + *ps2--; ac += *ps3--; ac += *ps4--; ac -= *pd1--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps2--; ac += *ps3--; ac += *ps4--; ac -= *pd1--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps2--; ac += *ps3--; ac += *ps4--; ac -= *pd1--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps2--; ac += *ps3--; ac += *ps4--; ac -= *pd1--; *pt-- = ac; ac >>= 8;

    /* Set up to add data that goes into A10; S1, S5, S6, D2, D3 not used */
    ps2 = A(23); ps3 = A(20); ps4 = A(19); pd1 = A(22);
    ac += *pt + *ps2--; ac += *ps3--; ac += *ps4--; ac -= *pd1--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps2--; ac += *ps3--; ac += *ps4--; ac -= *pd1--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps2--; ac += *ps3--; ac += *ps4--; ac -= *pd1--; *pt-- = ac; ac >>= 8;
    ac += *pt + *ps2--; ac += *ps3--; ac += *ps4--; ac -= *pd1--; *pt-- = ac; ac >>= 8;

    bnSetQ(r, 0);
    if (ac > 0) {
        *(A(12)) = ac;      /* Store the carry */
        bnInsertBigBytes(r, A(12), 0, 49);  /* 49: 12 * 4 byte words + 1 carry byte */
    }
    /* Negative carry requires that we add the modulo (carry * -1) times to make
     * the result positive. Then get the result mod(256).
     */
    else if (ac < 0) {
        int msb, maxMsb;

        *(A(12)) = 0;
        bnInsertBigBytes(r, A(12), 0, 49);  /* 49: 12 * 4 byte words + 1 carry byte */
        ac *= -1;
        while (ac--) {
            bnAdd(r, modulo);
        }
        maxMsb =  bnBits(modulo);
        msb = bnBits(r) - maxMsb;
        /* clear all bits above bit length of modulo. This length is 384 here, thus
         * we effectiviely doing a mod(384)
         */
        if (msb > 0) {
            BigNum tmp;
            bnBegin(&tmp);
            bnSetQ (&tmp, 1);
            bnLShift (&tmp, maxMsb);
            bnMod(r, r, &tmp);
            bnEnd(&tmp);
        }
    }
    else {
        *(A(12)) = 0;
        bnInsertBigBytes(r, A(12), 0, 49);  /* 49: 12 * 4 byte words + 1 carry byte */
    }
    while (bnCmp(r, modulo) >= 0) {
        bnSub(r, modulo);
    }
    return 0;
}
#undef A


/* new modulo for 521bit curve, much easier because the prime for 521 is a real Mersenne prime */
static int newMod521(BigNum *r, const BigNum *a, const BigNum *modulo)
{
    unsigned char buf1[200] = {0};
    unsigned char buf2[200] = {0};
    unsigned char *p1;
    unsigned char *p2;
    size_t modSize;
    short ac = 0;
    unsigned int i;
    int cmp;

    /* TODO: check if a is > modulo^2 */
#if 0
    if (a->s < 0)               /* is it a negative value? */
        return bnMod(r, a, modulo);
#endif
    cmp = bnCmp(modulo, a);
    if (cmp == 0) {             /* a is equal modulo, set resul to zero */
        bnSetQ(r, 0);
        return 0;
    }
    bnCopy(r, a);
    if (cmp > 0) {              /* modulo is greater than a - return the prepared r */
        return 0;
    }
    modSize = bnBytes(modulo);

    bnExtractBigBytes(a, buf1, 0, modSize*2); /* a must be less modulo^2 */
    buf1[modSize] &= 1;                   /* clear all bits except least significat */

    bnRShift(r, 521);
    bnExtractBigBytes(r, buf2, 0, modSize*2);
    buf2[modSize] &= 1;

    p1 = &buf2[131];            /* p1 is pointer to A0 */
    p2 = &buf1[131];            /* p2 is pointer to A1 */

    for (i = 0; i < modSize; i++) {
        ac += *p1 + *p2--; *p1-- = ac; ac >>= 8;
    }
    bnSetQ(r, 0);
    bnInsertBigBytes(r, p1+1, 0, modSize);

    while (bnCmp(r, modulo) >= 0) {
        bnSub(r, modulo);
    }
    return 0;
}

