/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/12/2015</p>
*/

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QFile>
#include <QTextStream>
#include <QDebug>

#include "sswproject.h"
#include "model/jsontreemodel.h"
#include "object/objectfactory.h"
#include "object/control/trackfloat.h"
#include "object/control/sequencefloat.h"
#include "object/util/objecteditor.h"
#include "math/timeline1d.h"
#include "io/error.h"

namespace MO {

struct SswProject::Private
{
    Private(SswProject * ssw) : ssw(ssw) { }
    ~Private()
    {
        clear();
    }

    void clear()
    {
        for (auto s : sources)
            delete s;
        sources.clear();
        document = QJsonDocument();
    }

    /** Returns the object for a json url, e.g. "uifm/model/ssc/audio/scenes/scene0" */
    QJsonObject getObjectByPath(const QString& path);
    /** Constructs the SswSources from the QJsonDocument */
    void createSources();
    void createSource(SswSource * source, const QJsonObject& desc);
    void createAnimation(SswSource * source, const QJsonObject& desc);
    static Vec3 arrayToVec3(const QJsonArray& a)
    {
        Vec3 v;
        for (int i=0; i<std::min(a.count(), 3); ++i)
            v[i] = a[i].toDouble();
        return v;
    }

    SswProject * ssw;
    QJsonDocument document;
    QList<SswSource*> sources;
};

SswProject::SswProject()
    : p_        (new Private(this))
{

}

SswProject::~SswProject()
{
    delete p_;
}

const QList<SswSource*>& SswProject::soundSources() const
{
    return p_->sources;
}

void SswProject::load(const QString &name)
{
    QFile file(name);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        MO_IO_ERROR(READ, QObject::tr("Can't open ssw project file\n'%1'\n%2")
                    .arg(name).arg(file.errorString()));

    QJsonParseError error;
    p_->document = QJsonDocument::fromJson(file.readAll(), &error);
    if (p_->document.isNull() || error.error != QJsonParseError::NoError)
        MO_IO_ERROR(READ, QObject::tr("Could not parse ssw project\n'%1'\n%2")
                    .arg(name).arg(error.errorString()));

    p_->createSources();
}

JsonTreeModel * SswProject::createTreeModel() const
{
    auto model = new JsonTreeModel();
    model->setRootObject(p_->document.object());
    return model;
}

QJsonObject SswProject::Private::getObjectByPath(
        const QString &path)
{
    QJsonObject obj = document.object();

    QStringList keys = path.split("/", QString::SkipEmptyParts);
    for (auto & k : keys)
        k.replace("[", "/");

    auto keyit = keys.begin();
    while (keyit != keys.end() && !obj.isEmpty())
    {
        //qInfo() << "searching " << *keyit << " in " << obj.count();
        auto it = obj.find(*keyit);
        if (it == obj.end())
            return QJsonObject();
        if (!it.value().isObject())
            return QJsonObject();
        obj = it.value().toObject();
        ++keyit;
    }
    return obj;
}

void SswProject::Private::createSources()
{
    /*  Scene objects are stored in "uifm/model/ssc/audio/scenes".
        Each scene contains a "sources" object which contains separate source objects.
        Important settings in each source object are:
            isActive(bool), gainDb(double), label(string), type(string), xyz(array[3])
        Less important (right now, at least):
            is2d(bool), rotation(array[3]), isDddOn(bool), isDdlOn(bool), ...
    */
    auto sourcesObj = getObjectByPath("uifm/model/ssc/audio/scenes/scene0/sources");

    for (auto it = sourcesObj.begin(); it != sourcesObj.end(); ++it)
    {
        auto val = it.value();
        // determine if this thing is a source object
        if (!it.key().startsWith("src"))
            continue;
        if (!val.isObject())
            continue;
        auto obj = val.toObject();
        // see if active (skip otherwise)
        auto it2 = obj.find("isActive");
        if (it2 == obj.end() || !it2.value().toBool())
            continue;
        // get index (they are more randomly ordered in the json file)
        int idx = it.key().mid(3).toInt();

        auto source = new SswSource(ssw);
        try
        {
            createSource(source, obj);
            source->p_index_ = idx;
            auto animObj = getObjectByPath(
                         "uifm/automation/"
                         "[uifm[ssc[audio[scenes[scene0[sources[" + it.key());
            if (!animObj.isEmpty())
                createAnimation(source, animObj);
        }
        catch (Exception & e)
        {
            delete source;
            e << "\nIn source object " << it.key();
            throw;
        }
        sources << source;
    }

    // sort by index
    qSort(sources.begin(), sources.end(), [=](SswSource*l, SswSource*r)
    {
        return l->index() < r->index();
    });
}

void SswProject::Private::createSource(SswSource *source, const QJsonObject& obj)
{
#define MO__GET(name__, type__) \
    { \
        auto it = obj.find(name__); \
        if (it == obj.end()) \
            MO_IO_ERROR(VERSION_MISMATCH, "Missing '" name__ "' value in src-object"); \
        val = it.value(); \
        if (!val.is##type__()) \
            MO_IO_ERROR(VERSION_MISMATCH, \
                "'" name__ "' in src-object has wrong type, expected '" #type__ "'"); \
    }

    QJsonValue val;

    MO__GET("gainDb", Double);
    source->p_gainDb_ = val.toDouble();

    MO__GET("label", String);
    source->p_label_ = val.toString();

    MO__GET("type", String);
    source->p_type_ = val.toString() == "planewave"
            ? SswSource::T_PLANE
            : SswSource::T_POINT;

    MO__GET("xyz", Array);
    source->p_pos_ = arrayToVec3(val.toArray());
}

void SswProject::Private::createAnimation(SswSource *source, const QJsonObject& baseobj)
{
    /*
        Automations are objects in "uifm/automation"
        Each automation object has a name like "uifm/ssc/audio/scenes/scene0/sources/src0"
        which identifies the source it is controlling (note the missing "model" in the path).

        Each animation object contains one or multiple objects, named "xyz", "gainDb" or "type".
        Each of those objects contains an array (named "takes") of objects.
        Each of those contains recordId(double), startTime(double), endTime(double),
                              initialValue(dependend), keyframes(array)
        Each array consists of pairs of a time-value (double) and the corresponding values (dependend),
        e.g.: time(d), x(d), y(d), z(d), time, x, y, z, ...
        or: time(d), type(string), time, type, ...
    */

    // for each "xyz", "gainDb" or whathaveyou
    for (auto i1 = baseobj.begin(); i1 != baseobj.end(); ++i1)
    {
        // determine type from name
        SswSource::AnimationType atype = SswSource::AT_XYZ;
        if (i1.key() == "gainDb")
            atype = SswSource::AT_GAIN;
        if (i1.key() == "type")
            atype = SswSource::AT_TYPE;

        // get the object
        auto aobj = i1.value().toObject();
        if (!aobj.isEmpty())
        {
            auto i2 = aobj.find("takes");
            if (i2 != aobj.end())
            {
                auto takesArray = i2.value().toArray();
                for (auto i3 : takesArray)
                {
                    // one actual take
                    auto obj = i3.toObject();

                    auto am = new SswSource::Automation();
                    QJsonValue val;
                    MO__GET("startTime", Double);
                    am->start = val.toDouble();
                    MO__GET("endTime", Double);
                    am->end = val.toDouble();
                    MO__GET("recordId", Double);
                    am->recordId = val.toInt();

                    // get the "keyframes" array
                    auto i4 = obj.find("keyframes");
                    if (i4 == obj.end())
                        MO_IO_ERROR(VERSION_MISMATCH,
                            "'keyframes' not found in animation-take-object");
                    auto array = i4.value().toArray();

                    // read keyframe values
                    switch (atype)
                    {
                        case SswSource::AT_XYZ:
                            am->x = new MATH::Timeline1d();
                            am->y = new MATH::Timeline1d();
                            am->z = new MATH::Timeline1d();
                            for (int i=0; i<array.count() / 4; ++i)
                            {
                                Double ti = array[i*4].toDouble();
                                am->x->add(ti, array[i*4+1].toDouble());
                                am->y->add(ti, array[i*4+2].toDouble());
                                am->z->add(ti, array[i*4+3].toDouble());
                            }
                        break;

                        case SswSource::AT_GAIN:
                            am->gainDb = new MATH::Timeline1d();
                            for (int i=0; i<array.count() / 2; ++i)
                            {
                                Double ti = array[i*2].toDouble();
                                am->gainDb->add(ti, array[i*2+1].toDouble());
                            }
                        break;

                        case SswSource::AT_TYPE:
                            am->type = new MATH::Timeline1d();
                            for (int i=0; i<array.count() / 2; ++i)
                            {
                                Double ti = array[i*2].toDouble();
                                QString name = array[i*2+1].toString();
                                Double val = name == "planewave"
                                        ? SswSource::T_PLANE
                                        : SswSource::T_POINT;
                                am->type->add(ti, val, MATH::Timeline1d::Point::CONSTANT);
                            }
                        break;
                    }
                    am->atype = atype;

                    // store automation
                    source->p_automation_ << am;
                }
            }
        }
    }

    // find min/max time
    auto it = source->p_automation_.begin();
    if (it != source->p_automation_.end())
    {
        source->p_start_ = (*it)->start;
        source->p_end_ = (*it)->end;
        for (; it != source->p_automation_.end(); ++it)
        {
            source->p_start_ = std::min(source->p_start_, (*it)->start);
            source->p_end_ = std::max(source->p_end_, (*it)->end);
        }

    }

    // sort by recordId
    qSort(source->p_automation_.begin(), source->p_automation_.end(),
          [=](SswSource::Automation*l, SswSource::Automation*r)
    {
        return l->recordId < r->recordId;
    });

#undef MO__GET
}

QString SswProject::infoString() const
{
    QString ret;
    QTextStream s(&ret);

    s << "<p>active sources: " << p_->sources.count()
      << "</p>\n"
      << "<table><tr>"
         "<td><b>index</b></td>"
         "<td><b>label</b></td>"
         "<td><b>gain(dB)</b></td>"
         "<td><b>type</b></td>"
         "<td><b>pos</b></td>"
         "<td><b>num auto</b></td>"
         "<td><b>auto start</b></td>"
         "<td><b>auto end</b></td>"
      << "</tr>\n";
    for (const SswSource * src : p_->sources)
    {
        s << "<tr><td>" << (1 + src->index())
          << "</td><td>" << src->label()
          << "</td><td>" << src->gainDb()
          << "</td><td>" << src->typeName()
          << "</td><td>" << src->position().x << ", "
                         << src->position().y << ", " << src->position().z
          << "</td><td>" << src->p_automation_.count()
          << "</td><td>" << src->startTime()
          << "</td><td>" << src->endTime()
          << "</td></tr>\n";
    }
    s << "</table>\n";

    return ret;
}


// ----------------------------- SswSource -----------------------------

SswSource::Automation::Automation()
    : atype     (AT_XYZ)
    , start     (0.)
    , end       (0.)
    , recordId  (0)
    , x         (0)
    , y         (0)
    , z         (0)
    , gainDb    (0)
    , type      (0)
{

}

SswSource::Automation::~Automation()
{
    if (x) x->releaseRef();
    if (y) y->releaseRef();
    if (z) z->releaseRef();
    if (gainDb) gainDb->releaseRef();
    if (type) type->releaseRef();
}

SswSource::SswSource(SswProject * p)
    : p_project_            (p)
    , p_gainDb_             (0.)
    , p_start_              (0.)
    , p_end_                (0.)
    , p_type_               (T_POINT)
    , p_pos_                (0.f, 0.f, 0.f)
    , p_active_             (true)
    , p_index_              (0)
    , p_label_              ("source")
{

}

SswSource::~SswSource()
{
    for (auto a : p_automation_)
        delete a;
}

QString SswSource::typeName() const
{
    if (p_type_ == T_PLANE)
        return "plane";
    else
        return "point";
}




Object * SswSource::createObject()
{
    Object * group = ObjectFactory::loadObject(":/templates/ssw_group.mo3-obj");
    group->setName(QString("src%1_%2").arg(index()).arg(label()));

    Object * src = group->findObjectByNamePath("/Soundsource");
    if (src)
        src->setName(QString("src%1").arg(index()));

    createSequences(group);

    return group;
}

void SswSource::createSequences(Object *group)
{
    for (Automation * a : automations())
    {
        if (a->x)
        {
            auto seq = static_cast<SequenceFloat*>(ObjectFactory::createObject("SequenceFloat"));
            seq->setName(QString("x%1").arg(a->recordId));
            seq->setTimeline(*a->x);
            seq->setStart(a->start);
            seq->setEnd(a->end);

            auto track = group->findObjectByNamePath("Pos X");
            MO_ASSERT(track, "Could not find track in ssw template");
            ObjectPrivate::addObject(track, seq);
        }
    }
}


} // namespace MO

