/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/20/2015</p>
*/

#ifndef MOSRC_NETWORK_OSCINPUTS_H
#define MOSRC_NETWORK_OSCINPUTS_H

#include <QObject>

namespace MO {

class OscInput;

/** Static container for OscInput modules */
class OscInputs : public QObject
{
    Q_OBJECT

    OscInputs(QObject * parent = 0);
    ~OscInputs();

public:

    // ----------- static interface ------------

    /** Singleton instance */
    static OscInputs* instance();

    /** Returns a listener for the specified port.
        Only one instance will be created for the given port. */
    static OscInput * getListener(uint16_t port);

    /** Releases a reference on the specified listener.
        If this functions is called as many times as getListener() with
        the same port number, the listener is removed/destroyed. */
    static void releaseListener(uint16_t port);

    /** Returns all currently open listeners */
    static QList<OscInput*> listeners();

signals:

    /** Emitted when getListener() created a listener or
        releaseListener() removed a listener */
    void listenersChanged();
};


} // namespace MO


#endif // MOSRC_NETWORK_OSCINPUTS_H
