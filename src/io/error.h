/** @file error.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/26/2014</p>
*/

#ifndef MOSRC_IO_ERROR_H
#define MOSRC_IO_ERROR_H

#include <exception>

namespace MO {

class BasicException : public std::exception
{

};





#define MO_IO_ERROR(text__)


} // namespace MO


#endif // MOSRC_IO_ERROR_H
