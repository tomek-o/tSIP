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

#include <ec/ec.h>
#include <ec/ecdh.h>

int ecdhGeneratePublic(const EcCurve *curve, EcPoint *Q, const BigNum *d)
{
    EcPoint G;

    INIT_EC_POINT(&G);
    SET_EC_BASE_POINT(curve, &G);

    ecMulPointScalar(curve, Q, &G, d);
    ecGetAffine(curve, Q, Q);

    FREE_EC_POINT(&G);

    return ecCheckPubKey(curve, Q);
}

int ecdhComputeAgreement(const EcCurve *curve, BigNum *agreement, const EcPoint *Q, const BigNum *d)
{
    EcPoint t0;

    INIT_EC_POINT(&t0);

    ecMulPointScalar(curve, &t0, Q, d);
    ecGetAffine(curve, &t0, &t0);
    /* TODO: check for infinity here */

    bnCopy(agreement, t0.x);

    FREE_EC_POINT(&t0);

    return 0;
}
