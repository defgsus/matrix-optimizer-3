/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/14/2016</p>
*/

#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QFileInfo>
#include <QVBoxLayout>

#include "ObjectActions.h"
#include "object/Object.h"
#include "object/control/SequenceFloat.h"
#include "object/util/ObjectEditor.h"
#include "object/util/ObjectFactory.h"
#include "object/interface/GeometryEditInterface.h"
#include "object/interface/ValueTextureInterface.h"
#include "object/interface/ValueShaderSourceInterface.h"
#include "object/interface/EvolutionEditInterface.h"
#include "model/ObjectTreeMimeData.h"
#include "gl/Texture.h"
#include "gl/ShaderSource.h"
#include "gui/util/ObjectMenu.h"
#include "gui/util/AppIcons.h"
#include "gui/util/RecentFiles.h"
#include "gui/GeometryDialog.h"
#include "gui/TextEditDialog.h"
#include "gui/GeometryDialog.h"
#include "gui/EvolutionDialog.h"
#include "gui/widget/SoundFileWidget.h"
#include "io/Files.h"
#include "io/CurrentTime.h"
#include "io/Application.h"

namespace MO {
namespace GUI {

namespace {

    // for sorting the insert-object list
    bool sortObjectList_Priority(const Object * o1, const Object * o2)
    {
        return ObjectFactory::objectPriority(o1) > ObjectFactory::objectPriority(o2);
    }
}


void ObjectActions::createNewObjectActions(
        ActionList &actions, Object * obj,
        QObject* parent,
        std::function<void(Object*)> onCreated,
        std::function<void(Object*)> onAdded)
{
    auto editor = obj->editor();
    if (!editor)
        return;

    actions.addSeparator(parent);

    const bool isRoot = !obj->parentObject();

    /*
    QPoint objPos = QPoint(0, 0);
    if (obj != root)
        if (auto item = parent->itemForObject(obj))
            objPos = item->globalGridPos();
    */

    QAction * a;

    // create new child
    actions.append( a = new QAction(isRoot ?
                tr("new object") : tr("new child object"), parent) );

    QMenu * menu = createObjectsMenu(obj, true, false, true,
                                     Object::TG_ALL, parent);
    a->setMenu(menu);
    QObject::connect(menu, &QMenu::triggered, [=](QAction*act)
    {
        QString id = act->data().toString();
        Object * onew;
        //std::cout << "HERE[" << id << "]" << std::endl;
        if (id == "_template_")
        {
            QString fn;
            onew = ObjectFactory::loadObjectTemplateDialog(&fn);
            if (!fn.isEmpty())
                recentObjectTemplates()->addFilename(fn);
        }
        else if (id.startsWith("_template_"))
            onew = loadObjectTemplate(id.mid(10));
        else
            onew = ObjectFactory::createObject(id);
        if (onew)
        {
            if (onCreated)
                onCreated(onew);
            editor->addObject(obj, onew);//, popupGridPos - objPos);
            onew->releaseRef("finish add");
            if (onAdded)
                onAdded(onew);
        }
    });

    // append texture processor
    if (dynamic_cast<ValueTextureInterface*>(obj))
    {
        menu = createObjectsMenu(obj, true, false, false,
                                 Object::T_SHADER | Object::T_TEXTURE,
                                 parent);
        if (menu)
        {
            actions.append( a = new QAction(tr("append texture processor"), parent) );
            a->setMenu(menu);

            QObject::connect(menu, &QMenu::triggered, [=](QAction*act)
            {
                QString id = act->data().toString();
                Object * onew = ObjectFactory::createObject(id);
                if (onew)
                {
                    if (onCreated)
                        onCreated(onew);
                    editor->appendTextureProcessor(obj, onew);
                    onew->releaseRef("finish add");
                    if (onAdded)
                        onAdded(onew);
                }
            });
        }
    }

}


QMenu * ObjectActions::createObjectsMenu(
        Object *parent, bool with_template, bool with_shortcuts,
        bool child_only, int groups, QObject* pparent)
{
    QList<const Object*> list;
    if (child_only)
        list = ObjectFactory::possibleChildObjects(parent);
    else
        list = ObjectFactory::objects(groups);

    if (list.empty() && !with_template)
        return 0;

    QMenu * menu = new QMenu();

    // from template
    if (with_template)
    {
        auto names = recentObjectTemplates()->filenames();
        if (names.empty())
        {
            auto a = menu->addAction(tr("from template ..."));
            a->setData("_template_");
        }
        else
        {
            auto sub = new QMenu(tr("from template"), menu);
            auto a = sub->addAction(tr("..."));
            a->setData("_template_");
            for (auto& n : names)
            {
                auto fn = QFileInfo(n).baseName();
                a = sub->addAction(fn);
                a->setData("_template_" + n);
            }
            menu->addMenu(sub);
            menu->addSeparator();
        }
    }

    if (!list.empty())
    {
        // sort by priority
        qStableSort(list.begin(), list.end(), sortObjectList_Priority);

        int curprio = ObjectFactory::objectPriority( list.front() );
        QMenu * sub = new QMenu(menu);
        sub->setTitle(ObjectFactory::objectPriorityName(curprio));
        menu->addMenu(sub);
        for (auto o : list)
        {
            if (curprio != ObjectFactory::objectPriority(o))
            {
                curprio = ObjectFactory::objectPriority(o);
                sub = new QMenu(menu);
                sub->setTitle(ObjectFactory::objectPriorityName(curprio));
                menu->addMenu(sub);
            }

            QAction * a = new QAction(AppIcons::iconForObject(o), o->name(), pparent);
            a->setData(o->className());
            if (with_shortcuts)
            {
                if (o->className() == "AxisRotate")
                    a->setShortcut(Qt::ALT + Qt::SHIFT + Qt::Key_R);
                if (o->className() == "Translate")
                    a->setShortcut(Qt::ALT + Qt::SHIFT + Qt::Key_T);
                if (o->className() == "Scale")
                    a->setShortcut(Qt::ALT + Qt::SHIFT + Qt::Key_S);
            }
            sub->addAction(a);
        }
    }

    return menu;
}




void ObjectActions::createEditActions(
        ActionList &actions, Object * obj, QObject* parent)
{
    auto editor = obj->editor();
    if (!editor)
        return;

    actions.addSeparator(parent);

    QAction * a;

    // display error
    if (obj->hasError())
    {
        QString err = obj->errorString();
        if (err.size() > 100)
            err = "..." + err.right(100);
        a = actions.addAction(tr("Error: %1").arg(err), parent);
        // XXX make it visible as error!
        // a->setIcon();
    }

    // set color
    a = actions.addAction(tr("Change color"), parent);
    auto sub = ObjectMenu::createHueMenu();
    a->setMenu(sub);
    QObject::connect(sub, &QMenu::triggered, [=](QAction * a)
    {
        int hue = a->data().toInt();
        editor->setObjectHue(obj, hue);
    });

    // GeometryDialog
    if (GeometryEditInterface * geom = dynamic_cast<GeometryEditInterface*>(obj))
    {
        // edit geometry
        a = actions.addAction(QIcon(":/icon/obj_geometry.png"), tr("Edit geometry"), parent);
        a->setStatusTip(tr("Opens a dialog for editing the attached geometry data"));
        QObject::connect(a, &QAction::triggered, [=]()
        {
            GeometryDialog::openForInterface(geom);
        });
    }

    // display ShaderSource
    if (ValueShaderSourceInterface* ssrc =
            dynamic_cast<ValueShaderSourceInterface*>(obj))
    {
        // show source code
        auto sub = new QMenu(tr("Show shader source"));
        a = actions.addMenu(sub, parent);
        a->setIcon(QIcon(":/icon/obj_glsl.png"));
        a->setStatusTip(tr("Shows the full shader source code including user-code, "
                           "automatic defines and overrides, and include files"));

        a = sub->addAction(tr("vertex shader"));
        a->setData(0);
        a = sub->addAction(tr("fragment shader"));
        a->setData(1);

        /** @todo Does not account for ShaderSource channels */
        QObject::connect(sub, &QMenu::triggered, [=](QAction * a)
        {
            QString src = a->data().toInt() == 0
                    ? ssrc->valueShaderSource(0).vertexSource()
                    : ssrc->valueShaderSource(0).fragmentSource();

            auto diag = new TextEditDialog(src, TT_GLSL, application()->mainWindow());
            diag->setAttribute(Qt::WA_DeleteOnClose, true);
            diag->show();
        });
    }

    if (SequenceFloat * seq = dynamic_cast<SequenceFloat*>(obj))
    if (!obj->findParentObject(Object::T_TRACK_FLOAT))
    {
        // wrap float sequence in float track
        a = actions.addAction(tr("Wrap into track"), parent);
        a->setStatusTip(tr("Creates a track and puts the sequence inside"));
        QObject::connect(a, &QAction::triggered, [=]()
        {
            editor->wrapIntoTrack(seq);
        });

        if (seq->sequenceType() == SequenceFloat::ST_SOUNDFILE)
        if (auto sf = seq->soundFile())
        {
            a = actions.addAction(tr("Display soundfile"), parent);
            QObject::connect(a, &QAction::triggered, [=]()
            {
                openSoundfileDisplay(sf);
            });
        }
    }

    // EvolutionDialog
    if (EvolutionEditInterface * evo = dynamic_cast<EvolutionEditInterface*>(obj))
    {
        for (int i=0; i<evo->evolutionKeys().size(); ++i)
        {
            a = actions.addAction(tr("Evolve %1")
                                  .arg(evo->evolutionKeys()[i]), parent);
            a->setStatusTip(tr("Opens the evolution dialog for editing "
                               "the specimen"));
            QObject::connect(a, &QAction::triggered, [=]()
            {
                EvolutionDialog::openForInterface(evo, evo->evolutionKeys()[i]);
            });
        }
    }


    actions.addSeparator(parent);

    a = actions.addAction(QIcon(":/icon/disk.png"),
                          tr("Save as template ..."), parent);
    a->setStatusTip(tr("Saves the object and it's sub-tree to a "
                       "file for later reuse"));
    QObject::connect(a, &QAction::triggered, [=]()
    {
        auto fn = ObjectFactory::saveObjectTemplateDialog(obj);
        if (!fn.isEmpty())
            recentObjectTemplates()->addFilename(fn);
    });

    // save texture output
    if (dynamic_cast<ValueTextureInterface*>(obj))
    {
        createSaveTextureMenu(actions, obj);
    }

}



void ObjectActions::createSaveTextureMenu(
        ActionList& actions, Object * obj, QObject* parent)
{
    ValueTextureInterface * iface = dynamic_cast<ValueTextureInterface*>(obj);
    if (!iface)
        return;

    // XXX hacky in many regards
    RenderTime rt(CurrentTime::time(), 1./60., MO_GUI_THREAD);

    struct Tex
    {
        const GL::Texture* tex;
        int index;
    };

    // collect texture outputs
    std::vector<Tex> tex;
    for (size_t i=0; ; ++i)
    {
        auto t = iface->valueTexture(i, rt);
        if (!t || !t->isAllocated()
            // XXX can't save cube textures atm.
            || t->isCube())
        {
            if (i < 4)
                continue;
            else
                break;
        }
        Tex x;
        x.tex = t;
        x.index = i;
        tex.push_back(x);
    }

    if (tex.empty())
        return;

    QAction * a;

    // single output
    if (tex.size() == 1)
    {
        a = actions.addAction(QIcon(":/icon/disk.png"), tr("Save texture '%1'")
                              .arg(obj->getOutputName(ST_TEXTURE, tex[0].index)),
                            parent);
        a->setStatusTip(tr("Saves the texture output to a file"));
        QObject::connect(a, &QAction::triggered, [=]() { saveTexture(tex[0].tex); });
    }
    else
    {
        auto sub = new QMenu(tr("Save output texture"));
        sub->setIcon(QIcon(":/icon/disk.png"));
        a = actions.addMenu(sub, parent);
        for (size_t i=0; i<tex.size(); ++i)
        {
            a = sub->addAction(obj->getOutputName(ST_TEXTURE, tex[i].index));
            a->setStatusTip(tr("Saves the texture output to a file"));
            QObject::connect(a, &QAction::triggered,
                             [=]() { saveTexture(tex[i].tex); });
        }
    }
}


void ObjectActions::saveTexture(const GL::Texture* tex)
{
    auto fn = IO::Files::getSaveFileName(IO::FT_TEXTURE);
    if (fn.isEmpty())
        return;
    try
    {
        tex->bind();
        tex->saveImageFile(fn);
    }
    catch (const Exception& e)
    {
        QMessageBox::critical(0, tr("saving image failed"), e.what() );
    }
}


void ObjectActions::createClipboardActions(
        ActionList& actions,
        const QList<Object*>& objects,
        QObject* par)
{
    actions.addSeparator(par);

    const bool plural = objects.size() > 1;
    Object * obj = objects.isEmpty() ? nullptr : objects.first();

    QAction * a;

    auto editor = obj ? obj->editor() : nullptr;

    if (obj && obj->parentObject() != nullptr && editor)
    {
        // copy
        a = actions.addAction(plural ? tr("Copy objects") : tr("Copy"), par);
        a->setStatusTip(tr("Copies the selected object(s) and "
                           "all it's children to the clipboard"));
        a->setShortcut(Qt::CTRL + Qt::Key_C);
        QObject::connect(a, &QAction::triggered, [=]()
        {
            auto data = new ObjectTreeMimeData();
            data->storeObjectTrees(objects);
            application()->clipboard()->setMimeData(data);
        });

        // delete
        a = actions.addAction(plural ? tr("Delete objects") : tr("Delete"), par);
        a->setStatusTip(tr("Deletes the object(s) from the scene"));
        //a->setShortcut(Qt::CTRL + Qt::Key_Delete);
        QObject::connect(a, &QAction::triggered, [=]()
        {
            if (!plural)
                editor->deleteObject(obj);
            else
                editor->deleteObjects(objects);
        });

    }

    if (!plural && ObjectTreeMimeData::isObjectInClipboard() && editor)
    {
        const auto data = application()->clipboard()->mimeData();

        QString pname = obj ? obj->name() : tr("Scene");

        // paste
        a = actions.addAction(tr("Paste into %1").arg(pname), par);
        a->setShortcut(Qt::CTRL + Qt::Key_V);
        QObject::connect(a, &QAction::triggered, [=]()
        {
            auto trees = static_cast<const ObjectTreeMimeData*>(data)->getObjectTrees();
            if (!trees.isEmpty())
                editor->addObjects(obj, trees);
        });
    }
}

RecentFiles* ObjectActions::recentObjectTemplates()
{
    static RecentFiles* r = nullptr;
    if (!r)
    {
        r = new RecentFiles(23, application());
        r->setObjectName("_RecentObjectTemplates");
        r->loadSettings();
        r->setAutoSave(true);
    }
    return r;
}

Object* ObjectActions::loadObjectTemplate(const QString &fn, bool showErrorDiag)
{
    try
    {
        Object * o = ObjectFactory::loadObject(fn);
        recentObjectTemplates()->addFilename(fn);
        return o;
    }
    catch (const Exception& e)
    {
        if (showErrorDiag)
            QMessageBox::critical(nullptr, tr("io error"),
                              tr("Could not load the object template\n%1\n%2")
                              .arg(fn).arg(e.what()));
    }
    return nullptr;
}

void ObjectActions::openSoundfileDisplay(AUDIO::SoundFile* sf)
{
    auto diag = new QDialog(application()->mainWindow());
    diag->setWindowTitle("Soundfile");
    diag->setAttribute(Qt::WA_DeleteOnClose, true);

    auto lv = new QVBoxLayout(diag);

        auto w = new SoundFileWidget(diag);
        lv->addWidget(w);

        w->setSoundFile(sf);

    diag->show();
}


} // namespace GUI
} // namespace MO
