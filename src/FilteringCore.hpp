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
    struct FilteringCore {
        const int numRow, numColumn;
        const double threshold;
        unsigned long timestamp = 1;
        const double factor;
        const int lenData;
        unsigned long *const indexEdge; // Pre-compute the index to-be-modified, thanks to the Same-Layout Assumption
        unsigned long *const indexSource;
        unsigned long *const indexDestination;
        CountMinSketch numCurrentEdge, numTotalEdge, scoreEdge;
        CountMinSketch numCurrentSource, numTotalSource, scoreSource;
        CountMinSketch numCurrentDestination, numTotalDestination, scoreDestination;
        double timestampReciprocal = 0;
        bool *const shouldMerge;

        FilteringCore(int numRow, int numColumn, double threshold, double factor = 0.5) :
                numRow(numRow),
                numColumn(numColumn),
                threshold(threshold),
                factor(factor),
                lenData(numRow *
                        numColumn), // I assume all CMSs have same size, but Same-Layout Assumption is not that strict
                indexEdge(new unsigned long[numRow]),
                indexSource(new unsigned long[numRow]),
                indexDestination(new unsigned long[numRow]),
                numCurrentEdge(numRow, numColumn),
                numTotalEdge(numCurrentEdge),
                scoreEdge(numCurrentEdge),
                numCurrentSource(numRow, numColumn),
                numTotalSource(numCurrentSource),
                scoreSource(numCurrentSource),
                numCurrentDestination(numRow, numColumn),
                numTotalDestination(numCurrentDestination),
                scoreDestination(numCurrentDestination),
                shouldMerge(new bool[numRow * numColumn]) {}

        FilteringCore(int numRow, int numColumn, double threshold, unsigned long timestamp, double factor,
                      std::vector<unsigned long> indexEdge, std::vector<unsigned long> indexSource,
                      std::vector<unsigned long> indexDestination, CountMinSketch *numCurrentEdge,
                      CountMinSketch *numTotalEdge, CountMinSketch *scoreEdge, CountMinSketch *numCurrentSource,
                      CountMinSketch *numTotalSource, CountMinSketch *scoreSource,
                      CountMinSketch *numCurrentDestination, CountMinSketch *numTotalDestination,
                      CountMinSketch *scoreDestination, double timestampReciprocal, std::vector<bool> shouldMerge) :
                numRow(numRow),
                numColumn(numColumn),
                threshold(threshold),
                factor(factor),
                lenData(numRow * numColumn),
                timestamp(timestamp),
                indexEdge(new unsigned long[numRow]),
                indexSource(new unsigned long[numRow]),
                indexDestination(new unsigned long[numRow]),
                numCurrentEdge(*numCurrentEdge),
                numTotalEdge(*numTotalEdge),
                scoreEdge(*scoreEdge),
                numCurrentSource(*numCurrentSource),
                numTotalSource(*numTotalSource),
                scoreSource(*scoreSource),
                numCurrentDestination(*numCurrentDestination),
                numTotalDestination(*numTotalDestination),
                scoreDestination(*scoreDestination),
                timestampReciprocal(timestampReciprocal),
                shouldMerge(new bool[numRow * numColumn]) {
            std::copy(indexSource.begin(), indexSource.end(), this->indexSource);
            std::copy(indexEdge.begin(), indexEdge.end(), this->indexEdge);
            std::copy(indexDestination.begin(), indexDestination.end(), this->indexDestination);
            std::copy(shouldMerge.begin(), shouldMerge.end(), this->shouldMerge);
        }

        virtual ~FilteringCore() {
            delete[] indexEdge;
            delete[] indexSource;
            delete[] indexDestination;
            delete[] shouldMerge;
        }

        static double ComputeScore(double a, double s, double t) {
            return s == 0 ? 0 : pow(a + s - a * t, 2) /
                                (s * (t - 1)); // If t == 1, then s == 0, so no need to check twice
        }

        void ConditionalMerge(const double *current, double *total, const double *score) const {
            for (int i = 0; i < lenData; i++)
                shouldMerge[i] = score[i] < threshold;
            for (int i = 0, I = lenData; i < I; i++) // Vectorization
                total[i] += shouldMerge[i] * current[i] + (true - shouldMerge[i]) * total[i] * timestampReciprocal;
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
                ConditionalMerge(numCurrentEdge.data, numTotalEdge.data, scoreEdge.data);
                ConditionalMerge(numCurrentSource.data, numTotalSource.data, scoreSource.data);
                ConditionalMerge(numCurrentDestination.data, numTotalDestination.data, scoreDestination.data);
                numCurrentEdge.MultiplyAll(factor);
                numCurrentSource.MultiplyAll(factor);
                numCurrentDestination.MultiplyAll(factor);
                timestampReciprocal = 1.f / (timestamp - 1); // So I can skip an if-statement
                this->timestamp = timestamp;
            }
            numCurrentEdge.Hash(indexEdge, source, destination);
            numCurrentEdge.Add(indexEdge);
            numCurrentSource.Hash(indexSource, source);
            numCurrentSource.Add(indexSource);
            numCurrentDestination.Hash(indexDestination, destination);
            numCurrentDestination.Add(indexDestination);
            return std::max({
                                    scoreEdge.Assign(indexEdge,
                                                     ComputeScore(numCurrentEdge(indexEdge), numTotalEdge(indexEdge),
                                                                  timestamp)),
                                    scoreSource.Assign(indexSource, ComputeScore(numCurrentSource(indexSource),
                                                                                 numTotalSource(indexSource),
                                                                                 timestamp)),
                                    scoreDestination.Assign(indexDestination,
                                                            ComputeScore(numCurrentDestination(indexDestination),
                                                                         numTotalDestination(indexDestination),
                                                                         timestamp)),
                            });
        }

        json SerializeAsJson() {
            json model = json();
            auto copyIndexEdge = json::array();
            auto copyIndexSource = json::array();
            auto copyIndexDestination = json::array();
            auto copyShouldMerge = json::array();


            std::copy(indexEdge, indexEdge + numRow, std::back_inserter(copyIndexEdge));
            std::copy(indexSource, indexSource + numRow, std::back_inserter(copyIndexSource));
            std::copy(indexDestination, indexDestination + numRow, std::back_inserter(copyIndexDestination));
            std::copy(shouldMerge, shouldMerge + (numRow * numColumn), std::back_inserter(copyShouldMerge));

            model["numRow"] = numRow;
            model["numColumn"] = numColumn;
            model["threshold"] = threshold;
            model["timestamp"] = timestamp;
            model["factor"] = factor;
            model["indexEdge"] = copyIndexEdge;
            model["indexSource"] = copyIndexSource;
            model["indexDestination"] = copyIndexDestination;
            model["numCurrentEdge"] = numCurrentEdge.SerializeAsJson();
            model["numTotalEdge"] = numTotalEdge.SerializeAsJson();
            model["scoreEdge"] = scoreEdge.SerializeAsJson();
            model["numCurrentSource"] = numCurrentSource.SerializeAsJson();
            model["numTotalSource"] = numTotalSource.SerializeAsJson();
            model["scoreSource"] = scoreSource.SerializeAsJson();
            model["numCurrentDestination"] = numCurrentDestination.SerializeAsJson();
            model["numTotalDestination"] = numTotalDestination.SerializeAsJson();
            model["scoreDestination"] = scoreDestination.SerializeAsJson();
            model["timestampReciprocal"] = timestampReciprocal;
            model["shouldMerge"] = copyShouldMerge;

            return model;
        }

        int DumpToFile(const std::string &path) {
            int rc = 0;
            std::ofstream out(path);
            try {
                json model = SerializeAsJson();
                out << model.dump(4);
            }
            catch (std::exception &e) {
                std::cout << e.what() << std::endl;
            }
            catch (...) {
                rc = -1;
            }
            return rc;
        }

        static FilteringCore *LoadFromJson(json model) {
            FilteringCore *ret = nullptr;

            try {
                // extracting elements
                int numRow = model["numRow"];
                int numColumn = model["numColumn"];
                double threshold = model["threshold"];
                unsigned long timestamp = model["timestamp"];
                double factor = model["factor"];

                std::vector<unsigned long> tempIndexEdge = model["indexEdge"];
                std::vector<unsigned long> tempIndexSource = model["indexSource"];
                std::vector<unsigned long> tempIndexDestination = model["indexDestination"];

                json numCurrentEdgeJson = model["numCurrentEdge"];
                json numTotalEdgeJson = model["numTotalEdge"];
                json scoreEdgeJson = model["scoreEdge"];
                json numCurrentSourceJson = model["numCurrentSource"];
                json numTotalSourceJson = model["numTotalSource"];
                json scoreSourceJson = model["scoreSource"];
                json numCurrentDestinationJson = model["numCurrentDestination"];
                json numTotalDestinationJson = model["numTotalDestination"];
                json scoreDestinationJson = model["scoreDestination"];

                double timestampReciprocal = model["timestampReciprocal"];
                std::vector<bool> tempShouldMerge = model["shouldMerge"];

                // verify number of elements
                if ((tempIndexEdge.size() == numRow) && (tempIndexSource.size() == numRow) &&
                    (tempIndexDestination.size() == numRow) && (tempShouldMerge.size() == (numRow * numColumn))) {

                    CountMinSketch *numCurrentEdge = CountMinSketch::LoadFromJson(numCurrentEdgeJson);
                    CountMinSketch *numTotalEdge = CountMinSketch::LoadFromJson(numTotalEdgeJson);
                    CountMinSketch *scoreEdge = CountMinSketch::LoadFromJson(scoreEdgeJson);
                    CountMinSketch *numCurrentSource = CountMinSketch::LoadFromJson(numCurrentSourceJson);
                    CountMinSketch *numTotalSource = CountMinSketch::LoadFromJson(numTotalSourceJson);
                    CountMinSketch *scoreSource = CountMinSketch::LoadFromJson(scoreSourceJson);
                    CountMinSketch *numCurrentDestination = CountMinSketch::LoadFromJson(numCurrentDestinationJson);
                    CountMinSketch *numTotalDestination = CountMinSketch::LoadFromJson(numTotalDestinationJson);
                    CountMinSketch *scoreDestination = CountMinSketch::LoadFromJson(scoreDestinationJson);

                    if (
                            numCurrentEdge != nullptr
                            && numTotalEdge != nullptr
                            && scoreEdge != nullptr
                            && numCurrentSource != nullptr
                            && numTotalSource != nullptr
                            && scoreSource != nullptr
                            && numCurrentDestination != nullptr
                            && numTotalDestination != nullptr
                            && scoreDestination != nullptr
                            ) {

                        ret = new FilteringCore(numRow, numColumn, threshold, timestamp, factor, tempIndexEdge,
                                                tempIndexSource, tempIndexDestination, numCurrentEdge, numTotalEdge,
                                                scoreEdge, numCurrentSource, numTotalSource, scoreSource,
                                                numCurrentDestination, numTotalDestination, scoreDestination,
                                                timestampReciprocal, tempShouldMerge);
                    }

                    delete numCurrentEdge;
                    delete numTotalEdge;
                    delete scoreEdge;
                    delete numCurrentSource;
                    delete numTotalSource;
                    delete scoreSource;
                    delete numCurrentDestination;
                    delete numTotalDestination;
                    delete scoreDestination;
                }
            }
            catch (std::exception &e) {
                std::cout << e.what() << std::endl;
            }
            catch (...) {}

            return ret;

        }

        static FilteringCore *LoadFromFile(const std::string &path) {
            std::ifstream in(path);
            FilteringCore *ret = nullptr;

            try {
                json model = json::parse(in);
                ret = FilteringCore::LoadFromJson(model);
            }
            catch (std::exception &e) {
                std::cout << e.what() << std::endl;
            }
            catch (...) {}

            return ret;
        }
    };
}
