// -----------------------------------------------------------------------------
// Copyright 2020 Rui Liu (liurui39660) and Siddharth Bhatia (bhatiasiddharth)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// 	http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// -----------------------------------------------------------------------------

#pragma once

#include <cmath>
#include <string>
#include "CountMinSketch.hpp"

namespace MIDAS {
    struct NormalCore {
        unsigned long timestamp = 1;
        unsigned long *const index; // Pre-compute the index to-be-modified, thanks to the same structure of CMSs
        CountMinSketch numCurrent, numTotal;

        NormalCore(int numRow, int numColumn) :
                index(new unsigned long[numRow]),
                numCurrent(numRow, numColumn),
                numTotal(numCurrent) {}

        virtual ~NormalCore() {
            delete[] index;
        }

        static double ComputeScore(double a, double s, double t) {
            return s == 0 || t - 1 == 0 ? 0 : pow((a - s / t) * t, 2) / (s * (t - 1));
        }

        static unsigned long HashStr(const std::string &str) {
            unsigned long hash = 5381;
            for (auto c: str) {
                hash = ((hash << 5) + hash) + (unsigned long) c; /* hash * 33 + c */
            }
            return hash;
        }

        double operator()(const std::string &source, const std::string &destination, unsigned long timestamp) {
            unsigned long intSource = HashStr(source);
            unsigned long intDestination = HashStr(destination);

            return this->operator()(intSource, intDestination, timestamp);
        }

        double operator()(unsigned long source, unsigned long destination, unsigned long timestamp) {
            if (this->timestamp < timestamp) {
                numCurrent.ClearAll();
                this->timestamp = timestamp;
            }
            numCurrent.Hash(index, source, destination);
            numCurrent.Add(index);
            numTotal.Add(index);
            return ComputeScore(numCurrent(index), numTotal(index), timestamp);
        }

    };
}
