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
    struct RelationalCore {
        const int numRow, numColumn;
        unsigned long timestamp = 1;
        const double factor;
        unsigned long *const indexEdge; // Pre-compute the index to-be-modified, thanks to the same structure of CMSs
        unsigned long *const indexSource;
        unsigned long *const indexDestination;
        CountMinSketch numCurrentEdge, numTotalEdge;
        CountMinSketch numCurrentSource, numTotalSource;
        CountMinSketch numCurrentDestination, numTotalDestination;

        RelationalCore(int numRow, int numColumn, double factor = 0.5) :
                numRow(numRow),
                numColumn(numColumn),
                factor(factor),
                indexEdge(new unsigned long[numRow]),
                indexSource(new unsigned long[numRow]),
                indexDestination(new unsigned long[numRow]),
                numCurrentEdge(numRow, numColumn),
                numTotalEdge(numCurrentEdge),
                numCurrentSource(numRow, numColumn),
                numTotalSource(numCurrentSource),
                numCurrentDestination(numRow, numColumn),
                numTotalDestination(numCurrentDestination) {}

        RelationalCore(int numRow, int numColumn, unsigned long timestamp, double factor,
                       std::vector<unsigned long> indexEdge, std::vector<unsigned long> indexSource,
                       std::vector<unsigned long> indexDestination, CountMinSketch *numCurrentEdge,
                       CountMinSketch *numTotalEdge, CountMinSketch *numCurrentSource, CountMinSketch *numTotalSource,
                       CountMinSketch *numCurrentDestination, CountMinSketch *numTotalDestination) :
                numRow(numRow),
                numColumn(numColumn),
                factor(factor),
                timestamp(timestamp),
                indexEdge(new unsigned long[numRow]),
                indexSource(new unsigned long[numRow]),
                indexDestination(new unsigned long[numRow]),
                numCurrentEdge(*numCurrentEdge),
                numTotalEdge(*numTotalEdge),
                numCurrentSource(*numCurrentSource),
                numTotalSource(*numTotalSource),
                numCurrentDestination(*numCurrentDestination),
                numTotalDestination(*numTotalDestination) {
            std::copy(indexSource.begin(), indexSource.end(), this->indexSource);
            std::copy(indexEdge.begin(), indexEdge.end(), this->indexEdge);
            std::copy(indexDestination.begin(), indexDestination.end(), this->indexDestination);
        }

        virtual ~RelationalCore() {
            delete[] indexEdge;
            delete[] indexSource;
            delete[] indexDestination;
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
                                    ComputeScore(numCurrentDestination(indexDestination),
                                                 numTotalDestination(indexDestination), timestamp),
                            });
        }

        json SerializeAsJson() {
            json model = json();
            auto copyIndexEdge = json::array();
            auto copyIndexSource = json::array();
            auto copyIndexDestination = json::array();

            std::copy(indexEdge, indexEdge + numRow, std::back_inserter(copyIndexEdge));
            std::copy(indexSource, indexSource + numRow, std::back_inserter(copyIndexSource));
            std::copy(indexDestination, indexDestination + numRow, std::back_inserter(copyIndexDestination));

            model["numRow"] = numRow;
            model["numColumn"] = numColumn;
            model["timestamp"] = timestamp;
            model["factor"] = factor;
            model["indexEdge"] = copyIndexEdge;
            model["indexSource"] = copyIndexSource;
            model["indexDestination"] = copyIndexDestination;
            model["numCurrentEdge"] = numCurrentEdge.SerializeAsJson();
            model["numTotalEdge"] = numTotalEdge.SerializeAsJson();
            model["numCurrentSource"] = numCurrentSource.SerializeAsJson();
            model["numTotalSource"] = numTotalSource.SerializeAsJson();
            model["numCurrentDestination"] = numCurrentDestination.SerializeAsJson();
            model["numTotalDestination"] = numTotalDestination.SerializeAsJson();

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
    };

    RelationalCore *LoadRelationCoreFromJson(json model) {
        RelationalCore *ret = nullptr;

        try {
            // extracting elements
            int numRow = model["numRow"];
            int numColumn = model["numColumn"];
            double factor = model["factor"];
            unsigned long timestamp = model["timestamp"];

            std::vector<unsigned long> tempIndexEdge = model["indexEdge"];
            std::vector<unsigned long> tempIndexSource = model["indexSource"];
            std::vector<unsigned long> tempIndexDestination = model["indexDestination"];

            json numCurrentEdgeJson = model["numCurrentEdge"];
            json numTotalEdgeJson = model["numTotalEdge"];
            json numCurrentSourceJson = model["numCurrentSource"];
            json numTotalSourceJson = model["numTotalSource"];
            json numCurrentDestinationJson = model["numCurrentDestination"];
            json numTotalDestinationJson = model["numTotalDestination"];

            // verify number of elements
            if ((tempIndexEdge.size() == numRow) && (tempIndexSource.size() == numRow) &&
                (tempIndexDestination.size() == numRow)) {

                CountMinSketch *numCurrentEdge = MIDAS::LoadCountMinSketchFromJson(numCurrentEdgeJson);
                CountMinSketch *numTotalEdge = MIDAS::LoadCountMinSketchFromJson(numTotalEdgeJson);
                CountMinSketch *numCurrentSource = MIDAS::LoadCountMinSketchFromJson(numCurrentSourceJson);
                CountMinSketch *numTotalSource = MIDAS::LoadCountMinSketchFromJson(numTotalSourceJson);
                CountMinSketch *numCurrentDestination = MIDAS::LoadCountMinSketchFromJson(numCurrentDestinationJson);
                CountMinSketch *numTotalDestination = MIDAS::LoadCountMinSketchFromJson(numTotalDestinationJson);

                if (
                        numCurrentEdge != nullptr
                        && numTotalEdge != nullptr
                        && numCurrentSource != nullptr
                        && numTotalSource != nullptr
                        && numCurrentDestination != nullptr
                        && numTotalDestination != nullptr
                        ) {

                    ret = new RelationalCore(numRow, numColumn, timestamp, factor, tempIndexEdge, tempIndexSource,
                                             tempIndexDestination, numCurrentEdge, numTotalEdge, numCurrentSource,
                                             numTotalSource, numCurrentDestination, numTotalDestination);
                }

                delete numCurrentEdge;
                delete numTotalEdge;
                delete numCurrentSource;
                delete numTotalSource;
                delete numCurrentDestination;
                delete numTotalDestination;
            }
        }
        catch (std::exception &e) {
            std::cout << e.what() << std::endl;
        }
        catch (...) {}

        return ret;

    }

    RelationalCore *LoadRelationalCoreFromFile(const std::string &path) {
        std::ifstream in(path);
        RelationalCore *ret = nullptr;

        try {
            json model = json::parse(in);
            ret = LoadRelationCoreFromJson(model);
        }
        catch (std::exception &e) {
            std::cout << e.what() << std::endl;
        }
        catch (...) {}

        return ret;
    }
}
