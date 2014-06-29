/** @file model.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/

#ifndef MOSRC_GL_MODEL_H
#define MOSRC_GL_MODEL_H

#include <QObject>

namespace MO {
namespace GL {

class Model : public QObject
{
    Q_OBJECT
public:
    explicit Model(QObject *parent = 0);

signals:

public slots:

};

} // namespace GL
} // namespace MO

#endif // MOSRC_GL_MODEL_H
