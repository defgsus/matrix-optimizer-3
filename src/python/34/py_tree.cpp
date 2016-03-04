/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/4/2016</p>
*/

#ifdef MO_ENABLE_PYTHON34

#include "py_utils.h"
#include "py_tree.h"
#include "io/log.h"

namespace MO {
namespace PYTHON34 {

PyObjectTreeWrapper::PyObjectTreeWrapper(PyObject* obj)
    : object(obj)
{
    Py_XINCREF(object);
    typeName = object ? PYTHON34::typeName(object) : "NULL";
    treeName = typeName;
}

PyObjectTreeWrapper::~PyObjectTreeWrapper()
{
    Py_XDECREF(object);
}

namespace {

    bool treeContains(PyObjectTree* node, PyObject* obj)
    {
        if (node->object()->object == obj)
            return true;
        node = node->root();
        return node->find(true, [=](PyObjectTreeWrapper*w){ return w->object == obj; });
    }

    PyObjectTree* newWrapper(PyObject* obj)
    {
        auto wrapper = new PyObjectTreeWrapper(obj);
        return new PyObjectTree(wrapper);
    }

    void fromDict(PyObjectTree* node)
    {
        PyObject *key, *value;
        Py_ssize_t pos = 0;

        while (PyDict_Next(node->object()->object, &pos, &key, &value))
        {
            if (treeContains(node, value))
                continue;
            auto childNode = createPyObjectTree(value);
            childNode->object()->treeName = "key[" + toString(key) + "]";
            node->append( childNode );
        }
    }

    void fromModule(PyObjectTree* node)
    {
        auto dict = PyModule_GetDict(node->object()->object);
        if (dict)
        {
            if (treeContains(node, dict))
                return;

            auto dictNode = newWrapper(dict);
            dictNode->object()->treeName = "module-dict";
            node->append( dictNode );
            fromDict(dictNode);
        }
    }

    void fromSequence(PyObjectTree* node)
    {
        Py_ssize_t num = PySequence_Size(node->object()->object);

        for (Py_ssize_t i=0; i<num; ++i)
        {
            auto item = PySequence_GetItem(node->object()->object, i);
            if (!treeContains(node, item))
                node->append( createPyObjectTree(item) );
        }
    }

} // namespace

PyObjectTree* createPyObjectTree(PyObject* root)
{
    MO_PRINT("CREATE " << typeName(root));
    auto node = newWrapper(root);

    if (root == nullptr)
        return node;

    if (PyModule_Check(root))
        fromModule(node);
    else if (PyDict_Check(root))
        fromDict(node);
    else if (PySequence_Check(root))
        fromSequence(node);

    return node;
}

} // namespace PYTHON34
} // namespace MO


#endif // MO_ENABLE_PYTHON34

