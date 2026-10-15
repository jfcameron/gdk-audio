// © Joseph Cameron - All Rights Reserved

#include <gdk/audio/impl_efx.h>

#include <AL/alc.h>

#include <optional>

namespace gdk::audio {
    namespace {
        std::optional<const efx_functions *> g_Loaded;

        efx_functions g_Functions;

        template <typename function_type>
        [[nodiscard]] bool load(function_type &aInto, const char *const apName) {
            aInto = reinterpret_cast<function_type>(alGetProcAddress(apName));

            return aInto != nullptr;
        }
    }

    const efx_functions *efx() {
        if (g_Loaded) return *g_Loaded;

        auto *const pContext = alcGetCurrentContext();
        auto *const pDevice = pContext ? alcGetContextsDevice(pContext) : nullptr;

        if (!pDevice || !alcIsExtensionPresent(pDevice, "ALC_EXT_EFX")) {
            g_Loaded = nullptr;

            return nullptr;
        }

        const bool all = load(g_Functions.alGenEffects, "alGenEffects")
            && load(g_Functions.alDeleteEffects, "alDeleteEffects")
            && load(g_Functions.alEffecti, "alEffecti")
            && load(g_Functions.alEffectf, "alEffectf")
            && load(g_Functions.alGenFilters, "alGenFilters")
            && load(g_Functions.alDeleteFilters, "alDeleteFilters")
            && load(g_Functions.alFilteri, "alFilteri")
            && load(g_Functions.alFilterf, "alFilterf")
            && load(g_Functions.alGenAuxiliaryEffectSlots, "alGenAuxiliaryEffectSlots")
            && load(g_Functions.alDeleteAuxiliaryEffectSlots, "alDeleteAuxiliaryEffectSlots")
            && load(g_Functions.alAuxiliaryEffectSloti, "alAuxiliaryEffectSloti");

        g_Loaded = all ? &g_Functions : nullptr;

        return *g_Loaded;
    }

    void forget_efx() {
        g_Loaded.reset();
    }
}
