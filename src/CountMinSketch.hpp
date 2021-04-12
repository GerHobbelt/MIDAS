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

#include <algorithm>
#include <iterator>
#include <string>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>


using json = nlohmann::json;
namespace MIDAS {
    struct CountMinSketch {
        // Fields
        // --------------------------------------------------------------------------------

        const int r, c, m = 104729; // Yes, a magic number, I just pick a random prime
        const int lenData;
        int *const param1;
        int *const param2;
        double *const data;
        constexpr static double infinity = std::numeric_limits<double>::infinity();

        // Methods
        // --------------------------------------------------------------------------------

        CountMinSketch() = delete;

        CountMinSketch &operator=(const CountMinSketch &b) = delete;

        CountMinSketch(int numRow, int numColumn) :
                r(numRow),
                c(numColumn),
                lenData(r * c),
                param1(new int[r]),
                param2(new int[r]),
                data(new double[lenData]) {
            for (int i = 0; i < r; i++) {
                param1[i] = rand() + 1; // ×0 is not a good idea, see Hash()
                param2[i] = rand();
            }
            std::fill(data, data + lenData, 0);
        }

        CountMinSketch(const CountMinSketch &b) :
                r(b.r),
                c(b.c),
                lenData(b.lenData),
                param1(new int[r]),
                param2(new int[r]),
                data(new double[lenData]) {
            std::copy(b.param1, b.param1 + r, param1);
            std::copy(b.param2, b.param2 + r, param2);
            std::copy(b.data, b.data + lenData, data);
        }

        CountMinSketch(int numRow, int numColumn, int *param1, int *param2, double *data) :
                r(numRow),
                c(numColumn),
                lenData(r * c),
                param1(param1),
                param2(param2),
                data(data) {}

        ~CountMinSketch() {
            delete[] param1;
            delete[] param2;
            delete[] data;
        }

        void ClearAll(double with = 0) const {
            std::fill(data, data + lenData, with);
        }

        void MultiplyAll(double by) const {
            for (int i = 0, I = lenData; i < I; i++) // Vectorization
                data[i] *= by;
        }

        void Hash(unsigned long *indexOut, unsigned long a, unsigned long b = 0) const {
            for (int i = 0; i < r; i++) {
                indexOut[i] = ((a + m * b) * param1[i] + param2[i]) % c;
                indexOut[i] += i * c + (indexOut[i] < 0 ? c : 0);
            }
        }

        double operator()(const unsigned long *index) const {
            double least = infinity;
            for (int i = 0; i < r; i++)
                least = std::min(least, data[index[i]]);
            return least;
        }

        double Assign(const unsigned long *index, double with) const {
            for (int i = 0; i < r; i++)
                data[index[i]] = with;
            return with;
        }

        void Add(const unsigned long *index, double by = 1) const {
            for (int i = 0; i < r; i++)
                data[index[i]] += by;
        }

        json SerializeAsJson() {
            json model = json();
            auto copyParam1 = json::array();
            auto copyParam2 = json::array();
            auto copyData = json::array();

            std::copy(param1, param1 + r, std::back_inserter(copyParam1));
            std::copy(param2, param2 + r, std::back_inserter(copyParam2));
            std::copy(data, data + r, std::back_inserter(copyData));

            model["r"] = r;
            model["c"] = c;
            model["param1"] = copyParam1;
            model["param2"] = copyParam2;
            model["data"] = copyData;

            return model;
        }

        int DumpToFile(std::string path) {
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

    CountMinSketch *LoadCountMinSketchFromJson(json model) {
        CountMinSketch *ret = nullptr;

        try {
            // extracting elements
            int r = model["r"];
            int c = model["c"];

            std::vector<int> tempParam1 = model["param1"];
            std::vector<int> tempParam2 = model["param2"];
            std::vector<double> tempData = model["data"];

            // verify number of elements
            if ((tempData.size() == r) && (tempParam1.size() == r) && (tempParam2.size() == r)) {
                auto *param1 = new int[r];
                auto *param2 = new int[r];
                auto *data = new double[r];


                std::copy(tempParam1.begin(), tempParam1.end(), param1);
                std::copy(tempParam2.begin(), tempParam2.end(), param2);
                std::copy(tempData.begin(), tempData.end(), data);

                ret = new CountMinSketch(r, c, param1, param2, data);
            }
        }
        catch (std::exception &e) {
            std::cout << e.what() << std::endl;
        }
        catch (...) {}

        return ret;

    }

    CountMinSketch *LoadCountMinSketchFromFile(std::string path) {
        std::ifstream in(path);
        CountMinSketch *ret = nullptr;

        try {
            json model = json::parse(in);
            ret = LoadCountMinSketchFromJson(model);
        }
        catch (std::exception &e) {
            std::cout << e.what() << std::endl;
        }
        catch (...) {}

        return ret;
    }
}
