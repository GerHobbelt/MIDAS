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
            .def(py::init<unsigned long, unsigned long>(), py::arg("num_row"), py::arg("num_col"))
            .def("add_edge", &MIDAS::NormalCore::operator(), py::arg("source"), py::arg("destination"),
                 py::arg("timestamp"));

    py::class_<MIDAS::RelationalCore>(m, "MIDASR")
            .def(py::init<unsigned long, unsigned long, float>(), py::arg("num_row"), py::arg("num_col"), py::arg("factor") = 0.5)
            .def("add_edge", &MIDAS::RelationalCore::operator(), py::arg("source"), py::arg("destination"),
                 py::arg("timestamp"));

    py::class_<MIDAS::FilteringCore>(m, "MIDASF")
            .def(py::init<unsigned long, unsigned long, double, double>(), py::arg("num_row"), py::arg("num_col"), py::arg("threshold"),
                 py::arg("factor") = 0.5)
            .def("add_edge", &MIDAS::FilteringCore::operator(), py::arg("source"), py::arg("destination"),
                 py::arg("timestamp"));
}