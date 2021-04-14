//
// Created by Mun Hou on 2021/2/19.
//
// A simple wrapper for MIDAS

#include <pybind11/pybind11.h>
#include <RelationalCore.hpp>
#include <FilteringCore.hpp>
#include <NormalCore.hpp>
#include <string>

namespace py = pybind11;

PYBIND11_MODULE(MIDAS, m) {
    m.doc() = "MIDAS wrapper";

    py::class_<MIDAS::NormalCore>(m, "MIDAS")
            .def(py::init<int, int>(), py::arg("num_row"), py::arg("num_col"))
            .def("add_edge", static_cast<double (MIDAS::NormalCore::*)(unsigned long, unsigned long,
                                                                       unsigned long)>(&MIDAS::NormalCore::operator()),
                 py::arg("source"), py::arg("destination"),
                 py::arg("timestamp"))
            .def("add_edge", static_cast<double (MIDAS::NormalCore::*)(const std::string &, const std::string &,
                                                                       unsigned long)>(&MIDAS::NormalCore::operator()),
                 py::arg("source"), py::arg("destination"),
                 py::arg("timestamp"))
            .def("dump", &MIDAS::NormalCore::DumpToFile, py::arg("path"))
            .def_static("load", &MIDAS::NormalCore::LoadFromFile, py::return_value_policy::copy, py::arg("path"));

    py::class_<MIDAS::RelationalCore>(m, "MIDASR")
            .def(py::init<int, int, double>(), py::arg("num_row"), py::arg("num_col"), py::arg("factor") = 0.5)
            .def("add_edge", static_cast<double (MIDAS::RelationalCore::*)(unsigned long, unsigned long,
                                                                           unsigned long)>(&MIDAS::RelationalCore::operator()),
                 py::arg("source"), py::arg("destination"),
                 py::arg("timestamp"))
            .def("add_edge", static_cast<double (MIDAS::RelationalCore::*)(const std::string &, const std::string &,
                                                                           unsigned long)>(&MIDAS::RelationalCore::operator()),
                 py::arg("source"), py::arg("destination"),
                 py::arg("timestamp"))
            .def("dump", &MIDAS::RelationalCore::DumpToFile, py::arg("path"))
            .def_static("load", &MIDAS::RelationalCore::LoadFromFile, py::return_value_policy::copy, py::arg("path"));

    py::class_<MIDAS::FilteringCore>(m, "MIDASF")
            .def(py::init<int, int, double, double>(), py::arg("num_row"), py::arg("num_col"), py::arg("threshold"),
                 py::arg("factor") = 0.5)
            .def("add_edge", static_cast<double (MIDAS::FilteringCore::*)(unsigned long, unsigned long,
                                                                          unsigned long)>(&MIDAS::FilteringCore::operator()),
                 py::arg("source"), py::arg("destination"),
                 py::arg("timestamp"))
            .def("add_edge", static_cast<double (MIDAS::FilteringCore::*)(const std::string &, const std::string &,
                                                                          unsigned long)>(&MIDAS::FilteringCore::operator()),
                 py::arg("source"), py::arg("destination"),
                 py::arg("timestamp"))
            .def("dump", &MIDAS::FilteringCore::DumpToFile, py::arg("path"))
            .def_static("load", &MIDAS::FilteringCore::LoadFromFile, py::return_value_policy::copy, py::arg("path"));
}