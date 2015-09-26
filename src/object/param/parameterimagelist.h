/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/26/2015</p>
*/

#ifndef MOSRC_OBJECT_PARAM_PARAMETERIMAGELIST_H
#define MOSRC_OBJECT_PARAM_PARAMETERIMAGELIST_H


#include "parameter.h"
#include "types/int.h"
#include "types/float.h"
#include "io/filetypes.h"

namespace MO {
namespace GUI { class ImageListDialog; }

class ParameterImageList : public Parameter
{
public:

    ParameterImageList(Object * object, const QString& idName, const QString& name);
    ~ParameterImageList();

    virtual void serialize(IO::DataStream&) const;
    virtual void deserialize(IO::DataStream&);

    const QString& typeName() const { static QString s("imagelist"); return s; }
    SignalType signalType() const Q_DECL_OVERRIDE { return ST_TEXT; }

    Modulator * getModulator(const QString &, const QString &) Q_DECL_OVERRIDE { return 0; }

    // ---------------- getter -----------------

    QStringList value() const { return value_; }
    const QStringList& baseValue() const { return value_; }
    QStringList defaultValue() const { return defaultValue_; }

    QString valueText() const { return value_.join("; "); }
    QString defaultValueText() const { return defaultValue_.join("; "); }

    // ---------------- setter -----------------

    void setValue(const QStringList& fn) { value_ = fn; }
    void setDefaultValue(const QStringList& fn) { defaultValue_ = fn; }

    /** Opens a GUI::ImageListDialog and selects new files.
        If a file was selected, the scene object will be called with
        Scene::setParameterValue().
        This function returns immediately.
        @note The scene MUST be present for this call! */
    void openFileDialog(QWidget * parent = 0);

private:

    QStringList value_, defaultValue_;
    GUI::ImageListDialog * diag_;
};

} // namespace MO


#endif // MOSRC_OBJECT_PARAM_PARAMETERIMAGELIST_H
