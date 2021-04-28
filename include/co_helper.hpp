#ifndef CO_HELPER_HPP
#define CO_HELPER_HPP
#include <coroutine>
#include <optional>

namespace co_helper
{
    using namespace std;
    /**
     * @brief Awaitable
     * 
     * @tparam T , returned form struct Awaitable 
     * @tparam Deriver , derived from Awaitable
     */

    template <typename T, typename Deriver>
    struct Awaitable
    {
        bool await_ready() { return false; }

        T await_resume() { return _value; }

        void await_suspend(std::coroutine_handle<> co_handle)
        {
            static_assert(std::is_base_of_v<Awaitable, Deriver>);

            _co_handle = co_handle;

            // call operator () of Deriver
            (*static_cast<Deriver *>(this))();
        }

        // std::coroutine_handle<> &handle() { return _co_handle; }

    protected:
        T _value;

        std::coroutine_handle<> _co_handle;
    };

    /**
     * @brief Task, for coroutine function 
     * 
     * @tparam T , returned from Task
     */
    template <typename T>
    struct Task
    {
        struct promise_type;
        using co_handle = std::coroutine_handle<promise_type>;
        co_handle _handle;

        Task(co_handle handle) : _handle(handle) {}
        Task(const Task &t) = delete;
        Task &operator=(const Task &t) = delete;

        Task(Task &&t) : _handle(t.handle) { t._handle = nullptr; }
        Task &operator=(Task &&t)
        {
            _handle = t._handle;
            t._handle = nullptr;
            return *this;
        }

        ~Task()
        {
            if (_handle)
            {
                _handle.destroy();
            }

            // cout << __func__ << " was called ." << endl;
        }

        T get()
        {            
            return _handle.promise()._value;
        }

        struct promise_type
        {
            //auto get_return_object() { return Task{}; }
            auto get_return_object() { return Task<T>{co_handle::from_promise(*this)}; }

            auto initial_suspend() { return std::suspend_never{}; }

            auto final_suspend()
            {
                // if return suspend_never the co_handle will be destroy automaticlly, otherwise we need destroy co_handle manually.
                // Here we get value from promise_type, so we must return suspend_always.
                return std::suspend_always{};
            }

            auto return_void() { return std::suspend_never{}; }

            void unhandled_exception() { rethrow_exception(current_exception()); }

            template <
                typename VALUE,
                typename = std::enable_if_t<std::is_convertible_v<VALUE &&, T>>>
            auto yield_value(VALUE &&v)
            {
                _value = v;

                return std::suspend_always{};
            }

            template <
                typename VALUE,
                typename = std::enable_if_t<std::is_convertible_v<VALUE &&, T>>>
            auto return_value(VALUE &&v)
            {
                _value = v;

                return std::suspend_never{};
            }

            T _value;
            //enable_if_t<!is_void<T>::value, T> _value;
        };
    };

    template <>
    struct Task<void>
    {
        struct promise_type;
        using co_handle = std::coroutine_handle<promise_type>;
        co_handle _handle;

        // ~Task() { cout << __func__ << " was called ." << endl; }

        struct promise_type
        {
            auto get_return_object() { return Task{co_handle::from_promise(*this)}; }

            auto initial_suspend() { return std::suspend_never{}; }

            auto final_suspend() { return std::suspend_never{}; }

            auto return_void() { return std::suspend_never{}; }

            void unhandled_exception()
            {
                rethrow_exception(current_exception());
            }
        };
    };

    template <std::movable T>
    class Generator
    {
    public:
        struct promise_type
        {
            Generator<T> get_return_object()
            {
                return Generator{Handle::from_promise(*this)};
            }
            static std::suspend_always initial_suspend() noexcept
            {
                return {};
            }
            static std::suspend_always final_suspend() noexcept
            {
                return {};
            }
            std::suspend_always yield_value(T value) noexcept
            {
                current_value = std::move(value);
                return {};
            }
            // Disallow co_await in generator coroutines.
            void await_transform() = delete;
            [[noreturn]] static void unhandled_exception()
            {
                throw;
            }

            std::optional<T> current_value;
        };

        using Handle = std::coroutine_handle<promise_type>;

        explicit Generator(const Handle coroutine) : m_coroutine{coroutine}
        {
        }

        Generator() = default;
        ~Generator()
        {
            if (m_coroutine)
            {
                m_coroutine.destroy();
            }
        }

        Generator(const Generator &) = delete;
        Generator &operator=(const Generator &) = delete;

        Generator(Generator &&other) noexcept : m_coroutine{other.m_coroutine}
        {
            other.m_coroutine = {};
        }

        Generator &operator=(Generator &&other) noexcept
        {
            if (this != &other)
            {
                if (m_coroutine)
                {
                    m_coroutine.destroy();
                }
                m_coroutine = other.m_coroutine;
                other.m_coroutine = {};
            }
            return *this;
        }

        // Range-based for loop support.
        class Iter
        {
        public:
            void operator++()
            {
                m_coroutine.resume();
            }
            const T &operator*() const
            {
                return *m_coroutine.promise().current_value;
            }
            bool operator==(std::default_sentinel_t) const
            {
                return !m_coroutine || m_coroutine.done();
            }

            explicit Iter(const Handle coroutine) : m_coroutine{coroutine}
            {
            }

        private:
            Handle m_coroutine;
        };

        Iter begin()
        {
            if (m_coroutine)
            {
                m_coroutine.resume();
            }
            return Iter{m_coroutine};
        }
        std::default_sentinel_t end()
        {
            return {};
        }

    private:
        Handle m_coroutine;
    };
}

#endif