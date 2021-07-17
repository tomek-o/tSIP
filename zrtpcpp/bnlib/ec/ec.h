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

#ifndef _EC_H_
#define _EC_H_

#include <bn.h>

/**
 * @file ec.h
 * @brief Elliptic curve functions for bnlib
 * @defgroup BNLIB_EC Elliptic curve functions
 * @{
 */

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct BigNum BigNum;

typedef enum {
    NIST192P = 1,
    NIST224P = 2,
    NIST256P = 3,
    NIST384P = 4,
    NIST521P = 5,
    Curve25519 = 10,
    Curve3617  = 11
} Curves;

/**
 * \brief This structure contains the x, y affine coordinates and the z value if we
 *        use projective coordinates during EC point arithmetic.
 */
typedef struct _EcPoint {
    BigNum *x, *y, *z;
    BigNum tx, ty, tz;
} EcPoint;

/**
 * @brief This structure contains the value of EC curves over Prime Fields.
 *
 * The for NIST curves the field names correspond to the variable names defined in 
 * NIST FIPS 186-3, E.1.2. The <b>a</b> curve parameter is the constant -3 and is 
 * computed during initialization of the curve structure.
 *
 * For other curves, for example curve3917 we have less parameters to fill in, mostly
 * the prime number, the base point, etc. Refer to the curve's initialization function
 * about the use of the fileds.
 */
struct EcCurve;
struct EcCurve {
    Curves id;
    BigNum _p;
    BigNum _n;
    BigNum _SEED;
    BigNum _c;
    BigNum _a;
    BigNum _b;
    BigNum _Gx;
    BigNum _Gy;
    /* Pointers to the BigNum structures, for better readability mainly */
    BigNum *p;
    BigNum *n;
    BigNum *SEED;
    BigNum *c;
    BigNum *a;
    BigNum *b;
    BigNum *Gx;
    BigNum *Gy;
    /* some scratch pad variables, the EC algorithms use them to 
       avoid to much memory allocation/deallocatio0n overhead */
  BigNum _S1, _U1, _H, _R, _t0, _t1, _t2, _t3;
  BigNum *S1, *U1, *H, *R, *t0, *t1, *t2, *t3;
  int (*affineOp)(const struct EcCurve *curve, EcPoint *R, const EcPoint *P);
  int (*doubleOp)(const struct EcCurve *curve, EcPoint *R, const EcPoint *P);
  int (*addOp)(const struct EcCurve *curve, EcPoint *R, const EcPoint *P, const EcPoint *Q);
  int (*modOp)(BigNum *, const BigNum *, const BigNum *);
  int (*checkPubOp)(const struct EcCurve *curve, const EcPoint *pub);
  int (*randomOp)(const struct EcCurve *curve, BigNum *d);
  int (*mulScalar)(const struct EcCurve *curve, EcPoint *R, const EcPoint *P, const BigNum *scalar);

};

typedef struct EcCurve EcCurve;
typedef EcCurve NistECpCurve;

/**
 * \brief          Marco to initialize a EC point structure.
 *
 * \param P        Address of the EC point structure
 */
#define INIT_EC_POINT(P) {EcPoint *e = P; e->x = &e->tx; e->y = &e->ty; e->z = &e->tz; bnBegin(e->x); bnBegin(e->y); bnBegin(e->z);}

/**
 * \brief          Marco to free a EC point structure.
 *
 * \param P        Address of the EC point structure
 */
#define FREE_EC_POINT(P) {EcPoint *e = P; bnEnd(e->x); bnEnd(e->y); bnEnd(e->z);}

/**
 * \brief          Marco to set a EC point structure to the curve's base point.
 *
 * \param C        Address of the NistECpCurve structure.
 *
 * \param P        Address of the EC point structure.
 */
#define SET_EC_BASE_POINT(C, P) {EcPoint *e = P;  const EcCurve *c = C; bnCopy(e->x, c->Gx); bnCopy(e->y, c->Gy); bnSetQ(e->z, 1);}

/*
 * EC point helper functions
 */
extern void ecInitPoint(EcPoint *P);

extern void ecFreePoint(EcPoint *P);

extern void ecSetBasePoint(EcCurve *C, EcPoint *P);

/**
 * \brief          Get NIST EC curve parameters.
 *
 *                 Before reusing a EC curve structure make sure to call ecFreeCurveNistECp
 *                 to return memory.
 *
 * \param curveId  Which curve to initialize
 *
 * \param curve    Pointer to a EcCurve structure
 *
 * \return         0 if successful
 *
 * \note           Call ecFreeCurveNistECp to return allocated memory.
 */
int ecGetCurveNistECp(Curves curveId, NistECpCurve *curve);


