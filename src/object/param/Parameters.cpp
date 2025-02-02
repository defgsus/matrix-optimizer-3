/** @file parameters.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 02.11.2014</p>
*/

#include <QTextStream>

#include "Parameters.h"
#include "ParameterCallback.h"
#include "ParameterInt.h"
#include "ParameterImageList.h"
#include "ParameterFloat.h"
#include "ParameterFloatMatrix.h"
#include "ParameterFilename.h"
#include "ParameterFont.h"
#include "ParameterGeometry.h"
#include "ParameterSelect.h"
#include "ParameterTimeline1d.h"
#include "ParameterText.h"
#include "ParameterTexture.h"
#include "ParameterTransformation.h"
#include "object/Object.h"
#include "object/util/ObjectConnectionGraph.h"
#include "math/FloatMatrix.h"
#include "gl/opengl.h"
#include "io/DataStream.h"
#include "io/error.h"

namespace MO {

Parameters::Parameters(Object * o)
    : object_           (o)
    , isEvolve_         (true)
    , isEvolveGroup_    (false)
{
}

Parameters::~Parameters()
{
    for (auto p : parameters_)
        delete p;
}

void Parameters::copyFrom(const Parameters* other)
{
    for (auto po : other->parameters_)
    if (auto p = findParameter(po->idName()))
    {
        p->copyFrom(po);
    }
}


void Parameters::serialize(IO::DataStream & io) const
{
    // write parameters
    io.writeHeader("params", 1);

    // deterimine parameters to save
    qint32 count = 0;
    for (auto p : parameters_)
        if (!p->isZombie())
            ++count;

    io << count;

    for (auto p : parameters_)
    {
        if (p->isZombie())
            continue;

        io << p->idName();

        auto pos = io.beginSkip();
        p->serialize(io);
        io.endSkip(pos);
    }
}

void Parameters::deserialize(IO::DataStream & io)
{
    // read parameters
    io.readHeader("params", 1);

    qint32 num;
    io >> num;

    for (int i=0; i<num; ++i)
    {
        QString id;
        io >> id;

        try
        {
            // length for skipping
            qint64 length;
            io >> length;

            Parameter * p = findParameter(id);
            if (!p)
            {
    #ifdef MO_DEBUG
                MO_IO_WARNING(READ, "skipping unknown parameter '" << id << "' "
                                    "in input stream.");
    #endif
                io.skip(length);
            }
            else
                p->deserialize(io);
        }
        catch (Exception& e)
        {
            e << "\n  in param-id '" << id <<"'";
            throw;
        }
    }

}

const QString& Parameters::idName() const
{
    static QString s;
    return object() ? object()->idName() : s;
}

QList<Parameter*> Parameters::getVisibleGraphParameters() const
{
    QList<Parameter*> list;
    for (auto p : parameters_)
        if (p->isVisible() && p->isVisibleInGraph())
            list << p;
    return list;
}

Parameter * Parameters::findParameter(const QString &id)
{
    for (auto p : parameters_)
        if (p->idName() == id)
            return p;

    // check synonyms
    for (auto p : parameters_)
        if (p->hasSynonymId(id))
            return p;

    return 0;
}

Parameter * Parameters::findParameterName(const QString &name)
{
    for (auto p : parameters_)
        if (p->name() == name)
            return p;

    return 0;
}

QMap<QString, QList<Parameter*>> Parameters::getParameterGroups() const
{
    QMap<QString, QList<Parameter*>> map;
    for (auto p : parameters_)
    {
        auto i = map.find(p->groupName());
        if (i == map.end())
            map.insert(p->groupName(), QList<Parameter*>() << p);
        else
            i.value() << p;
    }
    return map;
}

QList<Object*> Parameters::getModulatingObjectsList(bool recursive) const
{
    ObjectConnectionGraph graph;
    getModulatingObjects(graph, recursive);
    auto list = graph.makeLinear();
    list.removeOne(object());
    return list;
}

void Parameters::getModulatingObjects(ObjectConnectionGraph& graph, bool recursive) const
{
    for (auto p : parameters_)
        p->getModulatingObjects(graph, recursive);
}

QList<QString> Parameters::getModulatorIds() const
{
    QSet<QString> set;
    getModulatorIds(set);
    return set.toList();
}

void Parameters::getModulatorIds(QSet<QString>& set) const
{
    for (auto p : parameters_)
    {
        auto l = p->getModulatorIds();
        for (auto o : l)
            set.insert(o);
    }
}

bool Parameters::haveInputsChanged(const RenderTime& time) const
{
    for (Parameter * p : parameters_)
        if (!p->isZombie() && p->isModulated() && p->hasChanged(time))
            return true;

    return false;
}

void Parameters::removeModulators(const QList<QString> &ids)
{
    for (auto p : parameters_)
        p->removeAllModulators(ids);
}


namespace {
    QString toAnchor(QString name)
    {
        name.replace(QRegExp("![a-z,A-Z]"), " ");
        return name;
    }
}

QString Parameters::getParameterDoc() const
{
    QString retstr;
    QTextStream html(&retstr);

    auto groups = getParameterGroups();

    // group index
    html << "<ul>";
    for (auto gi = groups.begin(); gi != groups.end(); ++gi)
    {
        html << "<li><a href=\"#" << toAnchor(gi.key()) << "\">"
             << gi.key() << "</a></li>";
    }
    html << "</ul>";

    // actual parameters
    html << "<ul>";

    for (auto gi = groups.begin(); gi != groups.end(); ++gi)
    {
        const QString groupName = gi.key();
        // exclude activity, it's the same for all
        // and not even fully used right now.
        // XXX hacky exclusion in the face of translations..
        if (groupName == Object::tr("activity"))
            continue;

        const QList<Parameter*>& params = gi.value();

        html << "<a name=\"" << toAnchor(groupName) << "\"></a>"
             << "<h3>" << groupName << "</h3><ul>\n";

        for (const Parameter * p : params)
        {
            html << p->getDoc() << "\n";
        }

        html << "</ul>\n";
    }

    html << "</ul>\n";

    return retstr;
}







// ------------------------------ parameter creation ----------------------------------


void Parameters::beginParameterGroup(const QString &id, const QString &name)
{
    curGroupId_ = id;
    curGroupName_ = name;
}

void Parameters::endParameterGroup()
{
    curGroupId_.clear();
    curGroupName_.clear();
}

void Parameters::beginEvolveGroup(bool evolvable)
{
    isEvolve_ = evolvable;
    isEvolveGroup_ = true;
}

void Parameters::endEvolveGroup()
{
    isEvolveGroup_ = false;
}

void Parameters::p_finishParam_(Parameter* param) const
{
    param->setGroup(curGroupId_, curGroupName_);
    if (isEvolveGroup_)
        param->setDefaultEvolvable(isEvolve_);
}

ParameterFloat * Parameters::createFloatParameter(
        const QString& id, const QString& name, const QString& statusTip,
        Double defaultValue, bool editable, bool modulateable)
{
    return createFloatParameter(id, name, statusTip, defaultValue,
                                -ParameterFloat::infinity, ParameterFloat::infinity,
                                1.0, editable, modulateable);
}

ParameterFloat * Parameters::createFloatParameter(
        const QString& id, const QString& name, const QString &statusTip,
        Double defaultValue, Double smallStep, bool editable, bool modulateable)
{
    return createFloatParameter(id, name, statusTip, defaultValue,
                                -ParameterFloat::infinity, ParameterFloat::infinity,
                                smallStep, editable, modulateable);
}

ParameterFloat * Parameters::createFloatParameter(
        const QString& id, const QString& name, const QString& statusTip,
        Double defaultValue, Double minValue, Double maxValue, Double smallStep,
        bool editable, bool modulateable)
{
    ParameterFloat * param = 0;

    // see if already there

    if (auto p = findParameter(id))
    {
        if (auto pf = dynamic_cast<ParameterFloat*>(p))
        {
            param = pf;
        }
        else
        {
            MO_ASSERT(false, "object '" << idName() << "' requested float "
                      "parameter '" << id << "' "
                      "which is already present as parameter of type " << p->typeName());
        }
    }

    // create new
    if (!param)
    {
        param = new ParameterFloat(object_, id, name);
        parameters_.append(param);

        // first time init
        param->setValue(defaultValue);
    }

    // override potentially previous
    param->setName(name);
    param->setDefaultValue(std::min(maxValue, std::max(minValue, defaultValue )));
    param->setMinValue(minValue);
    param->setMaxValue(maxValue);
    param->setSmallStep(smallStep);
    param->setStatusTip(statusTip);
    param->setEditable(editable);
    param->setModulateable(modulateable);

    p_finishParam_(param);

    return param;
}


ParameterFloatMatrix * Parameters::createFloatMatrixParameter(const QString& id, const QString& name, const QString& statusTip,
        const FloatMatrix& defaultValue,
        bool editable, bool modulateable)
{
    ParameterFloatMatrix * param = 0;

    // see if already there

    if (auto p = findParameter(id))
    {
        if (auto pf = dynamic_cast<ParameterFloatMatrix*>(p))
            param = pf;
        else
        {
            MO_LOGIC_ERROR("object '" << idName() << "' requested float "
                      "parameter '" << id << "' "
                      "which is already present as parameter of type " << p->typeName());
        }
    }

    // create new
    if (!param)
    {
        param = new ParameterFloatMatrix(object_, id, name);
        parameters_.append(param);

        // first time init
        param->setValue(defaultValue);
    }

    // override potentially previous
    param->setName(name);
    param->setDefaultValue(defaultValue);
    param->setStatusTip(statusTip);
    param->setEditable(editable);
    param->setModulateable(modulateable);

    p_finishParam_(param);

    return param;
}




ParameterInt * Parameters::createIntParameter(
        const QString& id, const QString& name, const QString& statusTip,
        Int defaultValue, bool editable, bool modulateable)
{
    return createIntParameter(id, name, statusTip, defaultValue,
                                -ParameterInt::infinity, ParameterInt::infinity,
                                1, editable, modulateable);
}

ParameterInt * Parameters::createIntParameter(
        const QString& id, const QString& name, const QString &statusTip,
        Int defaultValue, Int smallStep, bool editable, bool modulateable)
{
    return createIntParameter(id, name, statusTip, defaultValue,
                                -ParameterInt::infinity, ParameterInt::infinity,
                                smallStep, editable, modulateable);
}

ParameterInt * Parameters::createIntParameter(
        const QString& id, const QString& name, const QString& statusTip,
        Int defaultValue, Int minValue, Int maxValue, Int smallStep,
        bool editable, bool modulateable)
{
    ParameterInt * param = 0;

    // see if already there

    if (auto p = findParameter(id))
    {
        if (auto pf = dynamic_cast<ParameterInt*>(p))
        {
            param = pf;
        }
        else
        {
            MO_ASSERT(false, "object '" << idName() << "' requested int "
                      "parameter '" << id << "' "
                      "which is already present as parameter of type " << p->typeName());
        }
    }

    // create new
    if (!param)
    {
        param = new ParameterInt(object_, id, name);
        parameters_.append(param);

        // first time init
        param->setValue(defaultValue);
    }

    // override potentially previous
    param->setName(name);
    param->setDefaultValue(std::min(maxValue, std::max(minValue, defaultValue )));
    param->setMinValue(minValue);
    param->setMaxValue(maxValue);
    param->setSmallStep(smallStep);
    param->setStatusTip(statusTip);
    param->setEditable(editable);
    param->setModulateable(modulateable);

    p_finishParam_(param);

    return param;
}



ParameterSelect * Parameters::createBooleanParameter(
            const QString& id, const QString& name, const QString& statusTip,
            const QString& offStatusTip, const QString& onStatusTip,
            bool defaultValue, bool editable, bool modulateable)
{
    ParameterSelect * p = createSelectParameter(
            id, name, statusTip,
            { "off", "on" },
            { QObject::tr("off"), QObject::tr("on") },
            { offStatusTip, onStatusTip },
            { false, true },
            defaultValue, editable, modulateable);

    p->setBoolean(true);

    return p;
}

ParameterSelect * Parameters::createSelectParameter(
            const QString& id, const QString& name, const QString& statusTip,
            const QStringList& valueIds, const QStringList& valueNames, const QList<int> &valueList,
            int defaultValue, bool editable, bool modulateable)
{
    MO_ASSERT(valueIds.size() == valueNames.size()
           && valueIds.size() == valueList.size(), "list size mismatch for parameter '" << name << "'");

    return createSelectParameter(id, name, statusTip,
                                 valueIds, valueNames, QStringList(), valueList,
                                 defaultValue, editable, modulateable);
}

ParameterSelect * Parameters::createSelectParameter(
            const QString& id, const QString& name, const QString& statusTip,
            const QStringList& valueIds, const QStringList& valueNames, const QStringList& statusTips,
            const QList<int> &valueList,
            int defaultValue, bool editable, bool modulateable)
{
    MO_ASSERT(valueIds.size() == valueNames.size()
           && valueIds.size() == statusTips.size()
           && valueIds.size() == valueList.size(), "list size mismatch for parameter '" << name << "'");

    ParameterSelect * param = 0;

    // see if already there

    if (auto p = findParameter(id))
    {
        if (auto ps = dynamic_cast<ParameterSelect*>(p))
        {
            param = ps;
        }
        else
        {
            MO_ASSERT(false, "object '" << idName() << "' requested select "
                      "parameter '" << id << "' "
                      "which is already present as parameter of type " << p->typeName());
        }
    }

    // create new
    if (!param)
    {
        param = new ParameterSelect(object_, id, name);
        parameters_.append(param);

        // first time init
        param->setValueList(valueList);
        param->setValueIds(valueIds);
        param->setValueNames(valueNames);
        param->setValue(defaultValue);
    }

    // override potentially previous
    param->setName(name);
    param->setModulateable(modulateable);
    param->setStatusTip(statusTip);
    param->setEditable(editable);
    param->setValueList(valueList);
    param->setValueIds(valueIds);
    param->setValueNames(valueNames);
    param->setStatusTips(statusTips);
    param->setDefaultValue(defaultValue);

    p_finishParam_(param);

    return param;
}

ParameterText * Parameters::createTextParameter(
            const QString& id, const QString& name, const QString& statusTip,
            const QString& defaultValue,
            bool editable, bool modulateable)
{
    return createTextParameter(id, name, statusTip, TT_PLAIN_TEXT,
                               defaultValue, editable, modulateable);
}

ParameterText * Parameters::createTextParameter(
            const QString& id, const QString& name, const QString& statusTip,
            TextType textType,
            const QString& defaultValue,
            bool editable, bool modulateable)
{
    ParameterText * param = 0;

    // see if already there

    if (auto p = findParameter(id))
    {
        if (auto ps = dynamic_cast<ParameterText*>(p))
        {
            param = ps;
        }
        else
        {
            MO_ASSERT(false, "object '" << idName() << "' requested text "
                      "parameter '" << id << "' "
                      "which is already present as parameter of type " << p->typeName());
        }
    }

    // create new
    if (!param)
    {
        param = new ParameterText(object_, id, name);
        parameters_.append(param);

        // first time init
        param->setValue(defaultValue);
    }

    // override potentially previous
    param->setName(name);
    param->setModulateable(modulateable);
    param->setEditable(editable);
    param->setTextType(textType);
    param->setDefaultValue(defaultValue);
    param->setStatusTip(statusTip);

    p_finishParam_(param);

    return param;
}


ParameterFilename * Parameters::createFilenameParameter(
            const QString& id, const QString& name, const QString& statusTip,
            IO::FileType fileType, const QString& defaultValue, bool editable)
{
    ParameterFilename * param = 0;

    // see if already there

    if (auto p = findParameter(id))
    {
        if (auto ps = dynamic_cast<ParameterFilename*>(p))
        {
            param = ps;
        }
        else
        {
            MO_ASSERT(false, "object '" << idName() << "' requested filename "
                      "parameter '" << id << "' "
                      "which is already present as parameter of type " << p->typeName());
        }
    }

    // create new
    if (!param)
    {
        param = new ParameterFilename(object_, id, name);
        parameters_.append(param);

        // first time init
        param->setValue(defaultValue);
    }

    // override potentially previous
    param->setName(name);
    param->setModulateable(false);
    param->setEditable(editable);
    param->setFileType(fileType);
    param->setDefaultValue(defaultValue);
    param->setStatusTip(statusTip);
    param->setDefaultEvolvable(false);

    p_finishParam_(param);

    return param;
}


ParameterImageList* Parameters::createImageListParameter(
            const QString& id, const QString& name, const QString& statusTip,
            const QStringList& defaultValue, bool editable)
{
    ParameterImageList * param = 0;

    // see if already there

    if (auto p = findParameter(id))
    {
        if (auto ps = dynamic_cast<ParameterImageList*>(p))
        {
            param = ps;
        }
        else
        {
            MO_ASSERT(false, "object '" << idName() << "' requested filename "
                      "parameter '" << id << "' "
                      "which is already present as parameter of type " << p->typeName());
        }
    }

    // create new
    if (!param)
    {
        param = new ParameterImageList(object_, id, name);
        parameters_.append(param);

        // first time init
        param->setValue(defaultValue);
    }

    // override potentially previous
    param->setName(name);
    param->setModulateable(false);
    param->setEditable(editable);
    param->setDefaultValue(defaultValue);
    param->setStatusTip(statusTip);
    param->setDefaultEvolvable(false);

    p_finishParam_(param);

    return param;
}


ParameterCallback * Parameters::createCallbackParameter(
            const QString& id, const QString& name, const QString& statusTip,
            std::function<void()> callback, bool modulateable)
{
    ParameterCallback * param = 0;

    // see if already there
    if (auto p = findParameter(id))
    {
        if (auto ps = dynamic_cast<ParameterCallback*>(p))
        {
            param = ps;
        }
        else
        {
            MO_ASSERT(false, "object '" << idName() << "' requested callback "
                      "parameter '" << id << "' "
                      "which is already present as parameter of type " << p->typeName());
        }
    }

    // create new
    if (!param)
    {
        param = new ParameterCallback(object_, id, name);
        parameters_.append(param);

        // first time init
        // ... none
    }

    // override potentially previous
    param->setName(name);
    param->setModulateable(modulateable);
    param->setEditable(false);
    param->setStatusTip(statusTip);
    param->setCallback(callback);
    param->setDefaultEvolvable(false);

    p_finishParam_(param);

    return param;
}

ParameterTexture * Parameters::createTextureParameter(
            const QString& id, const QString& name, const QString& statusTip)
{
    ParameterTexture * param = 0;

    // see if already there
    if (auto p = findParameter(id))
    {
        if (auto ps = dynamic_cast<ParameterTexture*>(p))
        {
            param = ps;
        }
        else
        {
            MO_ASSERT(false, "object '" << idName() << "' requested texture "
                      "parameter '" << id << "' "
                      "which is already present as parameter of type " << p->typeName());
        }
    }

    // create new
    if (!param)
    {
        param = new ParameterTexture(object_, id, name);
        parameters_.append(param);

        // first time init
        // ... none
    }

    // override potentially previous
    param->setName(name);
    param->setModulateable(true);
    param->setEditable(false);
    param->setStatusTip(statusTip);
    param->setDefaultEvolvable(false);

    p_finishParam_(param);

    return param;
}

ParameterGeometry * Parameters::createGeometryParameter(
            const QString& id, const QString& name, const QString& statusTip)
{
    ParameterGeometry * param = 0;

    // see if already there
    if (auto p = findParameter(id))
    {
        if (auto ps = dynamic_cast<ParameterGeometry*>(p))
        {
            param = ps;
        }
        else
        {
            MO_ASSERT(false, "object '" << idName() << "' requested texture "
                      "parameter '" << id << "' "
                      "which is already present as parameter of type " << p->typeName());
        }
    }

    // create new
    if (!param)
    {
        param = new ParameterGeometry(object_, id, name);
        parameters_.append(param);

        // first time init
        // ... none
    }

    // override potentially previous
    param->setName(name);
    param->setModulateable(true);
    param->setEditable(false);
    param->setStatusTip(statusTip);
    param->setDefaultEvolvable(false);

    p_finishParam_(param);

    return param;
}


ParameterTimeline1D * Parameters::createTimeline1DParameter(
        const QString& id, const QString& name, const QString& statusTip,
        const MATH::Timeline1d * defaultValue,
        bool editable)
{
    return createTimeline1DParameter(id, name, statusTip,
                              defaultValue,
                              -ParameterTimeline1D::infinity,
                              +ParameterTimeline1D::infinity,
                              -ParameterTimeline1D::infinity,
                              +ParameterTimeline1D::infinity,
                              editable);
}

ParameterTimeline1D * Parameters::createTimeline1DParameter(
        const QString& id, const QString& name, const QString& statusTip,
        const MATH::Timeline1d * defaultValue,
        Double minTime, Double maxTime,
        bool editable)
{
    return createTimeline1DParameter(id, name, statusTip,
                              defaultValue,
                              minTime, maxTime,
                              -ParameterTimeline1D::infinity,
                              +ParameterTimeline1D::infinity,
                              editable);
}

ParameterTimeline1D * Parameters::createTimeline1DParameter(
        const QString& id, const QString& name, const QString& statusTip,
        const MATH::Timeline1d* defaultValue,
        Double minTime, Double maxTime, Double minValue, Double maxValue,
        bool editable)
{
    ParameterTimeline1D * param = 0;

    // see if already there

    if (auto p = findParameter(id))
    {
        if (auto pf = dynamic_cast<ParameterTimeline1D*>(p))
        {
            param = pf;
        }
        else
        {
            MO_ASSERT(false, "object '" << idName() << "' requested timeline1d "
                      "parameter '" << id << "' "
                      "which is already present as parameter of type " << p->typeName());
        }
    }

    // create new
    if (!param)
    {
        param = new ParameterTimeline1D(object_, id, name);
        parameters_.append(param);

        // first time init
        if (defaultValue)
            param->setValue(*defaultValue);
    }

    // override potentially previous
    param->setName(name);
    if (defaultValue)
        param->setDefaultTimeline(*defaultValue);
    param->setMinTime(minTime);
    param->setMaxTime(maxTime);
    param->setMinValue(minValue);
    param->setMaxValue(maxValue);
    param->setStatusTip(statusTip);
    param->setEditable(editable);

    p_finishParam_(param);

    return param;
}



int Parameters::getTexFormat(int format, int type)
{
    // 8-bit float is default on openGL (i presume)
    if (type == gl::GL_RGBA && format != gl::GL_RED)
        return format;

    switch (gl::GLenum(format))
    {
        case gl::GL_RED:
            switch (gl::GLenum(type))
            {
                case gl::GL_RGBA: return (int)gl::GL_R8;
                case gl::GL_RGBA16F: return (int)gl::GL_R16F;
                case gl::GL_RGBA32F: return (int)gl::GL_R32F;
                default: break;
            }
        break;

        case gl::GL_RG:
            switch (gl::GLenum(type))
            {
                case gl::GL_RGBA: return (int)gl::GL_RG8;
                case gl::GL_RGBA16F: return (int)gl::GL_RG16F;
                case gl::GL_RGBA32F: return (int)gl::GL_RG32F;
                default: break;
            }
        break;

        case gl::GL_RGB:
            switch (gl::GLenum(type))
            {
                case gl::GL_RGBA: return (int)gl::GL_RGB8;
                case gl::GL_RGBA16F: return (int)gl::GL_RGB16F;
                case gl::GL_RGBA32F: return (int)gl::GL_RGB32F;
                default: break;
            }
        break;

        case gl::GL_RGBA:
            switch (gl::GLenum(type))
            {
                case gl::GL_RGBA: return (int)gl::GL_RGBA8;
                case gl::GL_RGBA16F: return (int)gl::GL_RGBA16F;
                case gl::GL_RGBA32F: return (int)gl::GL_RGBA32F;
                default: break;
            }
        break;

        default: break;
    }

    // fallback
    return (int)gl::GL_RGBA;
}

ParameterSelect * Parameters::createTextureFormatParameter(
        const QString& id, const QString& name, const QString& statusTip, int minChan, int maxChan)
{
    QStringList ids, names, tips;
    QList<int> values;
    if (minChan <= 1 && 1 <= maxChan)
    {
        values << (int)gl::GL_RED; ids << "R";
        names << QObject::tr("R");
        tips << QObject::tr("Red channel");
    }
    if (minChan <= 2 && 2 <= maxChan)
    {
        values << (int)gl::GL_RG; ids << "RG"; names << "RG";
        tips << QObject::tr("Red and green channels");
    }
    if (minChan <= 3 && 3 <= maxChan)
    {
        values << (int)gl::GL_RGB; ids << "RGB"; names << "RGB";
        tips << QObject::tr("Full red-green-blue channels");
    }
    if (minChan <= 4 && 4 <= maxChan)
    {
        values << (int)gl::GL_RGBA; ids << "RGBA"; names << "RGBA";
        tips << QObject::tr("Full red-green-blue channels + alpha");
    }
    MO_ASSERT(values.size(), "wrong channel limit in Object("
              << object()->idName() << ")::createTextureFormatParameter()");

    return createSelectParameter(
                id, name, statusTip,
                ids, names, tips, values,
                values.back(),
                true, false);
}


ParameterSelect * Parameters::createTextureTypeParameter(
        const QString& id, const QString& name, const QString& statusTip, int def)
{
    return createSelectParameter(id, name, statusTip,
            { "8f", "16f", "32f" },
            { "8 bit float", "16 bit float", "32 bit float" },
            { "8 bit float", "16 bit float", "32 bit float" },
            { (int)gl::GL_RGBA, (int)gl::GL_RGBA16F, (int)gl::GL_RGBA32F },
            (int)( def >= 32 ? gl::GL_RGBA32F : def >= 16 ? gl::GL_RGBA16F : gl::GL_RGBA ),
            true, false);

}

ParameterTransformation * Parameters::createTransformationParameter(
        const QString &id, const QString &name, const QString &statusTip, const Mat4& defaultValue)
{
    ParameterTransformation* param = 0;

    // see if already there

    if (auto p = findParameter(id))
    {
        if (auto pf = dynamic_cast<ParameterTransformation*>(p))
        {
            param = pf;
        }
        else
        {
            MO_ASSERT(false, "object '" << idName() << "' requested transformation "
                      "parameter '" << id << "' "
                      "which is already present as parameter of type " << p->typeName());
        }
    }

    // create new
    if (!param)
    {
        param = new ParameterTransformation(object_, id, name);
        parameters_.append(param);

        // first time init
        param->setValue(defaultValue);
    }

    // override potentially previous
    param->setName(name);
    param->setDefaultValue(defaultValue);
    param->setStatusTip(statusTip);
    param->setModulateable(true);
    param->setEditable(false);

    p_finishParam_(param);

    return param;
}


ParameterFont * Parameters::createFontParameter(
        const QString &id, const QString &name, const QString &statusTip)
{
    ParameterFont* param = 0;

    // see if already there

    if (auto p = findParameter(id))
    {
        if (auto pf = dynamic_cast<ParameterFont*>(p))
        {
            param = pf;
        }
        else
        {
            MO_ASSERT(false, "object '" << idName() << "' requested font "
                      "parameter '" << id << "' "
                      "which is already present as parameter of type " << p->typeName());
        }
    }

    // create new
    if (!param)
    {
        param = new ParameterFont(object_, id, name);
        parameters_.append(param);

        // first time init
    }

    // override potentially previous
    param->setName(name);
    //param->setDefaultValue();
    param->setStatusTip(statusTip);
    param->setModulateable(false);
    param->setEditable(false);

    p_finishParam_(param);

    return param;
}


} // namespace MO
