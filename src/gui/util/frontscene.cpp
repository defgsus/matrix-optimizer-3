/** @file frontscene.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 31.01.2015</p>
*/

#include "frontscene.h"
#include "gui/item/abstractfrontitem.h"
#include "object/object.h"
#include "object/param/parameters.h"
#include "object/param/parameterint.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterselect.h"
#include "object/param/parametertext.h"
#include "object/param/parameterfilename.h"
#include "object/param/parametertimeline1d.h"

namespace MO {
namespace GUI {


struct FrontScene::Private
{
    Private(FrontScene * s) : gscene(s) { }


    FrontScene * gscene;


};



FrontScene::FrontScene(QObject *parent)
    : QGraphicsScene    (parent)
    , p_                (new Private(this))
{

    auto i = new AbstractFrontItem(0, 0);
    addItem(i);
            new AbstractFrontItem(0, i);
    addItem(new AbstractFrontItem(0, 0));
}

FrontScene::~FrontScene()
{
    delete p_;
}


void FrontScene::setRootObject(Object *root)
{

}


} // namespace GUI
} // namespace MO
