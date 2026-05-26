#include "EventMonitor.h"
#include "FileMonitor.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

using LogMonitor = O3::EventMonitor<O3::FileMonitor>;

namespace py = pybind11;

PYBIND11_MODULE(_pylogmon, m, py::mod_gil_not_used(), py::multiple_interpreters::per_interpreter_gil()) {
    py::class_<LogMonitor>(m, "LogMonitor")
        .def(py::init<const std::vector<std::string>&>())
        .def_readwrite("callback", &LogMonitor::m_callback);
}

