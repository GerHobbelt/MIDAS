//
// Created by Mun Hou on 2021/2/19.
//
// A simple wrapper for MIDAS

#include <pybind11/pybind11.h>
#include <RelationalCore.hpp>
#include <FilteringCore.hpp>
#include <NormalCore.hpp>

namespace py = pybind11;

PYBIND11_MODULE(MIDAS, m) {
    m.doc() = "MIDAS wrapper";

    py::class_<MIDAS::NormalCore>(m, "MIDAS")
            .def(py::init<int, int>(), py::arg("num_row"), py::arg("num_col"))
            .def("add_edge", static_cast<double (MIDAS::NormalCore::*)(unsigned long, unsigned long,
                                                                      unsigned long)>(&MIDAS::NormalCore::operator()),
                 py::arg("source"), py::arg("destination"),
                 py::arg("timestamp"))
            .def("add_edge", static_cast<double (MIDAS::NormalCore::*)(const char *, const char *,
                                                                      unsigned long)>(&MIDAS::NormalCore::operator()),
                 py::arg("source"), py::arg("destination"),
                 py::arg("timestamp"));

    py::class_<MIDAS::RelationalCore>(m, "MIDASR")
            .def(py::init<int, int, double>(), py::arg("num_row"), py::arg("num_col"), py::arg("factor") = 0.5)
            .def("add_edge", static_cast<double (MIDAS::RelationalCore::*)(unsigned long, unsigned long,
                                                                       unsigned long)>(&MIDAS::RelationalCore::operator()),
                 py::arg("source"), py::arg("destination"),
                 py::arg("timestamp"))
            .def("add_edge", static_cast<double (MIDAS::RelationalCore::*)(const char *, const char *,
                                                                       unsigned long)>(&MIDAS::RelationalCore::operator()),
                 py::arg("source"), py::arg("destination"),
                 py::arg("timestamp"));

    py::class_<MIDAS::FilteringCore>(m, "MIDASF")
            .def("add_edge", static_cast<double (MIDAS::FilteringCore::*)(unsigned long, unsigned long,
                                                                       unsigned long)>(&MIDAS::FilteringCore::operator()),
                 py::arg("source"), py::arg("destination"),
                 py::arg("timestamp"))
            .def("add_edge", static_cast<double (MIDAS::FilteringCore::*)(const char *, const char *,
                                                                       unsigned long)>(&MIDAS::FilteringCore::operator()),
                 py::arg("source"), py::arg("destination"),
                 py::arg("timestamp"));
}