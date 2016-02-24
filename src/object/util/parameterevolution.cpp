/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/19/2016</p>
*/

#include <QImage>
#include <QPainter>
#include <QTextStream>
#include <QJsonObject>
#include <QJsonArray>
#include <QFont>
#include <QFontDatabase>

#include "parameterevolution.h"
#include "object/visual/objectgl.h"
#include "object/scene.h"
//#include "object/control/sequencefloat.h"
#include "object/interface/valuetextureinterface.h"
#include "object/interface/valuefloatinterface.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterint.h"
#include "object/param/parameterselect.h"
#include "object/param/parameterfont.h"
#include "object/util/objecteditor.h"
#include "types/properties.h"
#include "math/random.h"
#include "gl/context.h"
#include "gl/texture.h"
#include "gl/texturerenderer.h"
#include "tool/generalimage.h"
#include "gui/painter/grid.h"
#include "gui/painter/valuecurve.h"
#include "io/currenttime.h"
#include "io/error.h"

#include "io/log.h"
namespace MO {

namespace {

    /** wrapper for ValueFloatInterface */
    class FloatCurveData : public GUI::PAINTER::ValueCurveData
    {
    public:
        const ValueFloatInterface * seq;
        RenderTime startTime;
        Double value(Double time) const
            { return seq->valueFloat(0, startTime + time); }
    };

}



MO_REGISTER_EVOLUTION(ParameterEvolution)

struct ParameterEvolution::Private
{
    Private(ParameterEvolution* p)
        : p         (p)
//        , render    (0)
    { }

    struct Param
    {
        Parameter* param;
        Double valueFloat, variance;
        Int valueInt;
        int valueSelect;
        QFont valueFont;
    };

    void getObjectParam(const Parameter*, Param*);
    void setObjectParam(Parameter*, const Param*);
    void getObjectParams();
    void setObjectParams();
    void updateGui();
    bool getImage(QImage& img, const RenderTime& time, ValueTextureInterface* val);
    bool getImage(QImage& img, const RenderTime& time, ValueFloatInterface* val);

    Param* getParam(const QString& id)
    { auto i = paramMap.find(id); return i==paramMap.end() ? nullptr : &(i->second); }
    void mutateParam(Param& param);

    ParameterEvolution* p;
    Object* object;

//    GL::TextureRenderer * render;

