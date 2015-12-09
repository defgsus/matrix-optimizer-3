#include <QMap>
#include <QObject> // for tr()

#include "csgbase.h"
#include "types/properties.h"
#include "tool/deleter.h"
#include "io/xmlstream.h"
#include "io/error.h"
#include "io/log.h"


#if 0
#   include <QDebug>
#   define MO_DEBUG_CSG(arg__) qInfo() << "CsgBase(" << this << ")::" << arg__;
#   define MO_DEBUG_CSGP(arg__) qInfo() << "CsgBase(" << p << ")::Private::" << arg__;
#else
#   define MO_DEBUG_CSG(unused__)
#   define MO_DEBUG_CSGP(unused__)
#endif

namespace MO {

QString CsgBase::typeName(Type t)
{
    switch (t)
    {
        default: return QObject::tr("*unknown*");
        case T_ROOT: return QObject::tr("root");
        case T_SOLID: return QObject::tr("solid");
        case T_COMBINE: return QObject::tr("combination");
        case T_DEFORM: return QObject::tr("deformation");
    }
}

struct CsgBase::PrivateCB
{
    PrivateCB(CsgBase*p)
        : p         (p)
        , parent    (0)
    {

    }

    void setParentSafe(CsgBase * newParent);
    void serialize(QList<CsgBase*>& list);

    CsgBase * p;
    Properties props;
    QList<CsgBase*> childs;
    CsgBase * parent;

