// © 2019 Joseph Cameron - All Rights Reserved

#include <gdk/audio/context.h>

#include <gdk/audio/openal_context.h>

namespace gdk::audio
{
    std::unique_ptr<context> context::make(implementation impl)
    {
        switch(impl)
        {
            case implementation::openal: return std::unique_ptr<context>(new openal_context());
        }
        
        throw std::invalid_argument("Could not create a context of requested type");
    }
}

