/** @file parameter.h

    @brief Abstract general purpose parameter for use in Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/27/2014</p>
*/

#ifndef MOSRC_OBJECT_PARAM_PARAMETER_H
#define MOSRC_OBJECT_PARAM_PARAMETER_H

#include <QString>
#include <QList>
#include <QPair>
#include <QMap>

#include "object/object_fwd.h"

namespace MO {


class Parameter
{

public:
    Parameter(Object * object, const QString& idName, const QString& name);
    virtual ~Parameter();

    virtual void serialize(IO::DataStream&) const;
    virtual void deserialize(IO::DataStream&);

    virtual const QString& typeName() const = 0;

    // --------------- getter -------------------

    /** Parent object */
    Object * object() const { return object_; }

    const QString& idName() const { return idName_; }
    const QString& name() const { return name_; }
    const QString& statusTip() const { return statusTip_; }

    /** Returns a string in the form of 'RealObject/SubObject.parameterId'.
        Goes up the branch until the first Object::TG_REAL_OBJECT */
    QString infoName() const;
    /** Returns a string in the form of 'RealObjectId/SubObjectId.parameterId'.
        Goes up the branch until the first Object::TG_REAL_OBJECT */
    QString infoIdName() const;

    const QString& groupId() const { return groupId_; }
    const QString& groupName() const { return groupName_; }

    bool isEditable() const { return isEditable_; }
    bool isModulateable() const { return isModulateable_; }

    /** Returns true if the parameter has at least one modulation source assigned. */
    bool isModulated() const { return !modulators_.isEmpty(); }

    /** Returns true if the parameter should be visible in the ParameterView */
    bool isVisible() const { return isVisible_; }

    /** Returns true if the parameter should be visible in the ObjectGraphView */
    bool isVisibleInGraph() const { return isVisibleGraph_; }

    // -------------- setter --------------------

    void setName(const QString& name) { name_ = name; }
    void setStatusTip(const QString& tip) { statusTip_ = tip; }
    void setEditable(bool enable) { isEditable_ = enable; }
    void setModulateable(bool enable) { isModulateable_ = enable; }

    void setGroup(const QString& id, const QString& name) { groupId_ = id; groupName_ = name; }

    /** Notifies scene (XXX need to refacture into ObjectEditor) */
    void setVisible(bool visible);
    /** Sets the flag for displaying the parameter in the ObjectGraphView */
    void setVisibleGraph(bool visible) { isVisibleGraph_ = visible; }

    // ------------ modulators ------------------

    /** Called when the idNames of objects have changed.
        This happens if a new object/branch is inserted into an existing branch.
        The map maps from old id to new id.
        Call ancestor's code in your derived function! */
    virtual void idNamesChanged(const QMap<QString, QString>& );

    /** Should return an OR combination of all Object types
        that are supported as modulators */
    virtual int getModulatorTypes() const { return 0; }

    /** Returns list of all modulator ids and output ids */
    QList<QPair<QString, QString>> modulatorIds() const;

    /** Adds an Object to the list of modulators.
        Modulators will be collected by
        collectModulators() in the derived class */
    Modulator * addModulator(const QString& idName, const QString& outputId);

    /** Removes the Object from the list of modulators and
        deletes it. */
    void removeModulator(const QString& idName, const QString& outputId);
    /** Removes all modulators coming from object @p idName */
    void removeAllModulators(const QString& idName);

    /** Removes all modulators IDs */
    void removeAllModulators();

    /** Finds the actual objects associated to the modulatorIds(). */
    void collectModulators();

    /** Returns a modulator for the given id.
        The Modulator is created or reused. */
    virtual Modulator * getModulator(const QString& modulatorId, const QString& outputId) = 0;

    /** Returns the Modulator for the given id, or NULL */
    Modulator * findModulator(const QString& modulatorId, const QString &outputId) const;
    /** Returns the first matching Modulator (ignoring the outputId), or NULL */
    Modulator * findModulator(const QString& modulatorId) const;

    /** Returns all modulators (valid after collectModulators) */
    const QList<Modulator*>& modulators() const { return modulators_; }

    /** Returns a list of all modulator objects (valid after collectModulators()) */
    QList<Object*> getModulatingObjects() const;

    /** Returns the list of objects that will be modulating objects when
        this object gets added to @p scene. */
    QList<Object*> getFutureModulatingObjects(const Scene * scene) const;

protected:

    /** Adds the modulator to the list */
    void addModulator_(Modulator *);

private:

    /** Removes all modulators (IDs stay untouched) */
    void clearModulators_();

    Object * object_;

    QString idName_, name_, statusTip_,
            groupId_, groupName_;

    bool isEditable_, isModulateable_,
         isVisible_, isVisibleGraph_;

    //QList<QString> modulatorIds_;

    QList<Modulator*> modulators_;

};

} // namespace MO


#endif // MOSRC_OBJECT_PARAM_PARAMETER_H
