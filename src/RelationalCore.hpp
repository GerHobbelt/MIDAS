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

#include "CountMinSketch.hpp"

namespace MIDAS {
struct RelationalCore {
	unsigned long timestamp = 1;
	const double factor;
	unsigned long* const indexEdge; // Pre-compute the index to-be-modified, thanks to the same structure of CMSs
	unsigned long* const indexSource;
	unsigned long* const indexDestination;
	CountMinSketch numCurrentEdge, numTotalEdge;
	CountMinSketch numCurrentSource, numTotalSource;
	CountMinSketch numCurrentDestination, numTotalDestination;

	RelationalCore(unsigned long numRow, unsigned long numColumn, double factor = 0.5):
		factor(factor),
		indexEdge(new unsigned long[numRow]),
		indexSource(new unsigned long[numRow]),
		indexDestination(new unsigned long[numRow]),
		numCurrentEdge(numRow, numColumn),
		numTotalEdge(numCurrentEdge),
		numCurrentSource(numRow, numColumn),
		numTotalSource(numCurrentSource),
		numCurrentDestination(numRow, numColumn),
		numTotalDestination(numCurrentDestination) { }

	virtual ~RelationalCore() {
		delete[] indexEdge;
		delete[] indexSource;
		delete[] indexDestination;
	}

	static double ComputeScore(double a, double s, double t) {
		return s == 0 || t - 1 == 0 ? 0 : pow((a - s / t) * t, 2) / (s * (t - 1));
	}

	double operator()(unsigned long source, unsigned long destination, unsigned long timestamp) {
		if (this->timestamp < timestamp) {
			numCurrentEdge.MultiplyAll(factor);
			numCurrentSource.MultiplyAll(factor);
			numCurrentDestination.MultiplyAll(factor);
			this->timestamp = timestamp;
		}
		numCurrentEdge.Hash(indexEdge, source, destination);
		numCurrentEdge.Add(indexEdge);
		numTotalEdge.Add(indexEdge);
		numCurrentSource.Hash(indexSource, source);
		numCurrentSource.Add(indexSource);
		numTotalSource.Add(indexSource);
		numCurrentDestination.Hash(indexDestination, destination);
		numCurrentDestination.Add(indexDestination);
		numTotalDestination.Add(indexDestination);
		return std::max({
			ComputeScore(numCurrentEdge(indexEdge), numTotalEdge(indexEdge), timestamp),
			ComputeScore(numCurrentSource(indexSource), numTotalSource(indexSource), timestamp),
			ComputeScore(numCurrentDestination(indexDestination), numTotalDestination(indexDestination), timestamp),
		});
	}
};
}
