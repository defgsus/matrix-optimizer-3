/** @file ladspaplugin.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 06.05.2015</p>
*/

#ifndef MO_DISABLE_LADSPA

#include <QObject> // for tr()

#include "LadspaPlugin.h"
#include "AudioBuffer.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {
namespace AUDIO {

struct LadspaPlugin::Port
{
    unsigned long index;
    LADSPA_PortDescriptor desc;
    std::vector<LADSPA_Data> buffer;
    LADSPA_Data value; // for control signals

    void zero() { memset(&buffer[0], 0, sizeof(F32) * buffer.size()); }

    bool isAudio() const { return LADSPA_IS_PORT_AUDIO(desc); }
    bool isControl() const { return LADSPA_IS_PORT_CONTROL(desc); }
    bool isInput() const { return LADSPA_IS_PORT_INPUT(desc); }
    bool isOutput() const { return LADSPA_IS_PORT_OUTPUT(desc); }
};


LadspaPlugin::LadspaPlugin()
    : RefCounted("LadspaPlugin")
    , p_sr_             (0)
    , p_bsize_          (0)
    , p_activateCalled_ (false)
    , p_handle_         (0)
    , p_desc_           (0)
{
    static_assert( sizeof(F32) == sizeof(LADSPA_Data), "ladspa.h should work with 32bit floats" );
}

LadspaPlugin::~LadspaPlugin()
{
    p_shutDown_();

    for (auto p : p_ports_)
        delete p;
}

bool LadspaPlugin::isLoaded() const { return p_desc_ != 0; }
bool LadspaPlugin::isInitialized() const { return p_handle_ != 0; }
size_t LadspaPlugin::numPorts() const { return p_desc_ ? p_desc_->PortCount : 0u; }
QString LadspaPlugin::name() const { return p_desc_ ? QString::fromLocal8Bit(p_desc_->Name) : QString(); }
QString LadspaPlugin::idName() const
{
    if (!p_desc_)
        return QString();
    return QString("LADSPA:%1:%2")
            .arg(p_filename_)
            .arg(QString::fromLocal8Bit(p_desc_->Label));
}

QString LadspaPlugin::infoString()
{
    if (p_ports_.empty())
        p_getPorts_();
    return  QObject::tr("name   : %1\n").arg(p_desc_->Name)
        +   QObject::tr("label  : %1\n").arg(p_desc_->Label)
        +   QObject::tr("id     : %1\n").arg(p_desc_->UniqueID) + "\n"
        +   QObject::tr("author : %1\n").arg(p_desc_->Maker)
        +   QObject::tr("copyr  : %1\n").arg(p_desc_->Copyright)
        +   QObject::tr("audios : %1 / %2\n").arg(numInputs()).arg(numOutputs())
        +   QObject::tr("cntrls : %1 / %2\n").arg(numControlInputs()).arg(numControlOutputs())
            ;
}

QList<QString> LadspaPlugin::inputNames() const
{
    QList<QString> list;
    if (!p_desc_)
        return list;
    for (auto p : p_audioIns_)
        list << QString::fromLocal8Bit( p_desc_->PortNames[p->index] );
    return list;
}

QList<QString> LadspaPlugin::outputNames() const
{
    QList<QString> list;
    if (!p_desc_)
        return list;
    for (auto p : p_audioOuts_)
        list << QString::fromLocal8Bit( p_desc_->PortNames[p->index] );
    return list;
}

LadspaPlugin::ControlInfo LadspaPlugin::getControlInfo(size_t index) const
{
    if (!p_desc_ || index >= p_controlIns_.size())
        return ControlInfo();

    size_t idx = p_controlIns_[index]->index;
    const LADSPA_PortRangeHint & range = p_desc_->PortRangeHints[idx];

    ControlInfo inf;
    inf.name = p_desc_->PortNames[idx];
    inf.isBool = LADSPA_IS_HINT_TOGGLED(range.HintDescriptor);
    inf.isInteger = LADSPA_IS_HINT_INTEGER(range.HintDescriptor);
    inf.isMinLimit = LADSPA_IS_HINT_BOUNDED_BELOW(range.HintDescriptor);
    inf.isMaxLimit = LADSPA_IS_HINT_BOUNDED_ABOVE(range.HintDescriptor);
    inf.minValue = range.LowerBound;
    inf.maxValue = range.UpperBound;

//    MO_PRINT(inf.name << " " << inf.minValue << "-" << inf.maxValue <<
//             " " << inf.isMinLimit << "/" << inf.isMaxLimit);

    if (LADSPA_IS_HINT_DEFAULT_0(range.HintDescriptor))
        inf.defaultValue = 0.f;
    else if (LADSPA_IS_HINT_DEFAULT_1(range.HintDescriptor))
        inf.defaultValue = 1.f;
    else if (LADSPA_IS_HINT_DEFAULT_100(range.HintDescriptor))
        inf.defaultValue = 100.f;
    else if (LADSPA_IS_HINT_DEFAULT_440(range.HintDescriptor))
        inf.defaultValue = 440.f;
    else if (LADSPA_IS_HINT_DEFAULT_LOW(range.HintDescriptor))
        inf.defaultValue = .75f * inf.minValue + .25f * inf.maxValue;
    else if (LADSPA_IS_HINT_DEFAULT_MIDDLE(range.HintDescriptor))
        inf.defaultValue = .5f * (inf.minValue + inf.maxValue);
    else if (LADSPA_IS_HINT_DEFAULT_HIGH(range.HintDescriptor))
        inf.defaultValue = .25f * inf.minValue + .75f * inf.maxValue;
    else if (LADSPA_IS_HINT_DEFAULT_MINIMUM(range.HintDescriptor))
        inf.defaultValue = inf.minValue;
    else if (LADSPA_IS_HINT_DEFAULT_MAXIMUM(range.HintDescriptor))
        inf.defaultValue = inf.maxValue;

    if (LADSPA_IS_HINT_SAMPLE_RATE(range.HintDescriptor))
    {
        inf.minValue *= sampleRate();
        inf.maxValue *= sampleRate();
        inf.defaultValue *= sampleRate();
    }

    return inf;
}

void LadspaPlugin::p_getPorts_()
{
    for (auto p : p_ports_)
        delete p;
    p_ports_.clear();
    p_audioIns_.clear();
    p_audioOuts_.clear();
    p_controlIns_.clear();
    p_controlOuts_.clear();

    if (!p_desc_)
        return;

    for (unsigned long i = 0; i<p_desc_->PortCount; ++i)
    {
        auto p = new Port();
        p->index = i;
        p->desc = p_desc_->PortDescriptors[i];
        p_ports_.push_back(p);

        // count specific ports

        if (p->isAudio())
        {
            if (p->isInput())
                p_audioIns_.push_back(p);
            else
                p_audioOuts_.push_back(p);
        }
        if (p->isControl())
        {
            if (p->isInput())
                p_controlIns_.push_back(p);
            else
                p_controlOuts_.push_back(p);
        }
    }
}

/** Creates buffers and connects ports */
void LadspaPlugin::p_connect_()
{
    for (Port * p : p_ports_)
    {
        if (p->isAudio())
        {
            p->buffer.resize(p_bsize_);
            p_desc_->connect_port(p_handle_, p->index, &p->buffer[0]);
        }
        else
        {
            p_desc_->connect_port(p_handle_, p->index, &p->value);
        }
    }
}


bool LadspaPlugin::initialize(size_t bufferSize, size_t sampleRate)
{
    if (!p_desc_)
        return false;

    if (p_handle_)
        p_shutDown_();

    p_handle_ = p_desc_->instantiate(p_desc_, sampleRate);
    if (!p_handle_)
        return false;

    p_bsize_ = bufferSize;
    p_sr_ = sampleRate;

    p_connect_();
    return true;
}

void LadspaPlugin::p_shutDown_()
{
    if (!p_desc_ || !p_handle_)
        return;

    // optional deactivate
    if (p_activateCalled_ && p_desc_->deactivate)
        p_desc_->deactivate(p_handle_);

    p_desc_->cleanup(p_handle_);
    p_handle_ = 0;
}


void LadspaPlugin::setControlValue(size_t index, F32 value)
{
    if (index < numControlInputs())
    {
        p_controlIns_[index]->value = value;
    }
}

void LadspaPlugin::process(const QList<AudioBuffer *> &ins, const QList<AudioBuffer *> &outs)
{
    if (!isInitialized())
        return;

    // copy input channels
    int i, num = std::min(ins.size(), (int)p_audioIns_.size());
    for (i = 0; i<num; ++i)
    {
        if (ins[i])
        {
            MO_ASSERT(ins[i]->blockSize() == blockSize(), "dsp blocksize mismatch "
                      << ins[i]->blockSize() << " != " << blockSize());
            ins[i]->readBlock(&p_audioIns_[i]->buffer[0]);
        }
        else
            p_audioIns_[i]->zero();
    }
    // zero rest
    for (; i<(int)p_audioIns_.size(); ++i)
        p_audioIns_[i]->zero();

    // activate function is optional
    if (p_desc_->activate && !p_activateCalled_)
    {
        p_desc_->activate(p_handle_);
        p_activateCalled_ = true;
    }

    // process
    p_desc_->run(p_handle_, blockSize());

    // copy output channels
    num = std::min(outs.size(), (int)p_audioOuts_.size());
    for (i=0; i<num; ++i)
    {
        if (outs[i])
        {
            MO_ASSERT(outs[i]->blockSize() == blockSize(), "dsp blocksize mismatch "
                      << outs[i]->blockSize() << " != " << blockSize());
            outs[i]->writeBlock(&p_audioOuts_[i]->buffer[0]);
        }
    }
    // zero rest
    for (; i<outs.size(); ++i)
        outs[i]->writeNullBlock();
}


} // namespace AUDIO
} // namespace MO


#endif // #ifndef MO_DISABLE_LADSPA
