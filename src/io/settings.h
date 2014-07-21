/** @file settings.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/14/2014</p>
*/

#ifndef MOSRC_IO_SETTINGS_H
#define MOSRC_IO_SETTINGS_H

#include <QSettings>

namespace MO {

class Settings : public QSettings
{
    Q_OBJECT
public:
    explicit Settings(QObject *parent = 0);

    QVariant getValue(const QString& key);

signals:

public slots:

private:

    void createDefaultValues_();

    QMap<QString, QVariant> defaultValues_;
};

/** singleton instance */
extern Settings * settings;


} // namespace MO

#endif // MOSRC_IO_SETTINGS_H
