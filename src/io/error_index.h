/***************************************************************************

Copyright (C) 2016  stefan.berke @ modular-audio-graphics.com

This source is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 3.0 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this software; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

****************************************************************************/

#ifndef MOSRC_IO_ERROR_INDEX_H
#define MOSRC_IO_ERROR_INDEX_H

#include "error.h"

/** A__ == B__ */
#define MO_ASSERT_EQUAL(A__, B__, text__) \
    MO_ASSERT_IMPL_((A__) == (B__), \
        #A__ << " expected to be equal to " << #B__ \
        << ", " << (A__) << "/" << (B__) << " " << text__)

/** A__ < B__ */
#define MO_ASSERT_LT(A__, B__, text__) \
    MO_ASSERT_IMPL_((A__) < (B__), \
        #A__ << " expected to be less than " << #B__ \
        << ", " << (A__) << "/" << (B__) << " " << text__)

/** A__ <= B__ */
#define MO_ASSERT_LTE(A__, B__, text__) \
    MO_ASSERT_IMPL_((A__) <= (B__), \
        #A__ << " expected to be less than or qual to " << #B__ \
        << ", " << (A__) << "/" << (B__) << " " << text__)

/** A__ > B__ */
#define MO_ASSERT_GT(A__, B__, text__) \
    MO_ASSERT_IMPL_((A__) > (B__), \
        #A__ << " expected to be greater than " << #B__ \
        << ", " << (A__) << "/" << (B__) << " " << text__)

/** A__ >= B__ */
#define MO_ASSERT_GTE(A__, B__, text__) \
    MO_ASSERT_IMPL_((A__) >= (B__), \
        #A__ << " expected to be greater than or qual to " << #B__ \
        << ", " << (A__) << "/" << (B__) << " " << text__)

#endif // MOSRC_IO_ERROR_INDEX_H

