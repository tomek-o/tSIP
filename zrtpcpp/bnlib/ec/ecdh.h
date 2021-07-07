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

#ifndef _ECDH_H_
#define _ECDH_H_
/**
 * @file ec.h
 * @brief Elliptic Diffie-Hellman functions for bnlib
 * @defgroup BNLIB_EC Elliptic curve functions
 * @{
 */

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Takes a secret large random number and computes the public EC point.
 *
 * @param curve is the curve to use.
 *
 * @param Q the functions writes the computed public point in this parameter.
 *
 * @param d is the secret random number.
 *
 * @return @c true (!0) if public key was computed, @c false otherwise.
 *
 * @sa ecGenerateRandomNumber
 */
int ecdhGeneratePublic(const EcCurve *curve, EcPoint *Q, const BigNum *d);

/**
 * @brief Computes the key agreement value.
 *
 * Takes the public EC point of the other party and applies the EC DH algorithm
 * to compute the agreed value.
 *
 * @param curve is the curve to use, must be the same curve as used in
 *              @c ecdhGeneratePublic.
 *
 * @param agreemtn the functions writes the computed agreed value in this parameter.
 *
 * @param Q is the other party's public point.
 *
 * @param d is the secret random number.
 */
int ecdhComputeAgreement(const EcCurve *curve, BigNum *agreement, const EcPoint *Q, const BigNum *d);

#ifdef __cplusplus
}
#endif
/**
 * @}
 */

#endif