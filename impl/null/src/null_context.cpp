// © Joseph Cameron - All Rights Reserved

#include <gdk/audio/null_context.h>

#include <gdk/audio/exception.h>

#include <algorithm>

using namespace gdk::audio;

std::shared_ptr<null_context> null_context::make() {
    return std::shared_ptr<null_context>(new null_context());
}

scene_shared_ptr_type null_context::make_scene() {
    ++mSceneCount;

    return std::make_shared<null_scene>();
}

std::size_t null_context::scene_count() const { return mSceneCount; }

std::vector<std::string> null_context::capture_device_names() const { return mCaptureDevices; }

void null_context::set_capture_device_names(std::vector<std::string> aNames) {
    mCaptureDevices = std::move(aNames);
}

microphone_shared_ptr_type null_context::make_microphone(const microphone::request &aRequest) {
    if (!aRequest.device_name.empty()
        && std::find(mCaptureDevices.begin(), mCaptureDevices.end(), aRequest.device_name)
            == mCaptureDevices.end())
        throw exception("no capture device named \"" + aRequest.device_name + "\"");

    return std::make_shared<null_microphone>(aRequest);
}