    static QMap<QString, CsgBase*>* regMap;
};
QMap<QString, CsgBase*>* CsgBase::PrivateCB::regMap = 0;

CsgBase::CsgBase()
    : p_cb_     (new PrivateCB(this))
{
    MO_DEBUG_CSG("CsgBase()");
}

CsgBase::~CsgBase()
{
    MO_DEBUG_CSG("~CsgBase()");

    deleteChildren();
    delete p_cb_;
}

CsgBase * CsgBase::parent() const
{
    return p_cb_->parent;
}

QString CsgBase::name() const
{
    const QString n = props().get("name").toString();
    return n.isEmpty() ? className().mid(3) : n;
}

const Properties& CsgBase::properties() const
{
    return p_cb_->props;
}

const Properties& CsgBase::props() const
{
    return p_cb_->props;
}

Properties& CsgBase::props()
{
    return p_cb_->props;
}

size_t CsgBase::numChildren() const
{
    return p_cb_->childs.size();
}

const QList<CsgBase*>& CsgBase::children() const
{
    return p_cb_->childs;
}

int CsgBase::indexInParent() const
{
    // Note: QList::indexOf() returns -1 as well
    // so it matches this interface,
    // in the unlikely case that parent does not contain *this
    return p_cb_->parent
            ? p_cb_->parent->p_cb_->childs.indexOf(const_cast<CsgBase*>(this))
            : -1;
}

QList<CsgBase*> CsgBase::serialized() const
{
    QList<CsgBase*> list;

    p_cb_->serialize(list);

    return list;
}

void CsgBase::PrivateCB::serialize(QList<CsgBase*>& list)
{
    list << p;
    for (auto c : p->children())
        c->p_cb_->serialize(list);
}


void CsgBase::setProperties(const Properties& p)
{
    p_cb_->props = p;
}

void CsgBase::setName(const QString &s)
{
    p_cb_->props.set("name", s);
}

void CsgBase::addChildren(CsgBase* o, int index)
{
    MO_DEBUG_CSG("addChildren(" << o << ", " << index << ")");

    if (index < 0)
        p_cb_->childs.append(o);
    else
        p_cb_->childs.insert(index, o);
    // set this as parent for new child
    o->p_cb_->setParentSafe(this);
}

void CsgBase::PrivateCB::setParentSafe(CsgBase *newParent)
{
    MO_DEBUG_CSGP("setParentSafe(" << newParent << ")");

    // remove from previous parent
    if (parent)
        parent->p_cb_->childs.removeOne(p);
    parent = newParent;
}

void CsgBase::deleteChildren()
{
    for (auto c : p_cb_->childs)
        delete c;
    p_cb_->childs.clear();
}

bool CsgBase::canBeReplacedBy(const CsgBase *other) const
{
    // both have unlimited children?
    if (canHaveNumChildren() < 0)
        return other->canHaveNumChildren() < 0;
    // other should have at least this possible children
    return canHaveNumChildren() <= other->canHaveNumChildren();
}

CsgBase* CsgBase::replace(CsgBase *node, const QString &className)
{
    if (!node || !node->parent())
        return 0;

    // find class for className
    auto i = PrivateCB::regMap->find(className);
    if (i == PrivateCB::regMap->end())
        return 0;

    // check compatibility
    if (!node->canBeReplacedBy(i.value()))
        return 0;

    // find index in parent's child list
    auto parent = node->parent();
    int idx = parent->p_cb_->childs.indexOf(node);
    if (idx < 0)
        return 0;

    // create replacement node
    auto newNode = i.value()->cloneClass();
    // set new parent
    newNode->p_cb_->setParentSafe(parent);
    // add childs
    for (auto n : node->children())
        newNode->addChildren(n);

    // insert in list
    parent->p_cb_->childs.replace(idx, newNode);

    // copy matching properties
    // XXX

    delete node;

    return newNode;
}

CsgBase* CsgBase::contain(CsgBase *node, const QString &className)
{
    if (!node || node->type() == T_ROOT)
        return 0;

    // find class for className
    auto i = PrivateCB::regMap->find(className);
    if (i == PrivateCB::regMap->end())
        return 0;

    // check compatibility
    if (!i.value()->canHaveChildren())
        return 0;

    // create new parent node
    auto newNode = i.value()->cloneClass();

    // install new node
    int idx = -1;
    if (node->parent())
    {
        idx = node->indexInParent();
        node->parent()->addChildren(newNode, idx);
    }
    // add contained node
    newNode->addChildren(node);

    return newNode;
}

void CsgBase::deleteNode(CsgBase* node)
{
    if (node->parent())
        node->parent()->p_cb_->childs.removeOne(node);

    delete node;
}

CsgBase * CsgBase::createClass(const QString& className)
{
    if (!PrivateCB::regMap
            || !PrivateCB::regMap->contains(className))
        return nullptr;

    return PrivateCB::regMap->value(className)->cloneClass();
}

bool CsgBase::registerCsgClass_(CsgBase* o)
{
    if (!PrivateCB::regMap)
        PrivateCB::regMap = new QMap<QString, CsgBase*>;

    if (PrivateCB::regMap->contains(o->className()))
    {
        MO_WARNING("Duplicate CSG class name '" << o->className() << "'");
        return false;
    }

    PrivateCB::regMap->insert(o->className(), o);
    return true;
}

QList<const CsgBase*> CsgBase::registeredClasses()
{
    QList<const CsgBase*> list;
    for (auto i : *PrivateCB::regMap)
        list << i;
    return list;
}



QString CsgBase::glslComment() const
{
    return QString("/* %1 */ ").arg(name());
}

QString CsgBase::toGlsl(double v)
{
    QString s = QString::number(v);
    if (!s.contains("."))
        s += ".";
    return s;
}

QString CsgBase::toGlsl(const Vec3& v)
{
    return QString("vec3(%1, %2, %3)")
            .arg(toGlsl(v.x)).arg(toGlsl(v.y)).arg(toGlsl(v.z));
}

QString CsgBase::glslFunctionName() const
{
    return QString("%1_0x%2").arg(className()).arg((size_t)this);
}

QString CsgBase::glslCall() const
{
    if (getGlslFunctionBody().isEmpty())
    {
        return getGlsl();
        //auto call = getGlsl();
        //return call.isEmpty() ? "MAX_DIST" : call;
    }
    return QString("%1(pos)").arg(glslFunctionName());
}

namespace {

    QString trailingNl(const QString& s)
    {
        return s.endsWith("\n") ? s : s + "\n";
    }

