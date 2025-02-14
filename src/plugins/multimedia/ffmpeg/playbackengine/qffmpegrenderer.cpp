// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "playbackengine/qffmpegrenderer_p.h"
#include <qloggingcategory.h>

QT_BEGIN_NAMESPACE

namespace QFFmpeg {

static Q_LOGGING_CATEGORY(qLcRenderer, "qt.multimedia.ffmpeg.renderer");

Renderer::Renderer(const TimeController &tc, const std::chrono::microseconds &seekPosTimeOffset)
    : m_timeController(tc),
      m_lastPosition(tc.currentPosition()),
      m_seekPos(tc.currentPosition(-seekPosTimeOffset))
{
}

void Renderer::syncSoft(TimePoint tp, qint64 trackTime)
{
    QMetaObject::invokeMethod(this, [this, tp, trackTime]() {
        m_timeController.syncSoft(tp, trackTime);
        scheduleNextStep(true);
    });
}

qint64 Renderer::seekPosition() const
{
    return m_seekPos;
}

qint64 Renderer::lastPosition() const
{
    return m_lastPosition;
}

void Renderer::setPlaybackRate(float rate)
{
    QMetaObject::invokeMethod(this, [this, rate]() {
        m_timeController.setPlaybackRate(rate);
        onPlaybackRateChanged();
        scheduleNextStep();
    });
}

void Renderer::doForceStep()
{
    if (m_isStepForced.testAndSetOrdered(false, true))
        QMetaObject::invokeMethod(this, [this]() {
            // maybe set m_forceStepMaxPos

            if (isAtEnd()) {
                setForceStepDone();
            }
            else {
                m_explicitNextFrameTime = Clock::now();
                scheduleNextStep();
            }
        });
}

bool Renderer::isStepForced() const
{
    return m_isStepForced;
}

void Renderer::onFinalFrameReceived()
{
    render({});
}

void Renderer::render(Frame frame)
{
    const auto isFrameOutdated = frame.isValid() && frame.absoluteEnd() < seekPosition();

    if (isFrameOutdated) {
        qCDebug(qLcRenderer) << "frame outdated! absEnd:" << frame.absoluteEnd() << "absPts"
                             << frame.absolutePts() << "seekPos:" << seekPosition();
        emit frameProcessed(frame);
        return;
    }

    m_frames.enqueue(frame);

    if (m_frames.size() == 1)
        scheduleNextStep();
}

void Renderer::onPauseChanged()
{
    m_timeController.setPaused(isPaused());
    PlaybackEngineObject::onPauseChanged();
}

bool Renderer::canDoNextStep() const
{
    return !m_frames.empty() && (m_isStepForced || PlaybackEngineObject::canDoNextStep());
}

float Renderer::playbackRate() const
{
    return m_timeController.playbackRate();
}

int Renderer::timerInterval() const
{
    auto frame = !m_frames.empty() ? m_frames.front() : Frame();
    if (frame.isValid()) {
        using namespace std::chrono;

        const auto nextTime = m_explicitNextFrameTime
                ? *m_explicitNextFrameTime
                : m_timeController.timeFromPosition(frame.absolutePts());

        const auto delay = nextTime - Clock::now();
        return std::max(0, static_cast<int>(duration_cast<milliseconds>(delay).count()));
    }

    return 0;
}

bool Renderer::setForceStepDone()
{
    if (!m_isStepForced.testAndSetOrdered(true, false))
        return false;

    m_explicitNextFrameTime.reset();
    emit forceStepDone();
    return true;
}

void Renderer::doNextStep()
{
    auto frame = m_frames.front();

    if (setForceStepDone()) {
        // if (frame.isValid() && frame.pts() > m_forceStepMaxPos) {
        //    scheduleNextStep(false);
        //    return;
        // }
    }

    const auto result = renderInternal(frame);

    if (result.done) {
        m_explicitNextFrameTime.reset();
        m_frames.dequeue();

        if (frame.isValid()) {
            m_lastPosition.storeRelease(std::max(frame.absolutePts(), lastPosition()));
            m_seekPos.storeRelaxed(frame.absoluteEnd());

            const auto loopIndex = frame.loopOffset().index;
            if (m_loopIndex < loopIndex) {
                m_loopIndex = loopIndex;
                emit loopChanged(id(), frame.loopOffset().pos, m_loopIndex);
            }

            emit frameProcessed(frame);
        }
    } else {
        m_explicitNextFrameTime = Clock::now() + result.recheckInterval;
    }

    setAtEnd(result.done && !frame.isValid());

    scheduleNextStep(false);
}

std::chrono::microseconds Renderer::frameDelay(const Frame &frame) const
{
    return std::chrono::duration_cast<std::chrono::microseconds>(
            Clock::now() - m_timeController.timeFromPosition(frame.absolutePts()));
}

void Renderer::changeRendererTime(std::chrono::microseconds offset)
{
    const auto now = Clock::now();
    const auto pos = m_timeController.positionFromTime(now);
    m_timeController.sync(now + offset, pos);
    emit synchronized(id(), now + offset, pos);
}

} // namespace QFFmpeg

QT_END_NAMESPACE

#include "moc_qffmpegrenderer_p.cpp"
