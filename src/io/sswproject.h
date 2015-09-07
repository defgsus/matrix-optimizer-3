/** @file

    @brief SpatialSound Wave loader

    <p>[SpatialSound Wave is copyright Fraunhofer Insitute]
    <br/>Sounds really slick, eh?</p>

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/12/2015</p>
*/

#ifndef MOSRC_IO_SSWPROJECT_H
#define MOSRC_IO_SSWPROJECT_H

#include <QString>

#include "types/vector.h"

namespace MO {
namespace MATH { class Timeline1d; }

class JsonTreeModel;
class SswSource;
class Object;

/** SpatialSound Wave project loader.
    This handles the new (since ~2014) JSON format of the SSW.
    Previous XML is not supported.
*/
class SswProject
{
public:
    SswProject();
    ~SswProject();

    static Double sswTime2Sec(Double t);

    // ------------- io -------------

    /** Loads a uifm file.
        @throws IoException on any error */
    void load(const QString& name);

    // ------ getter interface ------

    /** Creates a tree-model with the json data.
        The model is suitable to be viewed in a QTreeView.
        It just displays the plain json data. */
    JsonTreeModel * createTreeModel() const;

    /** Returns an html info string about all sources */
    QString infoString() const;

    /** Sources with automation.
        Ready after load() */
    const QList<SswSource*>& soundSources() const;

private:
    friend class SswSource;
    struct Private;
    Private * p_;
};

/** Representation of one sound source in a SswProject.
    *Currently* this structure is read-only,
    editing of SSW-Projects is not on the table right now. */
class SswSource
{
    friend class SswProject;
    /* private con/destructor */
    SswSource(SswProject*);
    ~SswSource();
    /* disable copy */
    SswSource(const SswSource&);
    void operator = (const SswSource&);

public:
    // -------- types ----------
    enum Type
    {
        T_POINT,
        T_PLANE
    };

    enum AnimationType
    {
        AT_XYZ,
        AT_GAIN,
        AT_TYPE
    };

    struct Automation
    {
    private:
        Automation();
        ~Automation();
        friend class SswSource;
        friend class SswProject;
    public:
        AnimationType atype;
        Double start, end;
        int recordId;
        MATH::Timeline1d
            * x, * y, * z,
            * gainDb, * type;
    };

    // -------- getter ---------

    SswProject * project() const { return p_project_; }
    int index() const { return p_index_; }\
    const QString& label() const { return p_label_; }

    Double startTime() const { return p_start_; }
    Double endTime() const { return p_end_; }
    Double lengthTime() const { return p_end_ - p_start_; }

    bool active() const { return p_active_; }
    const Vec3& position() const { return p_pos_; }
    Double gainDb() const { return p_gainDb_; }
    Type type() const { return p_type_; }
    QString typeName() const;

    const QList<Automation*>& automations() const { return p_automation_; }

    // ----- getter with automation -----

    bool active(Double second) const;
    Vec3 position(Double second) const;
    Double gainDb(Double second) const;
    Type type(Double second) const;

    // ------ object creation -----------

    /** Creates a Group with a SoundSource and Tracks for automation. */
    Object * createObject();
    /** Creates a Group with a SoundSource and Tracks for automation.
        If @p root contains the object it will be reused instead of created. */
    Object * createObject(Object * root, bool& created);
    /** Call this, once the object from createObject() is installed in the Scene
        (and ObjectEditor) */
    void createSequences(Object * group);

private:

    SswProject
        * p_project_;

    Double
        p_gainDb_,
        p_start_,
        p_end_;
    Type p_type_;
    Vec3 p_pos_;
    bool p_active_;
    int p_index_;
    QString p_label_;

    QList<Automation*> p_automation_;
};


} // namespace MO

#endif // MOSRC_IO_SSWPROJECT_H
