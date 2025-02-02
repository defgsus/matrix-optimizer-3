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
#include <QSet>

#include "object/Object_fwd.h"
#include "types/time.h"


namespace MO {
class Properties;

class Parameter
{

public:

    enum SpecificFlag
    {
        SF_NONE,
        /** ParameterInt is a keycode */
        SF_KEYCODE
    };

    Parameter(Object * object, const QString& idName, const QString& name);
    virtual ~Parameter();

    virtual void serialize(IO::DataStream&) const;
    virtual void deserialize(IO::DataStream&);

    virtual const QString& typeName() const = 0;
    virtual SignalType signalType() const = 0;

    /** Copies values from @p other to this.
        Override to copy actual values.
        Base impl. copies common values. */
    virtual void copyFrom(Parameter* other);

    // -------------- documentation -------------

    /** Returns the html documentation of this parameter. */
    QString getDoc() const;

    /** Returns the typeName().
        Reimplement to change behaviour. */
    virtual QString getDocType() const;

    /** Base implementation returns empty string.
        Reimplement to add, e.g., range and default value,
        or to add documentation for select states. */
    virtual QString getDocValues() const;

    /** Base returns the statusTip().
        Reimplement to add whatever. */
    virtual QString getDocDesc() const;

    // --------------- getter -------------------

    /** Parent object */
    Object * object() const { return object_; }

    const QString& idName() const { return idName_; }
    const QString& name() const { return name_; }
    /** Can be, and is by default, empty */
    const QString& userName() const { return userName_; }
    /** Returns either userName() or name() */
    const QString& displayName() const;
    const QString& statusTip() const { return statusTip_; }
    SpecificFlag specificFlag() const { return p_specFlag_; }

    bool hasSynonymId(const QString& id) const;

    /** Returns a string in the form of 'RealObject/SubObject.parameterId'.
        Goes up the branch until the first Object::TG_REAL_OBJECT */
    QString infoName() const;
    /** Returns a string in the form of 'RealObjectId/SubObjectId.parameterId'.
        Goes up the branch until the first Object::TG_REAL_OBJECT */
    QString infoIdName() const;

    /** Return the baseValue() as string */
    virtual QString baseValueString(bool inShort) const = 0;
    virtual QString valueString(const RenderTime& t, bool inShort) const = 0;

    const QString& groupId() const { return groupId_; }
    const QString& groupName() const { return groupName_; }

    bool isEditable() const { return isEditable_; }
    bool isModulateable() const { return isModulateable_; }

    /** Returns true if the parameter has at least one modulation source assigned. */
    bool isModulated() const { return !modulators_.isEmpty(); }

    /** Returns true if the parameter should be visible in the ParameterView */
    bool isVisible() const { return isVisible_ && !isZombie_; }

    /** Returns true if the parameter should be visible in the ObjectGraphView */
    bool isVisibleInGraph() const { return isVisibleGraph_ && !isZombie_; }

    /** Returns true if the parameter should be visible in the FrontEndView */
    bool isVisibleInterface() const { return isVisibleInterface_ && !isZombie_; }

    /** When true, the parameter is neither visible nor saved in a serialization stream */
    bool isZombie() const { return isZombie_; }

    /** Include in ParameterEvolution by default. Only a hint!
        True by default. */
    bool isEvolvable() const { return isEvolve_; }

    /* Read access to the interface settings */
    //const Properties& interfaceProperties() const { return *iProps_; }

    /** Pulling value change test.
        XXX Should be implemented for all Parameters i guess,
            only for textures right now. */
    virtual bool hasChanged(const RenderTime& time) const { Q_UNUSED(time); return false; }

    // -------------- setter --------------------

