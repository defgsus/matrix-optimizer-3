/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/26/2015</p>
*/

#include "parameterimagelist.h"
#include "object/scene.h"
#include "object/util/objecteditor.h"
#include "gui/imagelistdialog.h"
#include "io/datastream.h"
#include "io/error.h"
#include "io/log.h"



namespace MO {

ParameterImageList::ParameterImageList(
        Object * object, const QString& id, const QString& name)
    : Parameter     (object, id, name)
    , diag_         (0)
{
}

ParameterImageList::~ParameterImageList()
{
    if (diag_)
        diag_->deleteLater();
}

void ParameterImageList::serialize(IO::DataStream &io) const
{
    Parameter::serialize(io);

    io.writeHeader("paril", 1);

    io << value_;
}

void ParameterImageList::deserialize(IO::DataStream &io)
{
    Parameter::deserialize(io);

    io.readHeader("paril", 1);

    io >> value_;
}

void ParameterImageList::copyFrom(Parameter* other)
{
    Parameter::copyFrom(other);
    auto p = dynamic_cast<ParameterImageList*>(other);
    if (!p)
        return;
    value_ = p->value_;
    defaultValue_ = p->defaultValue_;
}


void ParameterImageList::openFileDialog(QWidget * parent)
{
    MO_ASSERT(object(), "no object for ParameterImageList::openFileDialog()");
    MO_ASSERT(object()->sceneObject(), "no scene for ParameterImageList::openFileDialog()");
    MO_ASSERT(object()->sceneObject()->editor(), "no Editor for ParameterImageList::openFileDialog()");

    if (!object() || !object()->sceneObject() || !object()->sceneObject()->editor())
        return;

    auto editor = object()->sceneObject()->editor();

    if (!diag_)
    {
        diag_ = new GUI::ImageListDialog(true, parent);
        diag_->setAttribute(Qt::WA_DeleteOnClose);

        QObject::connect(diag_, &GUI::ImageListDialog::destroyed, [=]()
        {
            diag_ = 0;
        });
        QObject::connect(diag_, &GUI::ImageListDialog::listChanged, [=]()
        {
            editor->setParameterValue(this, diag_->imageList());
        });
        QObject::connect(diag_, &GUI::ImageListDialog::accepted, [=]()
        {
            editor->setParameterValue(this, diag_->imageList());
        });
    }

    diag_->setImageList(value_);
    diag_->show();
}


} // namespace MO
