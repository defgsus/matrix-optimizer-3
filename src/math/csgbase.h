#ifndef MOSRC_MATH_CSGBASE_H
#define MOSRC_MATH_CSGBASE_H

#include <QString>
#include <QList>

#include "types/vector.h"

namespace MO {


#define MO_CSG_CONSTRUCTOR(Name__, type__)                                                          \
    Name__();                                                                                       \
    virtual Name__ * cloneClass() const override { return new Name__(); }                           \
    virtual const QString& className() const override { static QString s(#Name__); return s; }      \
    virtual QString getGlsl() const override;                                                       \
    virtual Type type() const override { return type__; }

#define MO_REGISTER_CSG(Name__) \
    namespace { static bool reg##Name__##__ = ::MO::CsgBase::registerCsgClass_(new Name__()); }


class Properties;

/** Base class for Csg object (constructive solid geometry) */
class CsgBase
{
public:
    // ------- types --------

    enum Type
    {
        T_ROOT,
        T_SOLID,
        T_COMBINER
    };


    // -------- ctor --------

    CsgBase();
    virtual ~CsgBase();

    // ------- getter -------

    /** Read access to properties */
    const Properties& properties() const;

    /** Returns the className() or the "name" field from properties, if set */
    QString name() const;

    /** Returns parent node or NULL */
    CsgBase * parent() const;

    /** Number of child nodes */
    size_t numChildren() const;

    /** Read access to child nodes */
    const QList<CsgBase*>& children() const;

    /** Returns a depth-first list of the whole tree */
    QList<CsgBase*> serialized() const;

    /** Constructs a complete function declaration if
        getGlslFunctionBody() returns a string,
        otherwise an empty string. */
    QString glslFunctionDeclaration() const;

    /** Returns the getGlsl() string, or a call to the
        function if getGlslFunctionBody() returns a string. */
    QString glslCall() const;

    // ------- setter -------

    /** Replace properties */
    void setProperties(const Properties&);

    /** Sets the "name" property */
    void setName(const QString& s);

    /** Adds a child node, ownership is taken.
        If index < 0, the node is append,
        otherwise the node is inserted before the specified index. */
    void addChildren(CsgBase*, int index = -1);

    /** Delete all children nodes */
    void deleteChildren();

    /** Returns name() as comment, or empty string if disabled */
    QString glslComment() const;

    /** Returns the name of the function for this object.
        Regardless of if this object is a function or not. */
    QString glslFunctionName() const;

    // ---- virtual interface ----

    /** Must return a new instance of the derived class */
    virtual CsgBase * cloneClass() const = 0;
    /** Must return the name of the class */
    virtual const QString& className() const = 0;
    /** Returns the type of the derived object */
    virtual Type type() const = 0;
    /** Override to allow childrens */
    virtual bool canHaveChildren() const { return false; }

    /** Returns the glsl code for the distance function */
    virtual QString getGlsl() const = 0;
    /** If this returns a non-empty string, the glsl code will be wrapped in a function.
        The function should work on existing variables 'float d' and 'vec3 pos'. */
    virtual QString getGlslFunctionBody() const { return QString(); }

    // ---- static interface -----

    /** Returns a new instance for the given className(), or NULL if unknown.
        Ownership is with caller. */
    static CsgBase * createClass(const QString& className);

    /** Helper to register the className() */
    static bool registerCsgClass_(CsgBase*);

    /** Converts a number to a 'x.y' string, always including the '.' */
    static QString toGlsl(double v);

    /** Converts a vector to a 'vec3(x,y,z)' string */
    static QString toGlsl(const Vec3& v);

protected:

    /** Read access to properties */
    const Properties& props() const;
    /** Write access to properties */
    Properties& props();

private:
    struct PrivateCB;
    PrivateCB * p_cb_;
};

class CsgRoot : public CsgBase
{
public:
    MO_CSG_CONSTRUCTOR(CsgRoot, T_ROOT)
    bool canHaveChildren() const override { return true; }

    /** Returns the full code for the glsl distance function */
    QString toGlsl(const QString& dist_func_name = QString("scene_d")) const;

};


class CsgPositionBase : public CsgBase
{
public:
    CsgPositionBase();

    Vec3 position() const;

    QString positionGlsl() const;
};


} // namespace MO

#endif // MOSRC_MATH_CSGBASE_H
