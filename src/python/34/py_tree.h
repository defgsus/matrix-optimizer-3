/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/4/2016</p>
*/

#ifdef MO_ENABLE_PYTHON34

#ifndef MOSRC_PYTHON_34_PY_TREE_H
#define MOSRC_PYTHON_34_PY_TREE_H

#include "py_utils.h"

#include <QString>

#include "graph/Tree.h"

namespace MO {
namespace PYTHON34 {

    struct PyObjectTreeWrapper
    {
        PyObjectTreeWrapper(PyObject*);
        ~PyObjectTreeWrapper();

        QString treeName, typeName;
        PyObject* object;
    };

    // traits specialization
    struct PyObjectTreeWrapperTraits
    {
        typedef TreeNode<PyObjectTreeWrapper*, PyObjectTreeWrapperTraits> Node;

        /** This is called when a node is created */
        static void creator(Node * node) { (void)node; }
        /** Called in destructor of node */
        static void destructor(Node * node) { delete node->object(); }

        /** Returns a string representation of each node */
        static QString toString(const Node * node)
            { return node->object() ? (node->object()->treeName
                                       + " (" + node->object()->typeName) : "NULL"; }
    };

    typedef TreeNode<PyObjectTreeWrapper*, PyObjectTreeWrapperTraits> PyObjectTree;

    PyObjectTree * createPyObjectTree(PyObject* root);

} // namespace PYTHON34

} // namespace MO


#endif // MOSRC_PYTHON_34_PY_TREE_H

#endif // MO_ENABLE_PYTHON34
