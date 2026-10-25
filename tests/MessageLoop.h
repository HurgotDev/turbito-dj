#pragma once

#include <juce_events/juce_events.h>

#if JUCE_MAC
#include <CoreFoundation/CoreFoundation.h>
#endif

#include <functional>

// Runs the message loop until done() or the timeout. JUCE_MODAL_LOOPS_PERMITTED is off
// (no runDispatchLoopUntil), and on macOS runDispatchLoop returns at once outside an
// app bundle, so there the main CFRunLoop (JUCE's message queue) is pumped directly.
inline void runMessageLoopUntil(const std::function<bool()>& done, int timeoutMs)
{
    const auto end = juce::Time::getMillisecondCounter() + static_cast<juce::uint32>(timeoutMs);
#if JUCE_MAC
    while (! done() && juce::Time::getMillisecondCounter() < end)
        CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.01, true);
#else
    struct Poll final : juce::Timer
    {
        std::function<void()> check;
        void timerCallback() override { check(); }
    } poll;

    poll.check = [&]
    {
        if (done() || juce::Time::getMillisecondCounter() >= end)
            juce::MessageManager::getInstance()->stopDispatchLoop();
    };
    poll.startTimer(10);
    juce::MessageManager::getInstance()->runDispatchLoop();
#endif
}
