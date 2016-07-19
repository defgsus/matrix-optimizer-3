/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/6/2016</p>
*/

#include <vector>
#include <cmath>
#include <sstream>
#include <random>

#include "beatdetector.h"
#include "math/convolution.h"
#include "resamplebuffer.h"
#include "math/fft.h"
#include "io/error.h"

namespace MO {
namespace AUDIO {

struct BeatDetector::Private
{
    Private(BeatDetector*p)
        : p             (p)
        , numHistory    (0)
        , numFreqResponses(1024)
        , beatHistoryWrite  (0)
        , numConvs      (2)
        , averageTime   (1.f)
        , threshold     (1.3f)
        , sampleRate    (44100.f)
        , freqLow       (0.f)
        , freqHigh      (sampleRate)
        , convAdaptSpeed(.02f)
        , isBeatsBinary (false)
    { }

    void setSize(size_t fftSize, size_t numBins, size_t numHistory);
    void calcBinWidth();

    void sampleCurrent();
    void sampleAverage();
    void sampleBeat();
    void sampleBeatHistory();
    void convolveHistory();
    void sampleCandidates1();
    void sampleCandidates2();
    void sampleCandidates3();
    void sampleBestMessure();

    void perform(const F32* data, size_t num);

    BeatDetector* p;

    MATH::Fft<F32> fft;
    MATH::Convolution<F32> convolve;
    ResampleBuffer<F32> rebuf;
    std::mt19937 rnd;

    std::vector<size_t>
        binWidth, highestResponsePos,
        bestMatch, matchCount, sortedBestMatch;
    std::vector<F32>
        average, current, beat,
        beatHistory, convolution,
        freqResponse,
        highestResponse, candidates,
        convScratch;

