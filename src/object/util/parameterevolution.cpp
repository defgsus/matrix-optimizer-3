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

#include "parameterevolution.h"
#include "object/visual/objectgl.h"
#include "object/scene.h"
#include "object/interface/valuetextureinterface.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/util/objecteditor.h"
#include "types/properties.h"
#include "math/random.h"
#include "gl/context.h"
#include "gl/texture.h"
#include "gl/texturerenderer.h"
#include "io/currenttime.h"
#include "io/error.h"

#include "io/log.h"
namespace MO {

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
        Double valueFloat;
    };

    void getObjectParam(const Parameter*, Param*);
    void setObjectParam(Parameter*, const Param*);
    void getObjectParams();
    void setObjectParams();
    void updateGui();

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
              0.1, 0., 1., 0.025);

    properties().set("mutation_amount",
              QObject::tr("mutation amount"),
              QObject::tr("Maximum change per mutation"),
              0.3, 0.025);
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
    }
    o.insert("parameter", ar);
}

void ParameterEvolution::deserialize(const QJsonObject& o)
{
    auto ar = o.find("parameter");
    if (ar == o.end())
        MO_IO_ERROR(VERSION_MISMATCH, "Missing 'parameter' in json evolution");
    if (!ar->isObject())
        MO_IO_ERROR(VERSION_MISMATCH, "'parameter' of wrong type in json evolution");
    auto obj = (*ar).toObject();
    auto list = obj.keys();
    for (auto key : list)
    if (auto param = p_->getParam(key))
    {
        if (dynamic_cast<ParameterFloat*>(param->param))
            param->valueFloat = obj.value(key).toDouble();
    }
}


Object* ParameterEvolution::object() const { return p_->object; }


void ParameterEvolution::Private::getObjectParam(const Parameter* opar, Param* par)
{
    if (auto pf = dynamic_cast<const ParameterFloat*>(opar))
        par->valueFloat = pf->baseValue();
}

void ParameterEvolution::Private::setObjectParam(Parameter* opar, const Param* par)
{
    if (auto pf = dynamic_cast<ParameterFloat*>(opar))
        pf->setValue(par->valueFloat);
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

    // assign
    p_->getObjectParam(param->param, param);
}

void ParameterEvolution::Private::getObjectParams()
{
    paramMap.clear();
    if (!object)
        return;
    for (auto par : object->params()->parameters())
        if (par->isVisible()
                && dynamic_cast<ParameterFloat*>(par)
                )
            p->setParameter(par);
}

void ParameterEvolution::Private::setObjectParams()
{
    if (!object)
        return;

    for (auto objPar : object->params()->parameters())
    if (auto param = getParam(objPar->idName()))
    {
        setObjectParam(objPar, param);
    }
}

void ParameterEvolution::Private::updateGui()
{
    if (object && object->editor())
        emit object->editor()->parametersChanged();
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
    bool valid = false;

    auto backup = new ParameterEvolution(p_->object);
    p_->setObjectParams();

    if (auto valtex = dynamic_cast<ValueTextureInterface*>(p_->object))
    {
        RenderTime time(CurrentTime::time(), 1./60., MO_GFX_THREAD);

        try
        {
            auto scene = p_->object->sceneObject();
            if (scene)
            {
                scene->glContext()->makeCurrent();
                scene->renderScene(time, true);

                if (auto tex = valtex->valueTexture(0, time))
                {
    #if 0
                    // create resampler
                    auto render = GL::TextureRenderer(img.width(), img.height() );

                    // gl-resize
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
    #else
                    img = tex->toQImage().scaled(img.size());
                    valid = true;
    #endif
                }
            }
        }
        catch (const Exception& e)
        {
            MO_WARNING("In ParameterEvolution: " << e.what());
        }
    }

    // set black / XXX should say error or something
    if (!valid)
        img.fill(QColor(40,0,0));

    if (backup)
        backup->p_->setObjectParams();
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
          << " (" << param->param->getDocType() << ")"
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

        if (!properties().get("_param_" + param->param->idName()).toBool())
            continue;

        if (auto pf = dynamic_cast<ParameterFloat*>(param->param))
        {
            Double r = rnd() - rnd();
            r = std::pow(std::abs(r), dev) * (r > 0. ? 1. : -1.);
            r = (mean + var * r);
            r += pf->defaultValue();
            param->valueFloat = std::max(pf->minValue(), std::min(pf->maxValue(), r ));
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
            Double val = mut_amt * (rnd() - rnd());

            param->valueFloat = std::max(pf->minValue(), std::min(pf->maxValue(),
                                    param->valueFloat + val ));
        }
    }
}

bool ParameterEvolution::isCompatible(const EvolutionBase* o) const
{
    return dynamic_cast<const ParameterEvolution*>(o);
}

void ParameterEvolution::mate(const EvolutionBase* otherBase)
{
    /*
    auto other = dynamic_cast<const ParameterEvolution*>(otherBase);
    if (!other)
        return;

    MATH::Twister rnd(properties().get("seed").toUInt());
    double range = std::pow(rnd(0.01, 1.), 1.36) * 2. * vector().size(),
           phase = rnd(0., 6.28),
           amp = rnd() * rnd();
    size_t num = std::min(vector().size(), other->vector().size());

    for (size_t i=0; i<num; ++i)
    {
        double v1 = vector(i),
               v2 = other->vector(i),
               mx = amp * std::cos(double(i)/num * 3.14159265 * range + phase);

        p_vec_[i] = v1 + mx * (v2 - v1);
    }
    */
}



} // namespace MO

