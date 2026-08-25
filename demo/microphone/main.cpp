// © Joseph Cameron - All Rights Reserved

#include <gdk/audio/context.h>
#include <gdk/audio/emitter.h>
#include <gdk/audio/exception.h>
#include <gdk/audio/microphone.h>
#include <gdk/audio/openal_context.h>
#include <gdk/audio/openal_policy.h>
#include <gdk/audio/scene.h>
#include <gdk/audio/sound.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <span>

using namespace gdk;

namespace {
    constexpr std::size_t SAMPLE_RATE = 22050;
    constexpr double RECORD_SECONDS = 4.0;
    constexpr double METER_SECONDS = 4.0;
    constexpr auto POLL_INTERVAL = std::chrono::milliseconds(16);

    void print_devices(const audio::context &aContext) {
        const auto names = aContext.capture_device_names();

        std::cout << "capture devices:\n";

        if (names.empty()) std::cout << "  (none -- this machine offers no microphone)\n";

        for (const auto &name : names) std::cout << "  " << name << "\n";
    }

    [[nodiscard]] audio::microphone::request recording_request(std::string aDeviceName) {
        audio::microphone::request request{
            .device_name = std::move(aDeviceName),
            .sample_rate = SAMPLE_RATE,
            .channel_count = 1,   
            .format = audio::pcm_format::signed_16,

            .capacity_in_frames = static_cast<std::size_t>(SAMPLE_RATE * RECORD_SECONDS),
            .drop_oldest_when_full = false,
        };

        return request;
    }

    [[nodiscard]] double peak_of(const std::vector<short> &aFrames, const std::size_t aCount) {
        short loudest = 0;

        for (std::size_t i = 0; i < aCount; ++i) loudest = std::max(loudest,
            static_cast<short>(std::abs(aFrames[i])));

        return static_cast<double>(loudest) / 32767.0;
    }

    void draw_meter(const double aPeak) {
        const auto filled = static_cast<int>(aPeak * 40.0);

        std::cout << "\r  [" << std::string(filled, '#') << std::string(40 - filled, ' ') << "] "
            << std::flush;
    }
}

int main() {
    try {
        auto pContext = audio::openal_context::make();

        print_devices(*pContext);

        if (pContext->capture_device_names().empty()) {
            std::cout << "nothing to record from.\n";

            return EXIT_SUCCESS;
        }

        auto pMicrophone = pContext->make_microphone(recording_request({}));

        std::cout << "\nrecording " << RECORD_SECONDS << "s from \"" << pMicrophone->device_name()
            << "\" at " << pMicrophone->sample_rate() << "Hz -- say something\n";

        pMicrophone->start();

        while (pMicrophone->capturing()) {
            std::this_thread::sleep_for(POLL_INTERVAL);

            pMicrophone->update();
        }

        auto pRecording = pMicrophone->take();

        std::cout << "captured " << pRecording->duration() << "s\n";

        auto pScene = pContext->make_scene();

        auto pEmitter = pScene->make_emitter(pRecording);

        std::cout << "playing it back\n";

        pEmitter->play();

        while (pEmitter->is_playing()) {
            pScene->update();

            std::this_thread::sleep_for(POLL_INTERVAL);
        }

        auto streaming = recording_request({});
        streaming.capacity_in_frames = SAMPLE_RATE / 2;
        streaming.drop_oldest_when_full = true;

        auto pMeter = pContext->make_microphone(streaming);

        std::cout << "\nlevel meter for " << METER_SECONDS << "s (streaming mode, nothing kept)\n";

        pMeter->start();

        std::vector<short> block(streaming.capacity_in_frames);

        const auto began = std::chrono::steady_clock::now();

        double lastPeak = -1;

        while (std::chrono::duration<double>(std::chrono::steady_clock::now() - began).count()
            < METER_SECONDS) {
            std::this_thread::sleep_for(POLL_INTERVAL);

            pMeter->update();

            if (const auto frames = pMeter->read(std::as_writable_bytes(std::span(block)))) {
                const auto peak = peak_of(block, frames);

                if (std::abs(peak - lastPeak) > 0.02) {
                    draw_meter(peak);

                    lastPeak = peak;
                }
            }
        }

        pMeter->stop();

        std::cout << "\ndone. " << pMeter->dropped_frame_count()
            << " frames dropped by falling behind, " << pMeter->available_in_frames()
            << " still buffered at exit\n";

        return EXIT_SUCCESS;
    }
    catch (const audio::exception &e) {
        std::cerr << "gdk-audio: " << e.what() << "\n";

        return EXIT_FAILURE;
    }
}
