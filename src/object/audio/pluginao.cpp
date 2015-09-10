/** @file pluginao.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 06.05.2015</p>
*/

//#ifndef MO_DISABLE_LADSPA

#include "pluginao.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterselect.h"
#include "object/param/parameterfilename.h"
#include "object/util/objecteditor.h"
#include "audio/tool/audiobuffer.h"
#include "audio/tool/ladspaplugin.h"
#include "gui/audioplugindialog.h"
#include "io/datastream.h"
#include "io/filemanager.h"
#include "io/ladspaloader.h"
#include "io/log.h"

#include <cmath>

namespace MO {

MO_REGISTER_OBJECT(PluginAO)

const size_t PluginAO::maxParams = 128;

class PluginAO::Private
{
    public:

    Private(PluginAO *ao)
        : ao(ao), plugin(0), usedPlugin(0), hasNewPlugin(false) { }

    void loadPlugin();

    /** Sets the AO parameters from the plugin parameters */
    void updateParameters(bool setDefaults);
    void updateChannels();

    PluginAO * ao;
    AUDIO::LadspaPlugin * plugin, * usedPlugin;
    volatile bool hasNewPlugin;
    QStringList audioInNames, audioOutNames;

    std::vector<ParameterFloat*> params;
    ParameterFilename * pPlugFile;

};

PluginAO::PluginAO(QObject * parent)
    : AudioObject(parent)
    , p_         (new Private(this))
{
    setName("Plugin");
    setNumberAudioInputsOutputs(0, 0, false);
#ifdef MO_DISABLE_LADSPA
    setError(tr("Audio Plugin object only supports LADSPA(Linux) at the moment"));
#endif
}

PluginAO::~PluginAO()
{
#ifndef MO_DISABLE_LADSPA
    if (p_->plugin)
        p_->plugin->releaseRef();
    if (p_->usedPlugin)
        p_->usedPlugin->releaseRef();
#endif
    delete p_;
}

void PluginAO::serialize(IO::DataStream & io) const
{
    Object::serialize(io);
    io.writeHeader("aoplug",1);
}

void PluginAO::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);
    io.readHeader("aoplug",1);
}

void PluginAO::createParameters()
{
    AudioObject::createParameters();

    params()->beginParameterGroup("plugin", tr("plugin"));
    initParameterGroupExpanded("plugin");

        p_->pPlugFile = params()->createFilenameParameter(
                    "plug_file", tr("select plugin"),
                    tr("Opens the plugin manager to select the plugin"),
                    IO::FT_LADSPA);

        // fixed number of parameters for control values
        for (size_t i=0; i<maxParams; ++i)
            p_->params.push_back( params()->createFloatParameter(
                                      QString("par%1").arg(i),
                                      QString("parameter %1").arg(i+1),
                                      tr("Parameter of the plugin"), 0., 0.1)
                                  );

    params()->endParameterGroup();
}

void PluginAO::onParametersLoaded()
{
    AudioObject::onParametersLoaded();

    p_->loadPlugin();
}

void PluginAO::onParameterChanged(Parameter * p)
{
    AudioObject::onParameterChanged(p);

    if (p == p_->pPlugFile)
        p_->loadPlugin();
}

void PluginAO::updateParameterVisibility()
{
    AudioObject::updateParameterVisibility();

    //p_->updateParameters();
}

void PluginAO::Private::updateParameters(bool setDefaults)
{
#ifndef MO_DISABLE_LADSPA
    size_t count = 0;

    if (plugin)
        count = plugin->numControlInputs();

    for (size_t i=0; i<params.size(); ++i)
    {
        ParameterFloat * p = params[i];
        if (i >= count)
        {
            // saves filesize and performance
            p->setZombie(true);
        }
        else
        {
            auto info = plugin->getControlInfo(i);

            p->setName(info.name);
            p->setVisible(true);
            p->setZombie(false);
            if (info.isBool)
            {
                p->setSmallStep(1);
                p->setMinValue(0);
                p->setMaxValue(1);
            }
            else
            {
                if (info.isInteger)
                    p->setSmallStep(1.);
                else
                    p->setSmallStep(0.1);
                if (info.isMinLimit)
                    p->setMinValue(info.minValue);
                else
                    p->setNoMinValue();
                if (info.isMaxLimit)
                    p->setMaxValue(info.maxValue);
                else
                    p->setNoMaxValue();
            }
            p->setDefaultValue(info.defaultValue);
            if (setDefaults)
                p->setValue(info.defaultValue);
        }
    }

    if (ao->editor())
        emit ao->editor()->parametersChanged();
#else
    Q_UNUSED(setDefaults);
#endif
}

