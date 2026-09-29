#pragma once

#include <span>
#include <utility>

#include <nanobind/nanobind.h>

namespace py = nanobind;

class PyBufferView {
	Py_buffer view{};
	bool acquired{false};

public:
	explicit PyBufferView(py::handle obj) {
		if (!obj.ptr() || PyObject_GetBuffer(obj.ptr(), &view, PyBUF_SIMPLE) != 0) {
			if (!PyErr_Occurred()) {
				PyErr_SetString(PyExc_TypeError, "expected a contiguous bytes-like object");
			}
			throw py::python_error();
		}
		acquired = true;
	}

	PyBufferView(const PyBufferView&) = delete;
	PyBufferView& operator=(const PyBufferView&) = delete;

	~PyBufferView() {
		if (acquired) {
			PyBuffer_Release(&view);
		}
	}

	[[nodiscard]] std::span<const std::byte> span() const {
		return {static_cast<const std::byte*>(view.buf), static_cast<std::size_t>(view.len)};
	}
};

template<typename Fn>
decltype(auto) call_nogil(Fn&& fn) {
	py::gil_scoped_release release;
	return std::forward<Fn>(fn)();
}

template<typename T>
py::bytes py_bytes_from(const T& d) {
	return py::bytes{d.data(), d.size()};
}
