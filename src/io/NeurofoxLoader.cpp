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

#include <QFile>
#include <QByteArray>
#include <QVector>
#include <QStringList>

#include "NeurofoxLoader.h"
#include "math/Timeline1d.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {

struct NeurofoxLoader::Private
{
    Private(NeurofoxLoader* p)
        : p         (p)
    { }

    void scanHex(const QString& s);
    void scanFloat(const QString& s);

    NeurofoxLoader* p;

    QVector<DataPoint> data;
};

NeurofoxLoader::NeurofoxLoader()
    : p_        (new Private(this))
{

}

NeurofoxLoader::~NeurofoxLoader()
{
    delete p_;
}

void NeurofoxLoader::loadFile(const QString &filename, Format format)
{
    QFile f(filename);
    if (!f.open(QFile::ReadOnly))
        MO_IO_ERROR(READ, "Could not open Neurofox file\n"
                    << f.errorString());

    QString s = f.readAll();
    if (s.isEmpty())
        MO_IO_ERROR(READ, "Empty Neurofox file");

    p_->data.clear();

    if (format == F_HEX)
        p_->scanHex(s);
    else
        p_->scanFloat(s);
}

void NeurofoxLoader::Private::scanHex(const QString& s)
{
    QStringList lines = s.split('\n');

    if (lines.empty())
        MO_IO_ERROR(READ, "Empty Neurofox file");

    int64_t initTime = -1;
    int lineNum = 0, count = 0;
    for (QString& line : lines)
    {
        ++lineNum;
        if (line.isEmpty())
            continue;
        line.replace(' ', '\t');

        QStringList vals = line.split('\t', QString::SkipEmptyParts);
        if (vals.size() != 5)
            MO_IO_ERROR(READ, "Error reading line " << lineNum << " '"
                        << line << "'");

        int64_t timeStamp = vals[0].toLongLong();
        if (initTime < 0)
            initTime = timeStamp;

        int v1 = (vals[1] + vals[2]).toInt(nullptr, 16),
            v2 = (vals[2] + vals[3]).toInt(nullptr, 16);


        DataPoint p;
        //p.time = (timeStamp - initTime) * 1e-9;
        p.time = Double(count++) / 500.;
        p.v1 = (Double(v1 - 0x800000) / 0x800000 - .5) * 5.;
        p.v2 = (Double(v2 - 0x800000) / 0x800000 - .5) * 5.;
        data << p;

        MO_PRINT(timeStamp << " " << vals[0]);
        MO_PRINT(p.time << " " << p.v1 << " " << p.v2);
    }
}

void NeurofoxLoader::Private::scanFloat(const QString& )
{

}

void NeurofoxLoader::getTimeline1d(MATH::Timeline1d* tl) const
{
    tl->clear();
    for (const DataPoint& d : p_->data)
        tl->add(d.time, d.v1, d.v2);
}



} // namespace MO
