/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/12/2015</p>
*/

#include <QColor>
#include <QBrush>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

#include "JsonTreeModel.h"
#include "io/error.h"

namespace MO {


struct JsonTreeModel::Wrapper
{
    Wrapper(const QJsonValue& v)
        : v         (v)
        , parent    (0)
    {
        if (v.isArray())
        {
            auto a = v.toArray();
            for (auto it : a)
            {
                auto w = new Wrapper(it);
                w->parent = this;
                children << w;
            }
        }
        if (v.isObject())
        {
            auto o = v.toObject();
            for (auto it = o.begin(); it != o.end(); ++it)
            {
                auto w = new Wrapper(it.value());
                w->parent = this;
                w->name = it.key();
                children << w;
            }
        }
    }

    ~Wrapper()
    {
        for (auto w : children)
            delete w;
    }

    QString niceName() const
    {
        QString n;
        if (!name.isEmpty())
            n = "\"" + name + "\" ";
        return n + "(" + typeName() + ")";
    }

    QString typeName() const
    {
        if (v.isArray())
            return QObject::tr("array[%1]").arg(children.count());
        else if (v.isBool())
            return QObject::tr("bool");
        else if (v.isDouble())
            return QObject::tr("double");
        else if (v.isNull())
            return QObject::tr("null");
        else if (v.isObject())
            return QObject::tr("object[%1]").arg(children.count());
        else if (v.isString())
            return QObject::tr("string");
        else return QObject::tr("undef");
    }

    QString valueName() const
    {
        return v.toVariant().toString();
    }

    QJsonValue v;
    QString name;
    Wrapper * parent;
    QList<Wrapper*> children;
};



JsonTreeModel::JsonTreeModel(QObject *parent)
    : QAbstractItemModel        (parent)
    , p_                        (0)
{
    headerNames_
            << "type"
            << "value";
}

void JsonTreeModel::setRootObject(const QJsonObject& rootObject)
{
    beginResetModel();

    rootObject_ = rootObject;
    delete p_;
    if (!rootObject_.isEmpty())
        p_ = new Wrapper(rootObject_);
    else
        p_ = 0;

    endResetModel();
}


JsonTreeModel::Wrapper * JsonTreeModel::wrapperForIndex_(const QModelIndex &index) const
{
    if (index.isValid())
    {
        if (auto w = static_cast<Wrapper*>(index.internalPointer()))
        {
            return w;
        }
    }
    return p_;
}

QModelIndex JsonTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    // sanity check
    if (rootObject_.isEmpty() || row < 0 || column < 0 || column >= headerNames_.size()
            //|| (parent.isValid() && column != 0)
            )
        return QModelIndex();

    auto wrap = wrapperForIndex_(parent);
    MO_ASSERT(wrap, "no object for index <"<<parent.row()<<","<<parent.column()<<">");

    if (row < wrap->children.count())
    {
        return createIndex(row, column, wrap->children[row]);
    }

    return QModelIndex();
}

QModelIndex JsonTreeModel::parent(const QModelIndex &child) const
{
    if (!child.isValid() || rootObject_.isEmpty())
        return QModelIndex();

    if (auto wrap = wrapperForIndex_(child))
    {
        // find parent object
        Wrapper * parent = wrap->parent;
        if (!parent || parent == p_)
            return QModelIndex();

        // get grandparent object
        if (Wrapper * pparent = parent->parent)
        {
            // find index of child
            const int row = pparent->children.indexOf(parent);
            if (row >= 0)
                return createIndex(row, 0, (void*)parent);
        }
    }

    return QModelIndex();
}

int JsonTreeModel::rowCount(const QModelIndex &parent) const
{
    if (rootObject_.isEmpty())
        return 0;

    if (auto wrap = wrapperForIndex_(parent))
        return wrap->children.size();

    return 0;
}


int JsonTreeModel::columnCount(const QModelIndex &parent) const
{
    return (parent.isValid() && parent.column() != 0)? 0 : headerNames_.size();
}


QVariant JsonTreeModel::data(const QModelIndex &index, int role) const
{
    if (rootObject_.isEmpty() || !index.isValid() || index.column() < 0
            || index.column() >= headerNames_.size())
        return QVariant();

    if (auto wrap = wrapperForIndex_(index))
    {
        // return text
        if (role == Qt::DisplayRole || role == Qt::EditRole)
        {
            switch (index.column())
            {
                case 0: return wrap->niceName();
                case 1: return wrap->valueName();
                default: MO_LOGIC_ERROR("no DisplayRole defined for column " << index.column());
            }
        }

        // text alignment
        if (role == Qt::TextAlignmentRole)
            return (int)(Qt::AlignLeft | Qt::AlignVCenter);
        /*
        if (role == Qt::BackgroundRole)
        {
            if (index.column() == 2)
                return QBrush(Qt::gray);
            if (index.column() == 1)
                return QBrush(Qt::green);
        }*/
    }

    return QVariant();
}

Qt::ItemFlags JsonTreeModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flag = QAbstractItemModel::flags(index);

    if (!rootObject_.isEmpty() && index.isValid())
    {
        flag |= Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }
    return flag;
}

QVariant JsonTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal && section < headerNames_.size())
        {
            return headerNames_[section];
        }
    }

    return QVariant();
}



} // namespace MO
