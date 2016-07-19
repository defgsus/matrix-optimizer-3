#ifndef MOSRC_MATH_CSGBASE_H
#define MOSRC_MATH_CSGBASE_H

#include <QString>
#include <QList>

#include "types/vector.h"

namespace MO {
class Properties;
namespace IO { class XmlStream; }

#define MO_CSG_CONSTRUCTOR(Name__, type__)                                              \
    Name__();                                                                           \
    virtual Name__ * cloneClass() const override { return new Name__(); }               \
    static const QString& staticClassName() { static QString s(#Name__); return s; }    \
    virtual const QString& className() const override { return staticClassName(); }     \
    virtual QString getGlsl() const override;                                           \
    virtual Type type() const override { return type__; }

#define MO_REGISTER_CSG(Name__) \
    namespace { static bool reg##Name__##__ = ::MO::CsgBase::registerCsgClass_(new Name__()); }


/** Base class for Csg object (constructive solid geometry) */
class CsgBase
{
public:
    // ------- types --------

    enum Type
    {
        T_ROOT,
        T_SOLID,
        T_COMBINE,
        T_DEFORM
    };

    static QString typeName(Type);

    // -------- ctor --------

    CsgBase();
    virtual ~CsgBase();

    // ------- io -----------

    /** Writes this node and all of it's children.
        Creates section "csg-node".
        @throws IoException */
    void serialize(IO::XmlStream& io) const;

    /** Reads a node tree from stream.
        Expects section "csg-node".
        @throws IoException */
    static CsgBase* deserialize(IO::XmlStream& io);

    void saveXml(const QString& fn) const;
    static CsgBase * loadXml(const QString& fn);

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

    /** Returns the index of this node in it's parent's child list,
        or -1 */
    int indexInParent() const;

    /** Returns true if the node can have at least one children */
    bool canHaveChildren() const { return canHaveNumChildren() != 0; }

    /** Returns true when this node can be replaced by @p other node */
    bool canBeReplacedBy(const CsgBase* other) const;

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
    /** Override to allow childrens, -1 for unlimited */
    virtual int canHaveNumChildren() const { return 0; }

    /** Returns the glsl code for the distance function */
    virtual QString getGlsl() const = 0;
    /** If this returns a non-empty string, the glsl code will be wrapped in a function.
        The function should work on existing variables 'float d' and 'vec3 pos'. */
    virtual QString getGlslFunctionBody() const { return QString(); }

    /** Override to provide some global helper functions.
        These will only be pasted once for every class. */
    virtual QString globalFunctions() const { return QString(); }

    // ---- static interface -----

    /** Returns a new instance for the given className(), or NULL if unknown.
        Ownership is with caller. */
    static CsgBase * createClass(const QString& className);

    /** Returns a list of all possible classes */
    static QList<const CsgBase*> registeredClasses();

    /** Helper to register the className() */
    static bool registerCsgClass_(CsgBase*);

    /** Replaces the @p node with a new class.
        Returns new node if the node could be replaced, NULL otherwise */
    static CsgBase* replace(CsgBase * node, const QString& className);
    /** Puts @p node into a new class.
        Returns the new node if possible, NULL otherwise */
    static CsgBase* contain(CsgBase * node, const QString& className);

    /** Deletes the node and removes it from the tree */
    static void deleteNode(CsgBase * node);

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

    /** disable copy */ CsgBase(const CsgBase&) = delete;
    /** disable copy */ void operator=(const CsgBase&) = delete;

    struct PrivateCB;
    PrivateCB * p_cb_;
};


class CsgRoot : public CsgBase
{
public:
    MO_CSG_CONSTRUCTOR(CsgRoot, T_ROOT)
    int canHaveNumChildren() const override { return -1; }

    /** Returns the full code for the glsl distance function */
    QString toGlsl(const QString& dist_func_name = QString("scene_d")) const;

};


class CsgSignedBase : public CsgBase
{
public:
    CsgSignedBase();

    bool isNegative() const;

    /** Puts a -( ) around the code if negative is selected */
    QString wrapInSign(const QString& code) const;
};


class CsgPositionSignedBase : public CsgSignedBase
{
public:
    CsgPositionSignedBase();

    Vec3 position() const;

    QString positionGlsl() const;
};

} // namespace MO

#endif // MOSRC_MATH_CSGBASE_H
