// © 2019 Joseph Cameron - All Rights Reserved

#ifndef GDK_MEMORY_SMART_HANDLE_H
#define GDK_MEMORY_SMART_HANDLE_H

#include <functional>
#include <memory>

namespace jfc::memory
{
    //TODO Move to different file
    template<class T> using unique_delete_ptr = std::unique_ptr<T, std::function<void(T *const)>>;

    /// \brief provides automatic based mechanism for cleanup of "handle types"
    /// a handle is responsible for some sort of manual state managemnet at end of life. e.g: OpenGL/AL buffer handles, LibClang nodes
    /// Deleter will only be called once: when final the copy of the smart handle goes out of scope.
    /// Implementation based on final_action found in the Cpp General Guidelines, extended to strongly associate the handle with the deletor (both are members) and to support copy operations in addition to moves
    /// analogous to stl shared_ptr
    template<class handle_type_param>
    class smart_handle final
    {
    public:
        using handle_type = handle_type_param;
        using deleter_type = std::function<void(const handle_type)>;

    private: 
        handle_type m_Handle;

        std::shared_ptr<deleter_type> m_pDeleter;

    public:
        //TODO: enable/disable ref depending on handle_type? if sizeof(handle_type) < sizeof(handle_type *), no ref.
        const handle_type &get() const
        {
            return m_Handle;
        }

        smart_handle(handle_type &&aValue, deleter_type &&aDeleter)
        : m_Handle(aValue)
        , m_pDeleter(std::make_shared<smart_handle<handle_type>::deleter_type>(aDeleter))
        {}

        ~smart_handle()
        {
            if (m_pDeleter.use_count() == 1) (*m_pDeleter)(m_Handle);
        }
    };

}

#endif

