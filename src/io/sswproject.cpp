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
#include <QDebug>

#include "sswproject.h"
#include "model/jsontreemodel.h"
#include "io/error.h"

namespace MO {

struct SswProject::Private
{
    Private(SswProject * ssw) : ssw(ssw) { }

    QJsonDocument document;
    SswProject * ssw;
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
    /*
    QJsonObject object = document.object();

    for (const QJsonValue& val : object)
    {
        qInfo() << val;
    }*/
}

JsonTreeModel * SswProject::createModel() const
{
    auto model = new JsonTreeModel();
    model->setRootObject(p_->document.object());
    return model;
}

} // namespace MO