    std::map<QString, Param> paramMap;
};

ParameterEvolution::ParameterEvolution(Object* obj)
    : EvolutionBase     ()
    , p_                (new Private(this))
{
    p_->object = obj;
    p_->getObjectParams();

    properties().set("mutation_prob",
              QObject::tr("mutation probability"),
              QObject::tr("Probability of a random change to a parameter"),
              0.3, 0., 1., 0.025);

    properties().set("mutation_amount",
              QObject::tr("mutation amount"),
              QObject::tr("Maximum change per mutation"),
              0.4, 0.025);
    properties().setMin("mutation_amount", 0.);

    properties().set("init_mean",
              QObject::tr("init mean"),
              QObject::tr("Mean value of random initialization"),
              0.0, 0.1);
    properties().set("init_var",
              QObject::tr("init variance"),
              QObject::tr("Range of random initialization"),
              1., 0.1);
    properties().setMin("init_var", 0.0);
    properties().set("init_dev",
              QObject::tr("init deviation"),
              QObject::tr("Distribution of random initialization, "
                          "close to mean (<1) or +/- variance (>1)"),
              1., 0.1);
    properties().setMin("init_dev", 0.0001);

}

ParameterEvolution::~ParameterEvolution()
{
    /*
    if (p_->render && p_->render->isGlInitialized())
        p_->render->releaseGl();
    delete p_->render;
    */
    delete p_;
}

void ParameterEvolution::copyFrom(const EvolutionBase* o)
{
    EvolutionBase::copyFrom(o);
    if (auto e = dynamic_cast<const ParameterEvolution*>(o))
    {
        p_->object = e->p_->object;
        p_->paramMap = e->p_->paramMap;
    }
}

void ParameterEvolution::serialize(QJsonObject& o) const
{
    //o.insert("version", 1);
    QJsonObject ar;
    for (auto& it : p_->paramMap)
    {
        Private::Param* param = &it.second;
        if (dynamic_cast<ParameterFloat*>(param->param))
            ar.insert(param->param->idName(), param->valueFloat);
        else if (dynamic_cast<ParameterInt*>(param->param))
            ar.insert(param->param->idName(), param->valueInt);
        else if (auto ps = dynamic_cast<ParameterSelect*>(param->param))
        {
            if (param->valueSelect >= 0 && param->valueSelect < ps->valueIds().size())
                ar.insert(param->param->idName(), ps->valueIds()[param->valueSelect]);
        }
        else if (dynamic_cast<ParameterFont*>(param->param))
            ar.insert(param->param->idName(), param->valueFont.toString());
    }
    o.insert("parameter", ar);
    if (p_->object)
    {
        o.insert("object", p_->object->idName());
        o.insert("object_class", p_->object->className());
    }
}

void ParameterEvolution::deserialize(const QJsonObject& o)
{
    auto ar = o.find("parameter");
    if (ar == o.end())
        MO_IO_ERROR(VERSION_MISMATCH, "Missing 'parameter' in json evolution");
    if (!ar->isObject())
        MO_IO_ERROR(VERSION_MISMATCH, "'parameter' of wrong type in json evolution");

    // associate Object from current scene
    if (!p_->object && Scene::currentScene())
    {
        QString id = o.value("object").toString();
        if (!id.isEmpty())
        {
            QString classN = o.value("object_class").toString();
            if (!classN.isEmpty())
            if (auto obj = Scene::currentScene()->findChildObject(id, true))
            {
                if (obj->className() == classN)
                {
                    p_->object = obj;
                    p_->getObjectParams();
                    //MO_DEBUG("assigned '" << id << "' to ParameterEvolution");
                }
            }
        }
    }

    auto data = (*ar).toObject();
    auto list = data.keys();
    for (auto key : list)
    if (auto param = p_->getParam(key))
    {
        if (dynamic_cast<ParameterFloat*>(param->param))
            param->valueFloat = data.value(key).toDouble();
        else if (dynamic_cast<ParameterInt*>(param->param))
            param->valueInt = data.value(key).toInt();
        else if (auto ps = dynamic_cast<ParameterSelect*>(param->param))
        {
            int i = ps->valueIds().indexOf( data.value(key).toString() );
            if (i>=0)
                ps->setValue(i);
        }
        else if (dynamic_cast<ParameterFont*>(param->param))
            param->valueFont.fromString( data.value(key).toString() );
    }
}


Object* ParameterEvolution::object() const { return p_->object; }


void ParameterEvolution::Private::getObjectParam(const Parameter* opar, Param* par)
{
    if (auto pf = dynamic_cast<const ParameterFloat*>(opar))
    {
        par->valueFloat = pf->baseValue();
        Double beg = pf->isMinLimit() ? pf->minValue() : 0.;
        par->variance = .5 * std::max(2., pf->defaultValue() - beg);
    }
    if (auto pi = dynamic_cast<const ParameterInt*>(opar))
    {
        par->valueInt = pi->baseValue();
        Int beg = pi->minValue() > -pi->infinity ? pi->minValue() : 0;
        par->variance = .5 * std::max(2, pi->defaultValue() - beg);
    }
    if (auto ps = dynamic_cast<const ParameterSelect*>(opar))
    {
        par->valueSelect = ps->baseValue();
        par->variance = 0.;//(ps->valueList().size() - .3) * .618;
    }
    if (auto pf = dynamic_cast<const ParameterFont*>(opar))
    {
        par->valueFont = pf->baseValue();
        par->variance = 0.;
    }
}

void ParameterEvolution::Private::setObjectParam(Parameter* opar, const Param* par)
{
    if (auto pf = dynamic_cast<ParameterFloat*>(opar))
        pf->setValue(par->valueFloat);
    if (auto pi = dynamic_cast<ParameterInt*>(opar))
        pi->setValue(par->valueInt);
    if (auto ps = dynamic_cast<ParameterSelect*>(opar))
        ps->setValue(par->valueSelect);
    if (auto pf = dynamic_cast<ParameterFont*>(opar))
        pf->setValue(par->valueFont);

    if (object)
        object->onParameterChanged(par->param);
}

void ParameterEvolution::setParameter(Parameter* p)
{
    // find or create entry
    Private::Param* param;
    if (!(param = p_->getParam(p->idName())))
    {
        Private::Param tmp;
        tmp.param = p;
        p_->paramMap.insert(std::make_pair(p->idName(), tmp));
        param = &(p_->paramMap.find(p->idName())->second);

        properties().set("_param_" + p->idName(),
                         QObject::tr("\"%1\"").arg(p->name()),
                         QObject::tr("Enables mutation of specific parameter"),
                         param->param->isEvolvable());
    }

    p_->getObjectParam(param->param, param);
}

void ParameterEvolution::Private::getObjectParams()
{
    paramMap.clear();
    if (!object)
        return;
    for (auto par : object->params()->parameters())
        if ((//par->isVisible()
            !par->isZombie()
             )
               && (   dynamic_cast<ParameterFloat*>(par)
                    || dynamic_cast<ParameterInt*>(par)
                    || dynamic_cast<ParameterSelect*>(par)
                    || dynamic_cast<ParameterFont*>(par)
                    )
                )
            p->setParameter(par);
}

void ParameterEvolution::Private::setObjectParams()
{
    if (!object)
        return;

    for (auto objPar : object->params()->parameters())
    if (auto param = getParam(objPar->idName()))
    if (p->properties().get("_param_" + param->param->idName()).toBool())
    {
        setObjectParam(objPar, param);
    }
}

void ParameterEvolution::Private::updateGui()
{
    if (object)
    {
        object->updateParameterVisibility();
        if (auto edit = object->editor())
        {
            emit edit->parametersChanged(object);
            //if (auto seq = dynamic_cast<SequenceFloat*>(object))
            //    emit edit->sequenceChanged(seq);
        }
    }
}

void ParameterEvolution::updateFromObject()
{
    p_->getObjectParams();
}

void ParameterEvolution::applyParametersToObject(bool updateGui) const
{
    if (!p_->object)
        return;
    p_->setObjectParams();
    if (updateGui)
        p_->updateGui();
}

void ParameterEvolution::getImage(QImage &img) const
{
    if (!p_->object)
    {
        GeneralImage::getErrorImage(img, QObject::tr("No\nobject\nassigned"));
        return;
    }

    bool valid = false;

    auto backup = new ParameterEvolution(p_->object);
    p_->setObjectParams();

    RenderTime time(CurrentTime::time(), 1./60., MO_GFX_THREAD);

    if (auto val = dynamic_cast<ValueTextureInterface*>(p_->object))
        valid = p_->getImage(img, time, val);
    else if (auto val = dynamic_cast<ValueFloatInterface*>(p_->object))
        valid = p_->getImage(img, time, val);


    if (!valid)
        GeneralImage::getErrorImage(img, QObject::tr("No graphic output\nfrom '%1'")
                                                .arg(p_->object->name()) );

    if (backup)
        backup->p_->setObjectParams();
}

bool ParameterEvolution::Private::getImage(
        QImage& img, const RenderTime& time, ValueTextureInterface* valtex)
{
    try
    {
        auto scene = object->sceneObject();
        if (scene)
        {
            scene->glContext()->makeCurrent();
            scene->renderScene(time, true);

            if (auto tex = valtex->valueTexture(0, time))
            {
#if 0
                // XXX NOT WORKING CURRENTLY ???

                // create resampler
                auto render = GL::TextureRenderer(img.width(), img.height() );

                // gl-resize
                tex->bind();
                render.render(tex, true);
                // download image
                if (auto stex = render.texture())
                {
                    stex->bind();
                    QImage img = stex->toQImage();
                    render.releaseGl();
                    valid = true;
                }
                render.releaseGl();
                return true;
#else
                tex->bind();
                img = tex->toQImage().scaled(img.size());
                return true;
#endif
            }
            else
            {
                GeneralImage::getErrorImage(img, QObject::tr("No\ntexture\noutput"));
                return true;
            }
        }
        else
        {
            GeneralImage::getErrorImage(img, QObject::tr("No\nscene\nobject"));
            return true;
        }
    }
    catch (const Exception& e)
    {
        MO_WARNING("In ParameterEvolution: " << e.what());
        GeneralImage::getErrorImage(img, QObject::tr("Error on\nrendering"));
        return true;
    }
}

bool ParameterEvolution::Private::getImage(
        QImage& img, const RenderTime& rtime, ValueFloatInterface* seq)
{
    Double length = 10., minV, maxV;
    seq->getValueFloatRange(0, rtime, length, &minV, &maxV);
    Double delta = std::max(0.1, (maxV - minV) / 50.);
    minV -= delta;
    maxV += delta;
    GUI::UTIL::ViewSpace viewSpace(rtime.second(), minV, length, maxV-minV);

    img.fill(Qt::black);

    QPainter p;
    p.begin(&img);
    p.setRenderHint(QPainter::Antialiasing, true);

        GUI::PAINTER::Grid grid;
        grid.setViewSpace(viewSpace);
        grid.setOptions(GUI::PAINTER::Grid::O_DrawX
                        | GUI::PAINTER::Grid::O_DrawY
                        | GUI::PAINTER::Grid::O_DrawTextY);
        grid.paint(p);

        FloatCurveData fdata;
        fdata.seq = seq;
        fdata.startTime = rtime;
        GUI::PAINTER::ValueCurve curve;
        curve.setViewSpace(viewSpace);
        curve.setCurveData(&fdata);

        curve.paint(p);

    p.end();
    return true;
}

QString ParameterEvolution::toString() const
{
    QString str;
    QTextStream s(&str);
    for (auto& it : p_->paramMap)
    {
        Private::Param
                *param = &it.second,
                backup;

        if (!properties().get("_param_" + param->param->idName()).toBool())
            continue;

        // temporarily change object parameter
        // (for getting the baseValueString)
        p_->getObjectParam(param->param, &backup);
        p_->setObjectParam(param->param, param);

        s << param->param->name()
          << ": " << param->param->baseValueString(true)
          << " (" << param->param->getDocType();
        if (param->variance > 0.)
            s << ", var. " << param->variance;
        s << ")"
          << "\n";

        p_->setObjectParam(param->param, &backup);
    }
    return str;
}


void ParameterEvolution::randomize()
{
    MATH::Twister rnd(properties().get("seed").toUInt());
    double  mean = properties().get("init_mean").toDouble(),
            var = properties().get("init_var").toDouble(),
            dev = 1./properties().get("init_dev").toDouble();

    for (auto it = p_->paramMap.begin(); it != p_->paramMap.end(); ++it)
    {
        Private::Param* param = &it->second;

        bool doEvo = properties().get("_param_" + param->param->idName()).toBool();

        if (auto pf = dynamic_cast<ParameterFloat*>(param->param))
        {
            Double r = rnd() - rnd();
            r = std::pow(std::abs(r), dev) * (r > 0. ? 1. : -1.);
            r = mean + var * r * param->variance;
            r += pf->defaultValue();
            param->valueFloat = !doEvo ? pf->defaultValue() :
                std::max(pf->minValue(), std::min(pf->maxValue(), r ));
        }
        if (auto pi = dynamic_cast<ParameterInt*>(param->param))
        {
            Double r = rnd() - rnd();
            r = std::pow(std::abs(r), dev) * (r > 0. ? 1. : -1.);
            r = mean + var * r * param->variance;
            r += pi->defaultValue();
            param->valueInt = !doEvo ? pi->defaultValue() :
                std::max(pi->minValue(), std::min(pi->maxValue(), int(r) ));
        }
        if (auto ps = dynamic_cast<ParameterSelect*>(param->param))
        {
            if (!doEvo)
                param->valueSelect = ps->defaultValue();
            else
            if (ps->valueList().size())
            {
                size_t idx = rnd.getUInt32() % ps->valueList().size();
                param->valueSelect = ps->valueList()[idx];
            }
        }
        if (auto pf = dynamic_cast<ParameterFont*>(param->param))
        {
            if (!doEvo)
                param->valueFont = pf->defaultValue();
            else
            {
                QFontDatabase db;
                auto fams = db.families(QFontDatabase::Any);
                if (!fams.empty())
                {
                    param->valueFont.setFamily(fams[rnd.getUInt32() % fams.size()]);
                    param->valueFont.setBold(rnd() < .3);
                    param->valueFont.setItalic(rnd() < .2);
                    param->valueFont.setUnderline(rnd() < .2);
                    param->valueFont.setOverline(rnd() < .1);
                    param->valueFont.setFixedPitch(rnd() < .1);
                }
            }
        }
    }

}

void ParameterEvolution::mutate()
{
    MATH::Twister rnd(properties().get("seed").toUInt());
    double  mut_prob  = properties().get("mutation_prob").toDouble(),
            mut_amt = properties().get("mutation_amount").toDouble();

    for (auto it = p_->paramMap.begin(); it != p_->paramMap.end(); ++it)
    if (rnd() < mut_prob)
    {
        Private::Param* param = &it->second;

        if (!properties().get("_param_" + param->param->idName()).toBool())
            continue;

        if (auto pf = dynamic_cast<ParameterFloat*>(param->param))
        {
            Double val = mut_amt * param->variance * (rnd() - rnd());

            param->valueFloat = std::max(pf->minValue(), std::min(pf->maxValue(),
                                    param->valueFloat + val ));
        }
        if (auto pi = dynamic_cast<ParameterInt*>(param->param))
        {
            Int val = mut_amt * param->variance * (rnd() - rnd());

            param->valueInt = std::max(pi->minValue(), std::min(pi->maxValue(),
                                       param->valueInt + val ));
        }
        if (auto ps = dynamic_cast<ParameterSelect*>(param->param))
        {
            if (ps->valueList().size())
            {
                size_t idx = rnd.getUInt32() % ps->valueList().size();
                param->valueSelect = ps->valueList()[idx];
            }
        }
        if (/*auto pf = */dynamic_cast<ParameterFont*>(param->param))
        {
            if (rnd() < .3)
            {
                QFontDatabase db;
                auto fams = db.families(QFontDatabase::Any);
                if (!fams.empty())
                    param->valueFont.setFamily(fams[rnd.getUInt32() % fams.size()]);
            }
            if (rnd() < .3)
                param->valueFont.setBold(!param->valueFont.bold());
            if (rnd() < .2)
                param->valueFont.setItalic(!param->valueFont.italic());
            if (rnd() < .2)
                param->valueFont.setUnderline(!param->valueFont.underline());
            if (rnd() < .1)
                param->valueFont.setOverline(!param->valueFont.overline());
            if (rnd() < .1)
                param->valueFont.setFixedPitch(!param->valueFont.fixedPitch());
        }

    }
}

bool ParameterEvolution::isCompatible(const EvolutionBase* o) const
{
    return dynamic_cast<const ParameterEvolution*>(o);
}

void ParameterEvolution::mate(const EvolutionBase* otherBase)
{
    auto other = dynamic_cast<const ParameterEvolution*>(otherBase);
    if (!other)
        return;

    MATH::Twister rnd(properties().get("seed").toUInt());
    double prob = rnd(.3, .7);

    for (auto it = p_->paramMap.begin(); it != p_->paramMap.end(); ++it)
    if (rnd() < prob)
    {
        Private::Param* param = &it->second;

        if (!properties().get("_param_" + param->param->idName()).toBool())
            continue;

        auto otherParam = other->p_->getParam(param->param->idName());
        // XXX Also copies Parameter pointer
        *param = *otherParam;
    }
}



} // namespace MO