    // XXX Doesn't work for loops and stuff
    QString leadingTab(const QString& s)
    {
        QString r;
        for (int i=0; i<s.size(); ++i)
        {
            if (i == 0 && s[i] != '\t')
                r += '\t';
            r += s[i];
            if (s[i] == '\n'
                && (i+1) < s.size()
                && s[i+1] != '\t')
                r += '\t';
        }
        return r;
    }
}

QString CsgBase::glslFunctionDeclaration() const
{
    const QString body = getGlslFunctionBody();
    if (body.isEmpty())
        return body;
    const QString comment = glslComment();

    if (!body.contains("return"))
    {
        return QString("%1float %2(in vec3 pos)\n{\n"
                       "%3"
                       "%4"
                       "\treturn d;\n}\n")
                         .arg(comment.isEmpty() ? comment : comment + "\n")
                         .arg(glslFunctionName())
                         .arg(body.contains("float d") ? "" : "\tfloat d = MAX_DIST;\n")
                         .arg(trailingNl(leadingTab(body)));
    }
    else
    {
        return QString("%1float %2(in vec3 pos)\n{\n"
                       "%3}\n")
                         .arg(comment.isEmpty() ? comment : comment + "\n")
                         .arg(glslFunctionName())
                         .arg(trailingNl(leadingTab(body)));
    }
}


void CsgBase::serialize(IO::XmlStream &io) const
{
    io.createSection("csg-node");

        // info per node
        io.write("version", 1);
        io.write("class", className());
        // and it's properties
        props().serialize(io);

        // all children
        for (auto n : children())
            n->serialize(io);

    io.endSection();
}

CsgBase* CsgBase::deserialize(IO::XmlStream &io)
{
    io.verifySection("csg-node");

        /*int ver = */io.expectInt("version");
        QString cname = io.expectString("class");

        // create node
        auto node = cname == CsgRoot::staticClassName()
                ? new CsgRoot()
                : createClass(cname);
        if (!node)
            MO_IO_ERROR(VERSION_MISMATCH, "Unknown class '" << cname << "' in stream");
        ScopedDeleter<CsgBase> auto_remove(node);

        while (io.nextSubSection())
        {
            // read properties
            if (io.section() == "properties")
                node->p_cb_->props.deserialize(io);
            else
            // read sub-nodes
            if (io.section() == "csg-node")
            {
                auto cnode = deserialize(io);
                node->addChildren(cnode);
            }

            io.leaveSection();
        }

        auto_remove.detach();
        return node;
}


void CsgBase::saveXml(const QString &fn) const
{
    IO::XmlStream xml;
    xml.startWriting("csg");
    serialize(xml);
    xml.stopWriting();
    xml.save(fn);
}

CsgBase* CsgBase::loadXml(const QString &fn)
{
    CsgBase * node = 0;

    IO::XmlStream xml;
    xml.load(fn);
    xml.startReading("csg");
    while (xml.nextSubSection())
    {
        if (xml.section() == "csg-node")
        {
            node = deserialize(xml);
            break;
        }
    }
    xml.stopReading();

    return node;
}









// #################################### CsgRoot #############################

CsgRoot::CsgRoot()
    : CsgBase()
{
}

QString CsgRoot::getGlsl() const { return QString(); }

QString CsgRoot::toGlsl(const QString &dist_func_name) const
{
    QString s;

    // -- function declarations --

    auto list = serialized();

    // class-specific functions
    QMap<QString, QString> gmap;
    for (auto n : list)
    {
        auto gf = n->globalFunctions();
        if (!gf.isEmpty() && !gmap.contains(n->className()))
            gmap.insert(n->className(), gf);
    }
    for (auto& i : gmap)
        s += i + "\n";

    // node-specific functions
    for (int i=0; i<list.size(); ++i)
    {
        auto func = list[list.size() - 1 - i]->glslFunctionDeclaration();
        if (!func.isEmpty())
            s += func + "\n";
    }

    // -- dist func body --

    s += QString("float %1(in vec3 pos)\n{\n\tfloat d = MAX_DIST;\n")
            .arg(dist_func_name);

    for (auto o : children())
    {
        auto call = o->glslCall();
        if (!call.isEmpty())
            s += QString("\td = min(d, %1);\n").arg(call);
    }

    s += "\treturn d;\n}\n";

    return s;
}




// ############################# CsgSignedBase #############################

CsgSignedBase::CsgSignedBase()
    : CsgBase()
{
    props().set("inverse", QObject::tr("inverse"), QObject::tr("Turn the inside outside"), false);
}

bool CsgSignedBase::isNegative() const
{
    return props().get("inverse").toBool();
}

QString CsgSignedBase::wrapInSign(const QString& code) const
{
    if (isNegative())
        return QString("-(%1)").arg(code);
    else
        return code;
}


// ############################# CsgPositionSignedBase #############################

CsgPositionSignedBase::CsgPositionSignedBase()
    : CsgSignedBase()
{
    props().set("x", QObject::tr("x"), QObject::tr("Position on X axis"), 0., 0.1);
    props().set("y", QObject::tr("y"), QObject::tr("Position on Y axis"), 0., 0.1);
    props().set("z", QObject::tr("z"), QObject::tr("Position on Z axis"), 0., 0.1);
}

Vec3 CsgPositionSignedBase::position() const
{
    return Vec3(props().get("x").toFloat(),
                props().get("y").toFloat(),
                props().get("z").toFloat());
}

QString CsgPositionSignedBase::positionGlsl() const
{
    QString s = "pos";

    const Vec3 p = position();
    if (p.x != 0 || p.y != 0 || p.z != 0)
        s += " - " + toGlsl(p);

    return s;
}



} // namespace MO