    size_t numHistory, numFreqResponses, beatHistoryWrite,
        bandOffset, numConvs;
    F32 averageTime, threshold,
        sampleRate, freqLow, freqHigh,
        convAdaptSpeed;
    bool isBeatsBinary;
};


BeatDetector::BeatDetector(
        size_t fftSize, size_t numBins, size_t numHistory,
        F32 sr)
    : p_        (new Private(this))
{
    setSize(fftSize, numBins, numHistory, sr);
}

BeatDetector::~BeatDetector()
{
    delete p_;
}

void BeatDetector::setSize(size_t f, size_t n, size_t numHistory, F32 sr)
    { p_->sampleRate = sr; p_->setSize(nextPowerOfTwo(f), n, numHistory); }
void BeatDetector::setAverageTime(F32 s) { p_->averageTime = std::max(0.0001f, s); }
void BeatDetector::setThreshold(F32 s) { p_->threshold = s; }
void BeatDetector::setFrequencyRange(F32 l, F32 h)
{
    l = std::max(0.0f, l); h = std::max(l+1.f, h);
    bool change = (l != p_->freqLow || h != p_->freqHigh);
    p_->freqLow = l; p_->freqHigh = h;
    if (change) p_->calcBinWidth();
}
void BeatDetector::setBeatsBinary(bool b) { p_->isBeatsBinary = b; }
void BeatDetector::setConvolutionAdaptSpeed(F32 s)
    { p_->convAdaptSpeed = std::max(0.0001f,std::min(1.f, s )); }
void BeatDetector::setNumConvolutions(size_t n) { p_->numConvs = n; }


void BeatDetector::push(const F32 *data, size_t count)
{
    if (count)
        p_->perform(data, count);
}

size_t BeatDetector::fftSize() const { return p_->fft.size(); }
size_t BeatDetector::numBins() const { return p_->current.size(); }
size_t BeatDetector::numHistory() const { return p_->numHistory; }
size_t BeatDetector::numConvolutions() const { return p_->numConvs; }
size_t BeatDetector::numFreqResponses() const { return p_->numFreqResponses; }

F32 BeatDetector::averageTime() const { return p_->averageTime; }
F32 BeatDetector::threshold() const { return p_->threshold; }
F32 BeatDetector::sampleRate() const { return p_->sampleRate; }
F32 BeatDetector::lowFrequency() const { return p_->freqLow; }
F32 BeatDetector::highFrequency() const { return p_->freqHigh; }
F32 BeatDetector::convolutionAdaptSpeed() const { return p_->convAdaptSpeed; }
F32 BeatDetector::historyLength() const
    { return F32(p_->numHistory * p_->fft.size()) / p_->sampleRate; }

const F32* BeatDetector::currentLevel() const { return &p_->current[0]; }
const F32* BeatDetector::averageLevel() const { return &p_->average[0]; }
const F32* BeatDetector::beat() const { return &p_->beat[0]; }
const F32* BeatDetector::beatHistory(size_t bin) const
    { return &p_->beatHistory[bin*p_->numHistory]; }
const F32* BeatDetector::convolution(size_t bin) const
    { return &p_->convolution[bin*p_->numHistory]; }
const F32* BeatDetector::freqResponse(size_t bin) const
    { return &p_->freqResponse[bin*p_->numFreqResponses*2]; }
const F32* BeatDetector::candidates(size_t bin) const
    { return &p_->candidates[bin*p_->numHistory]; }

size_t BeatDetector::matchCount(size_t bin) const { return p_->matchCount[bin]; }
size_t BeatDetector::lengthBuffers(size_t bin) const { return p_->bestMatch[bin]; }
F32 BeatDetector::lengthNormalized(size_t bin) const
    { return F32(p_->bestMatch[bin]) / p_->numHistory; }
F32 BeatDetector::lengthSeconds(size_t bin) const
    { return F32(p_->bestMatch[bin] * p_->fft.size()) / p_->sampleRate; }
F32 BeatDetector::beatsPerSecond(size_t bin) const
    { const F32 f = lengthSeconds(bin);
      return f > 0.f ? 1.f / f : 0.f; }
size_t BeatDetector::sortedLengthBuffers(size_t bin) const
    { return p_->sortedBestMatch[bin]; }
F32 BeatDetector::sortedLengthNormalized(size_t bin) const
    { return F32(p_->sortedBestMatch[bin]) / p_->numHistory; }
F32 BeatDetector::sortedLengthSeconds(size_t bin) const
    { return F32(p_->sortedBestMatch[bin] * p_->fft.size()) / p_->sampleRate; }
F32 BeatDetector::sortedBeatsPerSecond(size_t bin) const
    { const F32 f = sortedLengthSeconds(bin);
      return f > 0.f ? 1.f / f : 0.f; }


void BeatDetector::Private::setSize(size_t fftSize, size_t numBins, size_t numHis)
{
    fftSize = std::max(size_t(16), fftSize);
    numBins = std::max(size_t(1), std::min(fftSize, numBins));
    numHistory = std::max(size_t(8), numHis);
    numFreqResponses = std::max(numHistory*2, numFreqResponses);

    fft.setSize(fftSize);
    rebuf.setSize(fftSize);
    current.clear(); current.resize(numBins, 0.f);
    average.clear(); average.resize(numBins, 0.f);
    beat.clear(); beat.resize(numBins, 0.f);
    beatHistory.clear(); beatHistory.resize(numBins * numHistory);
    convolution.clear(); convolution.resize(numBins * numHistory);
    freqResponse.clear(); freqResponse.resize(numBins * numFreqResponses * 2);
    candidates.clear(); candidates.resize(numBins * numHistory);
    convScratch.clear(); convScratch.resize(numHistory * 2);
    highestResponse.clear(); highestResponse.resize(numBins);
    highestResponsePos.clear(); highestResponsePos.resize(numBins);
    bestMatch.clear(); bestMatch.resize(numBins);
    matchCount.clear(); matchCount.resize(numBins);
    sortedBestMatch.clear(); sortedBestMatch.resize(numBins);
    beatHistoryWrite = 0;

    calcBinWidth();
}

void BeatDetector::Private::calcBinWidth()
{
    // -- determine weighted bin-gather-size --

    bandOffset = freqLow / sampleRate * fft.size() / 2.f;
    bandOffset = std::min(size_t(fft.size())/2 - 1, bandOffset);

    // special case for one bin
    if (beat.size() == 1)
    {
        binWidth.resize(1);
        binWidth[0] = fft.size();
    }
    else
    {
        std::vector<F32> binWeight(beat.size());
        F32 sum = 0.f;
        for (size_t i=0; i<beat.size(); ++i)
            sum += binWeight[i] = F32(1+i); // fairly trivial weighting

        size_t size = freqHigh / sampleRate * fft.size() / 2.f;
        size = std::min(size, size_t(fft.size()) / 2 - bandOffset);

        size_t off = 0;
        binWidth.clear();
        for (auto w : binWeight)
        {
            size_t x = std::floor(w / sum * size);
            x = std::min(std::max(size_t(1),x), size - off );
            binWidth.push_back(x);
            //MO_PRINT(off << ": " << x);
            off += x;
        }
    }
}


/** Gather bins from fft-amplitude into current */
void BeatDetector::Private::sampleCurrent()
{
    MO_ASSERT(!current.empty(), "");
    MO_ASSERT(binWidth.size() == current.size(), "")

    const F32* src = fft.buffer() + bandOffset;
    for (size_t i=0; i<current.size(); ++i)
    {
        current[i] = 0.f;
        size_t width = binWidth[i];
        for (size_t j=0; j<width; ++j)
        {
            current[i] += *src++;
        }
        //current[i] /= binWidth[i];
    }
}

/** Get average from current */
void BeatDetector::Private::sampleAverage()
{
    MO_ASSERT(current.size() == average.size(), "");
    MO_ASSERT(fft.size(), "");

    const F32 mix = 8.f / std::max(8.f, averageTime * sampleRate / fft.size());
    for (size_t i=0; i<current.size(); ++i)
    {
        average[i] += mix * (current[i] - average[i]);
    }
}

/** If current > average * threshold, breat = current, else 0 */
void BeatDetector::Private::sampleBeat()
{
    MO_ASSERT(current.size() == average.size(), "");

    for (size_t i=0; i<current.size(); ++i)
    {
        beat[i] = current[i] > average[i] * threshold
                        ? (isBeatsBinary ? 1.f : current[i]) : 0.f;
    }
}

/** Put beat into history buffer */
void BeatDetector::Private::sampleBeatHistory()
{
    MO_ASSERT(numHistory, "");
    MO_ASSERT(!beatHistory.empty(), "");

    for (size_t bin=0; bin<beat.size(); ++bin)
    {
        beatHistory[bin*numHistory + beatHistoryWrite] =
                current[bin];
    }
    beatHistoryWrite = (beatHistoryWrite+1) % numHistory;
}

namespace {

