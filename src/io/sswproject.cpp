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

        Automations are objects in "uifm/automation"
        Each automation object has a name like "uifm/ssc/audio/scenes/scene0/sources/src0"
        which identifies the source it is controlling (note the missing "model" in the path).
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
            sources << source;
        }
        catch (...)
        {
            delete source;
            throw;
        }
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
      << "</tr>\n";
    for (const SswSource * src : p_->sources)
    {
        s << "<tr><td>" << (1 + src->index())
          << "</td><td>" << src->label()
          << "</td><td>" << src->gainDb()
          << "</td><td>" << src->typeName()
          << "</td></tr>\n";
    }
    s << "</table>\n";

    return ret;
}


// ----------------------------- SswSource -----------------------------

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
    , p_tl_x_               (0)
    , p_tl_y_               (0)
    , p_tl_z_               (0)
    , p_tl_gainDb_          (0)
    , p_tl_type_            (0)
{

}

SswSource::~SswSource()
{
    if (p_tl_x_) p_tl_x_->releaseRef();
    if (p_tl_y_) p_tl_y_->releaseRef();
    if (p_tl_z_) p_tl_z_->releaseRef();
    if (p_tl_gainDb_) p_tl_gainDb_->releaseRef();
    if (p_tl_type_) p_tl_type_->releaseRef();
}

QString SswSource::typeName() const
{
    if (p_type_ == T_PLANE)
        return "planewave";
    else
        return "point";
}

} // namespace MO