/**
 * \brief          Free EC curve parameters.
 *
 * \param curve    Pointer to a EcCurve structure
 *
 * \note           Curve parameters must be initialized calling ecGetCurveNistECp.
 */
void ecFreeCurveNistECp(EcCurve *curve);

/**
 * \brief          Double an EC point.
 *
 *                 This function uses affine coordinates to perform the computations. For
 *                 further reference see RFC 6090 or the standard work <i>Guide to Elliptic
 *                 Curve Cryptography</i>.
 *
 * \param          curve  Address of EC curve structure
 * \param          R      Address of resulting EC point structure
 * \param          P      Address of the EC point structure
 *
 * \return         0 if successful
 */
int ecDoublePoint(const EcCurve *curve, EcPoint *R, const EcPoint *P);

/**
 * \brief          Add two EC points.
 *
 *                 This function uses affine coordinates to perform the computations. For
 *                 further reference see RFC 6090 or the standard work <i>Guide to Elliptic
 *                 Curve Cryptography</i>.
 *
 * \param          curve  Address of EC curve structure
 * \param          R      Address of resulting EC point structure
 * \param          P      Address of the first EC point structure
 * \param          Q      Address of the second EC point structure
 *
 * \return         0 if successful
 */
int ecAddPoint(const EcCurve *curve, EcPoint *R, const EcPoint *P, const EcPoint *Q);

/**
 * \brief          Mulitply an EC point with a scalar value.
 *
 * \param          curve  Address of EC curve structure
 * \param          R      Address of resulting EC point structure
 * \param          P      Address of the EC point structure
 * \param          scalar Address of the scalar multi-precision integer value
 *
 * \return         0 if successful
 */
int ecMulPointScalar(const EcCurve *curve, EcPoint *R, const EcPoint *P, const BigNum *scalar);

/**
 * \brief          Convert an EC point from Jacobian projective coordinates to normal affine x/y coordinates.
 *
 * \param          curve  Address of EC curve structure
 * \param          R      Address of EC point structure that receives the x/y coordinates
 * \param          P      Address of the EC point structure that contains the jacobian x/y/z coordinates.
 *
 * \return         0 if successful
 */
int ecGetAffine(const EcCurve *curve, EcPoint *R, const EcPoint *P);

/**
 * @brief Generate a random number.
 *
 * The method generates a random number and checks if it matches the curve restricitions.
 * Use this number as ECDH private key.
 *
 * @param curve the NIST curve to use.
 *
 * @param d receives the generated random number.
 * 
 * @return 0 if random data generation is OK, <0 in case of an error.
 */
int ecGenerateRandomNumber(const NistECpCurve *curve, BigNum *d);

/**
 * @brief Check a public key.
 *
 * The method checks if a public key is valid. For NIST curves it uses the
 * ECC Partial Validation, NIST SP800-56A section 5.6.2.6
 * 
 * For other curves it computes the equation and compares the left hand and 
 * the right handresults. If they are equal the point is on the curve.
 *
 * @param curve the curve to use.
 *
 * @param pub the public key to check.
 *
 * @returns true (!0) if the check was ok, false (0) otherwise.
 *
 * @note The function uses some scratch pad variable of the NistECpCurve structure.
 */
int ecCheckPubKey(const EcCurve *curve, const EcPoint *pub);

int ecGetCurvesCurve(Curves curveId, EcCurve *curve);

void ecFreeCurvesCurve(EcCurve *curve);

/**
 * This is a special function for DJB's curve 25519. Actually it's the scalar multiplication
 * mypublic = basepoint * secret
 */
int curve25519_donna(unsigned char *mypublic, const unsigned char *secret, const unsigned char *basepoint);

/*
 * Some additional functions that are not available in bnlib
 */
int bnAddMod_ (struct BigNum *rslt, struct BigNum *n1, struct BigNum *mod);

int bnAddQMod_ (struct BigNum *rslt, unsigned n1, struct BigNum *mod);

int bnSubMod_ (struct BigNum *rslt, struct BigNum *n1, struct BigNum *mod);

int bnSubQMod_ (struct BigNum *rslt, unsigned n1, struct BigNum *mod);

int bnMulMod_ (struct BigNum *rslt, struct BigNum *n1, struct BigNum *n2, struct BigNum *mod, const EcCurve *curve);

int bnMulQMod_ (struct BigNum *rslt, struct BigNum *n1, unsigned n2, struct BigNum *mod, const EcCurve *curve);

int bnSquareMod_ (struct BigNum *rslt, struct BigNum *n1, struct BigNum *mod, const EcCurve *curve);

void ecDestroyStatic(void);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif
