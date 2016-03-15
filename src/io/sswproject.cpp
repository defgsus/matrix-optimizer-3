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
#include "object/util/objectfactory.h"
#include "object/control/trackfloat.h"
#include "object/control/sequencefloat.h"
#include "object/util/objecteditor.h"
#include "math/timeline1d.h"
#include "types/properties.h"
#include "io/error.h"


namespace MO {

Double SswProject::sswTime2Sec(Double t)
{
    return t / 1000. / 2.3183391;//1.174203812;
}

struct SswProject::Private
{
    Private(SswProject * ssw) : ssw(ssw)
    {
        createProperties();
    }

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

    void createProperties();
    /** Returns the object for a json url, e.g. "uifm/model/ssc/audio/scenes/scene0".
        If the object name contains '/' replace them with '['. */
    QJsonObject getObjectByPath(const QString& path);
    /** Constructs the SswSources from the QJsonDocument */
    void createSources();
    void createSource(SswSource * source, const QJsonObject& desc);
    void createAnimation(SswSource * source, const QJsonObject& desc);
    void setTimelineType(MATH::Timeline1d*);
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
    Double leftTime;
    Properties props;
};

SswProject::SswProject()
    : p_        (new Private(this))
{

}

SswProject::~SswProject()
{
    delete p_;
}

const Properties& SswProject::properties() const
{
    return p_->props;
}

void SswProject::setProperties(const Properties& p)
{
    p_->props = p;
}

void SswProject::Private::createProperties()
{
    props.set("single_seq", QObject::tr("single sequence"),
              QObject::tr("If enabled only a single sequence is created for all automations "
                          "of one track"), false);
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
        // get index (they are not necessarily ordered in the json file)
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

    // find leftmost animation time
    leftTime = -1.;
    if (!sources.isEmpty())
    {
        for (auto s : sources)
        if (!s->automations().isEmpty())
        {
            if (leftTime < 0.)
                leftTime = s->startTime();
            else
                leftTime = std::min(leftTime, s->startTime());
        }
    }
    if (leftTime < 0.)
        leftTime = 0.;

    // sort by index
    qSort(sources.begin(), sources.end(), [=](SswSource*l, SswSource*r)
    {
        return l->index() < r->index();
    });
}

void SswProject::Private::createSource(SswSource *source, const QJsonObject& obj)
{
    // assigns QVariant 'val' from children in QJsonObject 'obj'
    // and throws errors for unfound id or wrong type
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

void SswProject::Private::setTimelineType(MATH::Timeline1d * tl)
{
    if (tl->empty())
        return;

    for (auto & p : tl->getData())
    {
        p.second.type = MATH::TimelinePoint::SMOOTH;
    }

    tl->getData().rbegin()->second.type = MATH::TimelinePoint::CONSTANT;
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
                    am->start = sswTime2Sec(val.toDouble());
                    MO__GET("endTime", Double);
                    am->end = sswTime2Sec(val.toDouble());
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
                            //am->z = new MATH::Timeline1d();
                            for (int i=0; i<array.count() / 4; ++i)
                            {
                                Double ti = sswTime2Sec(array[i*4].toDouble());
                                // skip the infinity control points
                                if (ti < 0 || ti > 50000)
                                    continue;
                                am->x->add(ti, array[i*4+1].toDouble());
                                am->y->add(ti, array[i*4+2].toDouble());
                                //am->z->add(ti, array[i*4+3].toDouble());
                            }
                            setTimelineType(am->x);
                            setTimelineType(am->y);
                            //setTimelineType(am->z);
                        break;

                        case SswSource::AT_GAIN:
                            am->gainDb = new MATH::Timeline1d();
                            for (int i=0; i<array.count() / 2; ++i)
                            {
                                Double ti = sswTime2Sec(array[i*2].toDouble());
                                if (ti < 0 || ti > 50000)
                                    continue;
                                am->gainDb->add(ti, array[i*2+1].toDouble());
                            }
                            setTimelineType(am->gainDb);
                        break;

                        case SswSource::AT_TYPE:
                            am->type = new MATH::Timeline1d();
                            for (int i=0; i<array.count() / 2; ++i)
                            {
                                Double ti = sswTime2Sec(array[i*2].toDouble());
                                if (ti < 0 || ti > 50000)
                                    continue;
                                QString name = array[i*2+1].toString();
                                Double val = name == "planewave"
                                        ? SswSource::T_PLANE
                                        : SswSource::T_POINT;
                                am->type->add(ti, val, MATH::TimelinePoint::CONSTANT);
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
    // XXX Don't sort because it seems that
    // automations are sorted in order of creation
    // and may overwrite previous automations
#if 0
    qSort(source->p_automation_.begin(), source->p_automation_.end(),
          [=](SswSource::Automation*l, SswSource::Automation*r)
    {
        return l->recordId < r->recordId;
    });
#endif

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
          << "&nbsp;</td><td>" << src->label()
          << "&nbsp;</td><td>" << src->gainDb()
          << "&nbsp;</td><td>" << src->typeName()
          << "&nbsp;</td><td>" << src->position().x << ", "
                         << src->position().y << ", " << src->position().z
          << "&nbsp;</td><td>" << src->p_automation_.count()
          << "&nbsp;</td><td>" << src->startTime()
          << "&nbsp;</td><td>" << src->endTime()
          << "&nbsp;</td></tr>\n";
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
    if (x) x->releaseRef("SswSource::Automation destroy");
    if (y) y->releaseRef("SswSource::Automation destroy");
    if (z) z->releaseRef("SswSource::Automation destroy");
    if (gainDb) gainDb->releaseRef("SswSource::Automation destroy");
    if (type) type->releaseRef("SswSource::Automation destroy");
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
    bool c;
    return createObject(0, c);
}

Object * SswSource::createObject(Object * root, bool& created)
{
    const bool singleSeq = p_project_->properties().get("single_seq").toBool();

    const QString name = QString("src%1_%2").arg(index()).arg(label());
    Object * group = 0;

    // reuse
    if (root)
        if (auto o = root->findObjectByNamePath(name))
            group = o;

    // create
    if (!group)
    {
        if (singleSeq)
            group = ObjectFactory::loadObject(":/templates/ssw_group_seq.mo3-obj");
        else
            group = ObjectFactory::loadObject(":/templates/ssw_group.mo3-obj");
        group->setName(name);
        created = true;
    }
    else
        created = false;

    Object * src = group->findObjectByNamePath("/Soundsource");
    if (src)
        src->setName(QString("src%1").arg(index()));

    createSequences(group);

    return group;
}

void SswSource::createSequences(Object *group)
{
    const bool singleSeq = p_project_->properties().get("single_seq").toBool();

    auto track = group->findObjectByNamePath("Pos X");
    MO_ASSERT(track, "Could not find track in ssw template");
    ObjectPrivate::deleteChildren(track);
    track = group->findObjectByNamePath("Pos Y");
    MO_ASSERT(track, "Could not find track in ssw template");
    ObjectPrivate::deleteChildren(track);
    track = group->findObjectByNamePath("Pos Z");
    MO_ASSERT(track, "Could not find track in ssw template");
    ObjectPrivate::deleteChildren(track);

    if (!singleSeq)
    {
        for (Automation * a : automations())
        {
            if (a->x)
            {
                auto seq = static_cast<SequenceFloat*>(ObjectFactory::createObject("SequenceFloat"));
                seq->setName(QString("%1 x%2").arg(label()).arg(a->recordId));
                seq->setTimeline(*a->x);
                seq->setStart(a->start - p_project_->p_->leftTime);
                seq->setEnd(a->end - p_project_->p_->leftTime);

                auto track = group->findObjectByNamePath("Pos X");
                MO_ASSERT(track, "Could not find track in ssw template");
                ObjectPrivate::addObject(track, seq);
            }

            if (a->y)
            {
                auto seq = static_cast<SequenceFloat*>(ObjectFactory::createObject("SequenceFloat"));
                seq->setName(QString("%1 y%2").arg(label()).arg(a->recordId));
                seq->setTimeline(*a->y);
                seq->setStart(a->start - p_project_->p_->leftTime);
                seq->setEnd(a->end - p_project_->p_->leftTime);

                auto track = group->findObjectByNamePath("Pos Y");
                MO_ASSERT(track, "Could not find track in ssw template");
                ObjectPrivate::addObject(track, seq);
            }

            if (a->z)
            {
                auto seq = static_cast<SequenceFloat*>(ObjectFactory::createObject("SequenceFloat"));
                seq->setName(QString("%1 z%2").arg(label()).arg(a->recordId));
                seq->setTimeline(*a->z);
                seq->setStart(a->start - p_project_->p_->leftTime);
                seq->setEnd(a->end - p_project_->p_->leftTime);

                auto track = group->findObjectByNamePath("Pos Z");
                MO_ASSERT(track, "Could not find track in ssw template");
                ObjectPrivate::addObject(track, seq);
            }
        }
    }
    else
    {
        SequenceFloat
                *seqX = dynamic_cast<SequenceFloat*>(group->findObjectByNamePath("Pos X")),
                *seqY = dynamic_cast<SequenceFloat*>(group->findObjectByNamePath("Pos Y")),
                *seqZ = dynamic_cast<SequenceFloat*>(group->findObjectByNamePath("Pos Z"));

        for (Automation * a : automations())
        {
            if (a->x && seqX)
                seqX->overwriteTimeline(*a->x, a->start - p_project_->p_->leftTime, true);
            if (a->y && seqY)
                seqY->overwriteTimeline(*a->y, a->start - p_project_->p_->leftTime, true);
            if (a->z && seqZ)
                seqZ->overwriteTimeline(*a->z, a->start - p_project_->p_->leftTime, true);
        }
    }
}


} // namespace MO

