/** @file objectinfodialog.cpp

    @brief Object information

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/23/2014</p>
*/

#include <sstream>

#include <QLayout>
#include <QLabel>
#include <QPushButton>

#include "objectinfodialog.h"
#include "types/vector.h"
#include "io/streamoperators_qt.h"
#include "object/object.h"
#include "object/transform/transformation.h"
#include "object/scene.h"

namespace MO {
namespace GUI {



ObjectInfoDialog::ObjectInfoDialog(QWidget *parent, Qt::WindowFlags flags) :
    QDialog(parent, flags)
{
    auto lv = new QVBoxLayout(this);

        label_ = new QLabel(this);
        lv->addWidget(label_);

        auto but = new QPushButton(tr("Close"), this);
        lv->addWidget(but);
        but->setMaximumWidth(100);//setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        connect(but, SIGNAL(clicked()), this, SLOT(close()));
}


QString matrix2Html(const Mat4& mat)
{
    std::stringstream s;
    s << mat;
    QString tmp = QString::fromStdString(s.str()).toHtmlEscaped();
    tmp.replace('\n', "<br/>");
    return tmp;
}

void ObjectInfoDialog::setObject(Object * o)
{
    setWindowTitle(o->name());

    Double curTime = 0.0;
    if (Scene * s = o->sceneObject())
        curTime = s->sceneTime();

    std::stringstream s, s1;
    s << "<html><b>" << o->name() << "</b><br/>"
      << o->idNamePath() << "/" << o->idName() << "<br/>";

    s << "<p>" << tr("children objects") << ": " << o->numChildren(true) << "</p>";

    // matrix
    if (Transformation * tran = qobject_cast<Transformation*>(o))
    {
        Mat4 mat(1.0);
        tran->applyTransformation(mat, curTime);
        s << "<p>" << tr("applied transformation at %1").arg(curTime)
          << ":<br/>" << matrix2Html(mat) << "</p>";
    }
    else if (o->type() & Object::TG_REAL_OBJECT)
        s << "<p>" << tr("current transformation") << ":<br/>"
          << matrix2Html(o->transformation(0, 0)) << "</p>";

    s << "</html>";

    label_->setText(QString::fromStdString(s.str()));

}


} // namespace MO
} // namespace GUI