    void setName(const QString& name) { name_ = name; }
    void setUserName(const QString& name) { userName_ = name; }
    void setStatusTip(const QString& tip) { statusTip_ = tip; }
    void setEditable(bool enable) { isEditable_ = enable; }
    void setModulateable(bool enable) { isModulateable_ = enable; }
    void setZombie(bool enable) { isZombie_ = enable; }
    void setDefaultEvolvable(bool enable) { isEvolve_ = enable; }
    /** Sets a specific flag/type for the gui widget initialization */
    void setSpecificFlag(SpecificFlag sf) { p_specFlag_ = sf; }

    void setGroup(const QString& id, const QString& name) { groupId_ = id; groupName_ = name; }

    /** Notifies scene (XXX need to refacture into ObjectEditor) */
    bool setVisible(bool visible);
    /** Sets the flag for displaying the parameter in the ObjectGraphView */
    void setVisibleGraph(bool visible) { isVisibleGraph_ = visible; }
    /** Sets the flag for displaying the parameter in the Interface */
    void setVisibleInterface(bool visible) { isVisibleInterface_ = visible; }

    /* Sets the interface properties. Initially empty */
    //void setInterfaceProperties(const Properties& p);

    /** Sets an alias id for backward compatibility of Parameters::deserialize() */
    void addSynonymId(const QString& id);

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
        collectModulators() in the derived class.
        XXX @p outputId is currently only used for AudioObject output conversion. */
    Modulator * addModulator(const QString& idName, const QString& outputId);

    /** Removes the Object from the list of modulators and
        deletes it. */
    void removeModulator(const QString& idName, const QString& outputId);
    /** Removes all modulators coming from object @p idName */
    void removeAllModulators(const QString& idName);
    /** Removes all modulators coming from any of the objects in @p idNames */
    void removeAllModulators(const QList<QString>& idNames);

    /** Removes all modulators IDs */
    void removeAllModulators();

    /** Finds the actual objects associated to the modulatorIds(). */
    void collectModulators();

    /** Removes all modulator ids for which not modulator has been found.
        Must be called after collectModulators() or all modulators will be removed. */
    void clearNullModulators();

    /** Returns a modulator for the given id.
        The Modulator is created or reused. */
    virtual Modulator * getModulator(const QString& modulatorId, const QString& outputId) = 0;

    /** Returns the Modulator for the given id, or NULL */
    Modulator * findModulator(const QString& modulatorId, const QString &outputId) const;
    /** Returns the first matching Modulator (ignoring the outputId), or NULL */
    Modulator * findModulator(const QString& modulatorId) const;

    /** Returns all modulators (valid after collectModulators) */
    const QList<Modulator*>& modulators() const { return modulators_; }

    /** Returns a list of all modulator ids. */
    QList<QString> getModulatorIds() const;

    /** Adds all modulator objects to the graph (valid after collectModulators()) */
    void getModulatingObjects(ObjectConnectionGraph&, bool recursive) const;

    /** Returns the list of objects that will be modulating objects when
        this object gets added to @p scene. */
    void getFutureModulatingObjects(
            ObjectConnectionGraph&, const Scene * scene, bool recursive) const;

    /** Calls getModulatingObjects() and returns the linear list without this
        Parameter's object */
    QList<Object*> getModulatingObjectsList(bool recursive) const;

    /** Calls getFutureModulatingObjects() and returns the linear list without this
        Parameter's object */
    QList<Object*> getFutureModulatingObjectsList(const Scene *scene, bool recursive) const;

protected:

    /** Adds the modulator to the list */
    void addModulator_(Modulator *);

private:

    /** Removes all modulators (IDs stay untouched) */
    void clearModulators_();

    Object * object_;

    QString idName_, name_, userName_, statusTip_,
            groupId_, groupName_;

    bool isEditable_, isModulateable_,
         isVisible_, isVisibleGraph_, isVisibleInterface_,
         isZombie_, isEvolve_;
    SpecificFlag
        p_specFlag_;
    //QList<QString> modulatorIds_;

    QList<Modulator*> modulators_;
    QSet<QString> synonymIds_;
//    Properties * iProps_;
};

} // namespace MO


#endif // MOSRC_OBJECT_PARAM_PARAMETER_H
