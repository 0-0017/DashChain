#include "Trainer.h"

Trainer::Trainer() {
    lastBlock = 0;
    totalSupply = 0;
    circSupply = 0;
    Balance = 0;
    periodVotes = 0;
    height = 0;
    txVolume = 0;
}

std::vector<double> Trainer::trainData(std::tuple<double, double, double, unsigned long, unsigned int, unsigned long> data){
    /* Initialize Python */
    Py_Initialize();

    /* Import Python Module */
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('../python')");
    PyObject* pName = PyUnicode_DecodeFSDefault("Trainer"); // name of file
    PyObject* pModule = PyImport_Import(pName);
    Py_DECREF(pName);

    if (pModule) {
        if (PyObject* pFunc = PyObject_GetAttrString(pModule, "load_data"); pFunc && PyCallable_Check(pFunc)) {

            // Create and populate Python dict
            PyObject* pDict = PyDict_New();
            const std::map<std::string, VariantType> cppMap = {
                {"total_supply", VariantType{std::get<0>(data)}}, {"circ_supply", VariantType{std::get<1>(data)}},
                {"balance", VariantType{std::get<2>(data)}}, {"period_votes", VariantType{std::get<3>(data)}},
                {"height", VariantType{std::get<4>(data)}}, {"tx_volume", VariantType{std::get<5>(data)}}
            };

            for (const auto&[fst, snd] : cppMap) {
                PyObject* key = PyUnicode_FromString(fst.c_str());

                PyObject* val = std::visit([]<typename T0>(T0&& arg) -> PyObject* {
                    using T = std::decay_t<T0>;
                    if constexpr (std::is_same_v<T, unsigned int> || std::is_same_v<T, size_t>) {
                        return PyLong_FromUnsignedLong(arg);
                    } else if constexpr (std::is_same_v<T, double>) {
                        return PyFloat_FromDouble(arg);
                    } else {
                        return Py_None; // fallback
                    }
                }, snd);

                PyDict_SetItem(pDict, key, val);
                Py_DECREF(key);
                Py_DECREF(val);  // only if val is a new reference (Py_None is borrowed)

            }

            // Call Python function with one argument (tuple)
            PyObject* args = PyTuple_Pack(1, pDict);
            PyObject* pResult = PyObject_CallObject(pFunc, args);
            Py_DECREF(args);
            Py_DECREF(pDict);

            // Handle result (list of tuples)
            if (pResult == Py_None) {
                std::vector<double> empty;
                return empty;
            } else if (PyList_Check(pResult) && PyList_Size(pResult) == 6) {
                std::vector<double> action;
                for (Py_ssize_t i = 0; i < 6; ++i) {
                    if (PyObject* item = PyList_GetItem(pResult, i); PyFloat_Check(item)) {
                        action.push_back(PyFloat_AsDouble(item));
                    } else if (PyLong_Check(item)) {
                        action.push_back(static_cast<double>(PyLong_AsLong(item)));
                    } else {
                        std::cerr << "Unexpected item in result list.\n";
                        break;
                    }
                }
                Py_DECREF(pResult);
                return action;
            } else {
                PyErr_Print();
            }

            Py_XDECREF(pFunc);
        } else {
            PyErr_Print();
        }

        Py_DECREF(pModule);
    } else {
        PyErr_Print();
    }

    Py_Finalize();
    std::vector<double> empty;
    return empty;
}