void PluginAO::Private::updateChannels()
{
    size_t numIn = 0, numOut = 0;

#ifndef MO_DISABLE_LADSPA
    if (plugin)
    {
        numIn = plugin->numInputs();
        numOut = plugin->numOutputs();

        audioInNames = plugin->inputNames();
        audioOutNames = plugin->outputNames();
    }
    else
#endif
    {
        audioInNames.clear();
        audioOutNames.clear();
    }

    ao->setNumberAudioInputsOutputs(numIn, numOut);
}

void PluginAO::setNumberThreads(uint num)
{
    AudioObject::setNumberThreads(num);
}

QString PluginAO::getAudioInputName(uint channel) const
{
    if (channel < (uint)p_->audioInNames.size())
        return p_->audioInNames[channel];

    return AudioObject::getAudioInputName(channel);
}

QString PluginAO::getAudioOutputName(uint channel) const
{
    if (channel < (uint)p_->audioOutNames.size())
        return p_->audioOutNames[channel];

    return AudioObject::getAudioOutputName(channel);
}

void PluginAO::setPlugin(AUDIO::LadspaPlugin * p)
{
    if (p_->plugin == p)
        return;

    bool firstTime = true;

    if (p_->plugin)
    {
        firstTime = false;
#ifndef MO_DISABLE_LADSPA
        p_->plugin->releaseRef();
#endif
    }
    p_->plugin = p;

    if (p_->plugin)
    {
#ifndef MO_DISABLE_LADSPA
        p_->plugin->addRef();
        p_->plugin->initialize(256, sampleRate());
#endif
        //MO_DEBUG("PluginAO: plugin '" << p_->plugin->name() << "' initialized");
    }

    p_->updateParameters(!firstTime);
    p_->updateChannels();

    p_->hasNewPlugin = true;

    // XXX signal success or something
}

void PluginAO::Private::loadPlugin()
{
    QString //type = pPlugFile->baseValue().section(':', 0,0),
            fn = pPlugFile->baseValue().section(':', 1,1),
            label = pPlugFile->baseValue().section(':', 2,2);

    if (fn.isEmpty() || label.isEmpty())
    {
        updateParameters(false);
        updateChannels();
        return;
    }

#ifndef MO_DISABLE_LADSPA
    auto plug = IO::LadspaLoader::loadPlugin(IO::fileManager().localFilename(fn), label);

    ao->setPlugin(plug);
    if (plug)
        plug->releaseRef();
#endif
}

void PluginAO::processAudio(uint bsize, SamplePos pos, uint thread)
{
#ifndef MO_DISABLE_LADSPA
    // lazy exchange new plugin
    if (p_->hasNewPlugin)
    {
        if (p_->usedPlugin)
            p_->usedPlugin->releaseRef();
        p_->usedPlugin = p_->plugin;
        if (p_->usedPlugin)
            p_->usedPlugin->addRef();
        p_->hasNewPlugin = false;
    }

    if (!p_->usedPlugin)
        return;

    if (!p_->usedPlugin->isLoaded())
        return;

    // initialize with correct blocksize
    if (!p_->plugin->isInitialized()
        || p_->usedPlugin->blockSize() != bsize
        || p_->usedPlugin->sampleRate() != sampleRate())
        p_->usedPlugin->initialize(bsize, sampleRate());

    // execute
    if (p_->usedPlugin->isInitialized())
    {
        // set control values
        Double time = Double(pos) / sampleRate();
        for (size_t i=0; i<p_->usedPlugin->numControlInputs(); ++i)
        {
            if (i < p_->params.size())
                p_->usedPlugin->setControlValue(i, p_->params[i]->value(time, thread));
        }

        // process dsp
        p_->usedPlugin->process(audioInputs(thread), audioOutputs(thread));
    }
#else
    Q_UNUSED(bsize);
    writeNullBlock(pos, thread);
#endif
}


} // namespace MO

//#endif // #ifndef MO_DISABLE_LADSPA