    template <typename F>
    void normAndMean(F* data, size_t num)
    {
        F32 sum = 0.0001f;
        for (size_t i=0; i<num; ++i)
            sum = std::max(sum, std::abs(data[i]));
        F32 av = 0.0001f;
        for (size_t i=0; i<num; ++i)
            av += data[i] /= sum;
        av /= num;
        for (size_t i=0; i<num; ++i)
            data[i] -= av;
    }

    template <typename F>
    void normToOne(F* data, size_t num)
    {
        F32 mi = data[0], ma = mi;
        for (size_t i=0; i<num; ++i)
            mi = std::min(mi, data[i]),
            ma = std::max(ma, data[i]);
        ma -= mi;
        if (ma <= 0.f) ma = 1.f;
        for (size_t i=0; i<num; ++i)
            data[i] = (data[i]-mi) / ma;
    }
} // namespace

/** Convolve beat history into convolution */
void BeatDetector::Private::convolveHistory()
{
    //MO_ASSERT(numHistory > 4+minHist, "");
    MO_ASSERT(!beatHistory.empty(), "");
    MO_ASSERT(beatHistory.size() == convolution.size(), "");

    for (auto& f : candidates)
        f *= .99;

    convolve.setKernelSize(numHistory);

    for (size_t bin=0; bin<beat.size(); ++bin)
    {
        // put ringbuffer in order
        const size_t binofs = bin*numHistory;
        for (size_t i=0; i<numHistory; ++i)
        {
            const size_t mi = (i + beatHistoryWrite + 1) % numHistory;
            convolve.kernel()[i] = beatHistory[binofs + mi]
                    // * (F32(i+1) / numHistory) // some windowing
                    ;
        }

        // remove mean
        normAndMean(convolve.kernel(), numHistory);
        // auto-convolution
        convolve.convolveComplex(&convScratch[0], convolve.kernel(), numHistory);

        // convolve again
        for (size_t it=1; it<numConvs; ++it)
        {
            normAndMean(&convScratch[0], numHistory);
            for (size_t i=0; i<numHistory; ++i)
                convolve.kernel()[i] = convScratch[i];
            convolve.convolveComplex(&convScratch[0], convolve.kernel(), numHistory);
        }

        normAndMean(&convScratch[0], numHistory);

        // interpolate the convolution with previous frames
        for (size_t i=0; i<numHistory; ++i)
            convolution[binofs + i] +=
                    convAdaptSpeed * (convScratch[i] // *(1.f+4.f*i/numHistory)
                          - convolution[binofs+i]);
    }
}

void BeatDetector::Private::sampleCandidates1()
{
#if 0
    // do not check before this
    // to avoid self-similiarity
    const size_t minHist = 4;

    for (size_t bin = 0; bin < beat.size(); ++bin)
    {
        const size_t binofs = bin * numHistory;

        //MATH::real_fft(&convScratch[0], numHistory);
        //MATH::get_amplitude_phase(&convolution[binofs], numHistory);


        // get average convolution energy
        // (for auto-correlation messure below)
        F32 av = 0.f;
        for (size_t i=0; i<numHistory; ++i)
            av += convolution[binofs+i];
        av /= numHistory;

        // find matching distance 0 -> n
        // where the convolved signal is most similiar to itself
        size_t theBest = 0;
        F32 high = -1.f;
#if 0
        const size_t
                start = std::max(minHist, bestMatch[bin] / 2),
                end = std::max(start+10, std::min(numHistory, bestMatch[bin]*2+1)),
                num = end - start;
        for (size_t i=start; i<end; ++i)
        {
            F32 sum = 0.f;
            for (size_t j=0; j<num; ++j)
            {
            #if 0
                const F32 v1 = beatHistory[ofs + j],
                          v2 = beatHistory[ofs + j + i];
            #else
                const F32 v1 = convolution[ofs + j],
                          v2 = convolution[ofs + j + i];
            #endif
                sum += (v1-av)*(v2-av);
                        //v1 + v2;
                        //std::abs(v1 - v2);
            }
            if (sum > high || high < 0.f)
                high = sum, theBest = i;
        }
#else
        bool found = false;
        const size_t num = numHistory / 4;
        for (size_t i=0; i<10; ++i)
        {
            size_t k;
            if (i == 0)
                k = highestResponsePos[bin];
            if (i > 0 || k < minHist)
                k = minHist + rnd() % (numHistory - num - minHist);

            F32 sum = 0.f;
            for (size_t j=0; j<num; ++j)
            {
                const F32 v1 = convolution[binofs + j],
                          v2 = convolution[binofs + j + k];
                sum += (v1-av)*(v2-av);
            }
            if (i == 0)
                highestResponse[bin] = sum;
            else if (sum > highestResponse[bin])
                highestResponse[bin] = sum, theBest = k, found = true;
        }
        highestResponsePos[bin] = theBest;
#endif

        // add best match to candidate buffer
        //if (found)
            candidates[binofs + theBest] +=
                    //highestResponse[bin];
                    1.;
                    //average[bin];

        // find current best in candidates
        high = -1.;//candidates[ofs + bestMatch[bin]];
        theBest = 0;//bestMatch[bin];
        found = false;
        for (size_t i=minHist; i<numHistory-num; ++i)
        {
            F32 v = candidates[binofs + i];
            if (v >= high)
                high = v, theBest = i, found = true;
        }
        if (found)
            bestMatch[bin] = theBest;
        //else
        //    bestMatch[bin] = 0;

    }

    /*
    std::stringstream s;
    for (size_t i=0; i<bestMatch.size(); ++i)
        s << " " << highestResponsePos[i];
    MO_PRINT(s.str());
    */


    const size_t num = numHistory / 3;

    for (size_t ti = 8; ti < num; ++ti)
    //for (size_t ti = 10; ti < 11; ++ti)
    {

        for (size_t bin=0; bin<beat.size(); ++bin)
        {
            const size_t ofs = bin*numHistory;

            F32 beatAv = 0.0001;
            for (size_t i=0; i<numHistory; ++i)
                beatAv += beatHistory[ofs + i];
            beatAv /= numHistory;

            // generate beat-train
            F32 kernelSum = 0.0001f;
            for (size_t i=0; i<numHistory; ++i)
            {
            #if 1
                if (i % ti == 0)
                    convolve.kernel()[i] = 1.f;
                else
                    convolve.kernel()[i] = 0.f;
            #else
                convolve.kernel()[i] =
                        std::max(F32(0), std::sin(F32(i) * 3.14159265f * 2.f / ti));
            #endif
                convolve.kernel()[i] -= beatAv;
                kernelSum += convolve.kernel()[i];
            }
            kernelSum /= numHistory;


            // put ringbuffer in order
            for (size_t i=0; i<numHistory; ++i)
            {
                const size_t mi = (i + beatHistoryWrite + 1) % numHistory;
                convScratch[i] = beatHistory[ofs + mi]
                        // * (F32(i) / numHistory)
                        - beatAv
                        ;
            }

            convolve.convolveComplex(&convScratch[0], &convScratch[0], numHistory);
            //convolve.convolveComplex(&convScratch[0], &beatHistory[ofs], num);

            for (size_t i=0; i<numHistory; ++i)
                convolution[ofs + i] = convScratch[i];

            F32 sum = 0.f;
            for (size_t i=0; i<numHistory; ++i)
                sum += (convScratch[i]);

            candidates[ofs + ti] = sum / kernelSum;

        }

    }

    for (size_t bin=0; bin<beat.size(); ++bin)
    {
        const size_t ofs = bin*numHistory;

        F32 high = -1.f;
        size_t theBest = 0;
        for (size_t i=0; i<numHistory-num; ++i)
        {
            F32 v = candidates[ofs + i];
            if (v > high)
                high = v, theBest = i;
        }
        if (high >= 0.f)
            highest[bin] = F32(theBest)/numHistory;
    }
#endif
}

void BeatDetector::Private::sampleCandidates2()
{
    MO_ASSERT(numHistory <= numFreqResponses*2, "");

    for (size_t bin = 0; bin < beat.size(); ++bin)
    {
        F32 * response = &freqResponse[bin*numFreqResponses*2];
        const F32* conv = &convolution[bin*numHistory];

        // fill freq-response data with convolution buffer
        for (size_t i=0; i<numHistory; ++i)
            response[i] = conv[i];
        // and pad with zero
        for (size_t i=numHistory; i<numFreqResponses*2; ++i)
            response[i] = 0.f;
        // Note that the conv buffer already fades to zero
        // at the end :)

        // take the FT of the convolution to get
        // it's periodic frequencies
        MATH::real_fft(&response[0], numFreqResponses*2);
        MATH::get_amplitude_phase(&response[0], numFreqResponses*2);

        const size_t num = numFreqResponses;
        for (size_t i=0; i<num; ++i)
        {
            //response[i] = std::abs(response[i]);
            //response[i] += response[numHistory-1-i];
        }
        normToOne(&response[0], num);

        /*
        F32 peak = response[0];
        size_t i;
        for (i=1; i<num; ++i)
        {
            F32 v = response[i];
            if (v < peak)
                break;
            peak = v;
        }
        */
        F32 high = -10000.;
        size_t theBest = 0;
        for (size_t i=1; i<num; ++i)
        {
            const F32 v = response[i];
            if (v > high)
                high = v, theBest = i;
        }

        if (theBest != 0)
        {
            bestMatch[bin] = theBest;
        }
    }
}

/** "Quefrency ananlysis" */
void BeatDetector::Private::sampleCandidates3()
{
    MO_ASSERT(numHistory <= numFreqResponses*2, "");

    for (size_t bin = 0; bin < beat.size(); ++bin)
    {
        F32 * response = &freqResponse[bin*numFreqResponses*2];
        const F32* conv = &convolution[bin*numHistory];

        // fill freq-response data with convolution buffer
        for (size_t i=0; i<numHistory; ++i)
            response[i] = conv[i];
        // and pad with zero
        for (size_t i=numHistory; i<numFreqResponses*2; ++i)
            response[i] = 0.f;
        // Note that the conv buffer already fades to zero
        // at the end :)

        // take the FT of the convolution to get
        // it's periodic frequencies
        MATH::real_fft(&response[0], numFreqResponses*2);
        MATH::get_amplitude_phase(&response[0], numFreqResponses*2);

        const size_t num = numFreqResponses;
        for (size_t i=0; i<num; ++i)
        {
            response[i] = std::log(std::abs(response[i]));
            //response[i] += response[numHistory-1-i];
        }
        normToOne(&response[0], num);

        /*
        F32 peak = response[0];
        size_t i;
        for (i=1; i<num; ++i)
        {
            F32 v = response[i];
            if (v < peak)
                break;
            peak = v;
        }
        */
        F32 high = -10000.;
        size_t theBest = 0;
        for (size_t i=1; i<num; ++i)
        {
            const F32 v = response[i];
            if (v > high)
                high = v, theBest = i;
        }

        if (theBest != 0)
        {
            bestMatch[bin] = theBest;
        }
    }
}


/** Get the best (most confident value) from the bestMatch array */
void BeatDetector::Private::sampleBestMessure()
{
    MO_ASSERT(!bestMatch.empty(), "");
    MO_ASSERT(bestMatch.size() == matchCount.size(), "");

    for (size_t i = 0; i < bestMatch.size(); ++i)
        matchCount[i] = 0;

    for (size_t i = 0; i < bestMatch.size(); ++i)
    for (size_t j = i+1; j < bestMatch.size(); ++j)
    {
        if (bestMatch[i] == bestMatch[j])
        {
            ++matchCount[i];
            ++matchCount[j];
        }
    }

    std::map<size_t, size_t> map;
    for (size_t i = 0; i < bestMatch.size(); ++i)
        map.insert(std::make_pair(matchCount[i], bestMatch[i]));

    size_t j=0;
    for (auto& i : map)
        sortedBestMatch[j++] = i.second;

    /*
    F32 av = 0.;
    for (size_t i = 0; i < bestMatch.size(); ++i)
    {
        av += bestMatch[i];
    }
    av /= bestMatch.size();

    F32 av2
    for (size_t i = 0; i < bestMatch.size(); ++i)
    {
        F32 d = std::abs(F32(bestMatch[i]) - av);
        if (d <= 3.)
        {
            av
        }
    }
    */
}


void BeatDetector::Private::perform(const F32 *data, size_t count)
{
    // -- every fft.size() samples --

    rebuf.push(data, count);

    while (rebuf.pop(fft.buffer()))
    {
        if (p->numBins() > 1)
        {
            fft.fft();
            fft.toAmplitudeAndPhase();
        }

        sampleCurrent(); // for one bin this breaks down to average-over-signal
        sampleAverage();
        sampleBeat();
        sampleBeatHistory();
        convolveHistory();
        sampleCandidates2();
        sampleBestMessure();
    }
}



} // namespace AUDIO
} // namespace MO
