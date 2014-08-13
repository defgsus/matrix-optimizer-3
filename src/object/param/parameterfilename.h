/** @file parameterfilename.h

    @brief Parameter for filenames

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/13/2014</p>
*/

#ifndef MOSRC_OBJECT_PARAM_PARAMETERFILENAME_H
#define MOSRC_OBJECT_PARAM_PARAMETERFILENAME_H


#include "parameter.h"
#include "types/int.h"
#include "types/float.h"
#include "io/filetypes.h"

namespace MO {

class ParameterFilename : public Parameter
{
public:

    ParameterFilename(Object * object, const QString& idName, const QString& name);

    virtual void serialize(IO::DataStream&) const;
    virtual void deserialize(IO::DataStream&);

    const QString& typeName() const { static QString s("filename"); return s; }

    Modulator * getModulator(const QString &) Q_DECL_OVERRIDE { return 0; }

    // ---------------- getter -----------------

    QString value() const { return value_; }
    QString defaultValue() const { return defaultValue_; }

    IO::FileType fileType() const { return fileType_; }

    // ---------------- setter -----------------

    void setValue(const QString& fn) { value_ = fn; }
    void setDefaultValue(const QString& fn) { defaultValue_ = fn; }

    void setFileType(IO::FileType ft) { fileType_ = ft; }

    /** Opens a dialog and selects a new file.
        If a file was selected, the scene object will be called with
        Scene::setParameterValue().
        @note The scene MUST be present for this call!
        Returns true, when a new file was choosen. */
    bool openFileDialog(QWidget * parent = 0);

private:

    QString value_, defaultValue_;
    IO::FileType fileType_;
};

} // namespace MO


#endif // MOSRC_OBJECT_PARAM_PARAMETERFILENAME_H
