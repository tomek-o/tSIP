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

/*
 * Authors: Werner Dittmann <Werner.Dittmann@t-online.de>
 */

#ifndef _ZRTPSTATES_H_
#define _ZRTPSTATES_H_

/**
 * @file ZrtpStates.h
 * @brief The ZRTP state switching class
 *
 * @ingroup GNU_ZRTP
 * @{
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

class __EXPORT ZrtpStateClass;
/**
 * This structure hold the state name as enum (int) number and the pointer to
 * the functions that handles the various triggers that can occur in a state.
 */
typedef struct  {
    int32_t stateName;                      ///< The state number
    void (ZrtpStateClass::* handler)(void); ///< The state handler
} state_t;

/**
 * Implement a simple state switching.
 *
 * This class provides functions that manage the states and the event handler
 * functions. Its a very simple implementation.
 *
 * @author Werner Dittmann <Werner.Dittmann@t-online.de>
 */

class __EXPORT ZrtpStates {
 public:

    /// Create an initialize state switching
    ZrtpStates(state_t* const zstates,
           const int32_t numStates,
           const int32_t initialState):
    numStates(numStates), states(zstates), state(initialState) {}

    /// Call a state handler
    int32_t processEvent(ZrtpStateClass& zsc) {
        (zsc.*states[state].handler)();
        return 0;
    }

    /// Check if in specified state
    bool inState(const int32_t s) { return ((s == state)); }

    /// Set the next state
    void nextState(int32_t s)        { state = s; }

 private:
    const int32_t numStates;
    const state_t* states;
    int32_t  state;

    ZrtpStates();
};

/**
 * @}
 */
#endif  //ZRTPSTATES

