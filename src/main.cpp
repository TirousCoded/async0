

#include <coroutine>
#include <optional>

#include "general.h"


// TODO: I was learning coroutines. Gonna have to actually build the async/await
//       system at some point, lol.


template<std::movable T>
class Generator final {
public:
    struct promise_type final {
        std::optional<T> lastValue;


        Generator get_return_object() {
            return Generator(std::coroutine_handle<promise_type>::from_promise(*this));
        }

        // Not 100% sure what below do.
        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        // Apparently disallows co_await in generator coroutines.
        void await_transform() = delete;

        // This arises on co_yield.
        std::suspend_always yield_value(T value) noexcept {
            lastValue = std::move(value);
            return {}; // Suspends execution and returns control to caller.
        }
        // Handles the co_return expression (void in this case.)
        void return_void() noexcept {}
        // Handles the co_return expression.
        //void return_value(T value) noexcept {
        //    lastValue = std::move(value);
        //}

        // Handles unhandled exceptions thrown inside the coroutine.
        void unhandled_exception() { std::terminate(); }
    };
    

    std::coroutine_handle<promise_type> handle = nullptr;


    Generator() = default;
    explicit Generator(std::coroutine_handle<promise_type> h) :
        handle(h) {
    }
    ~Generator() noexcept {
        _drop_handle();
    }
    Generator(const Generator&) = delete;
    Generator(Generator&& other) noexcept :
        Generator() {
        std::swap(handle, other.handle);
    }
    Generator& operator=(const Generator&) = delete;
    Generator& operator=(Generator&& other) noexcept {
        if (this != &other) {
            _drop_handle();
            std::swap(handle, other.handle);
        }
        return *this;
    }

    
    // Doesn't advance generator.
    std::optional<T> peek() const {
        return handle.promise().lastValue;
    }
    // Advances generator.
    std::optional<T> pull() {
        if (handle && !handle.done()) {
            handle.resume();
        }
        return peek();
    }


private:
    void _drop_handle() {
        if (handle) {
            handle.destroy();
            handle = nullptr;
        }
    }
};

Generator<int32_t> count(int32_t first) {
    for (int32_t i = first;; i++) {
        co_yield i;
    }
}

int32_t main(int32_t argc, char** argv) {

    auto a = count(5);

    while (auto v = a.pull()) {
        println(" > {}", *v);
        if (v == 14) {
            break;
        }
    }

	return 0;
}


#if 0

#include <iostream>
#include <coroutine>
#include <exception>

// 1. Define the Coroutine Return Type
template<std::movable T>
struct Generator {
    // 2. Define the nested promise_type required by the compiler
    struct promise_type {
        T current_value;

        // Called when the coroutine is first invoked
        Generator get_return_object() {
            return Generator{ std::coroutine_handle<promise_type>::from_promise(*this) };
        }

        // coroutine starts suspended (lazily)
        std::suspend_always initial_suspend() noexcept { return {}; }

        // coroutine cleans up automatically when finished
        std::suspend_always final_suspend() noexcept { return {}; }

        // Handles the 'co_yield' expression
        std::suspend_always yield_value(T value) noexcept {
            current_value = std::move(value);
            return {}; // Suspends execution and returns control to caller
        }

        // Handles the 'co_return' expression (void in this case)
        void return_void() noexcept {}

        // Handles unhandled exceptions thrown inside the coroutine
        void unhandled_exception() { std::terminate(); }
    };

    // 3. Manage the coroutine lifecycle using its handle
    std::coroutine_handle<promise_type> handle;

    Generator(std::coroutine_handle<promise_type> h) : handle(h) {}

    ~Generator() {
        if (handle) handle.destroy();
    }

    // Move-only semantics to prevent accidental copying of the handle
    Generator(const Generator&) = delete;
    Generator& operator=(const Generator&) = delete;
    Generator(Generator&& other) noexcept : handle(other.handle) { other.handle = nullptr; }
    Generator& operator=(Generator&& other) noexcept {
        if (this != &other) {
            if (handle) handle.destroy();
            handle = other.handle;
            other.handle = nullptr;
        }
        return *this;
    }

    // Resumes the coroutine and returns true if there is a new value available
    bool next() {
        if (!handle || handle.done()) return false;
        handle.resume();
        return !handle.done();
    }

    // Fetches the current value yielded by the coroutine
    T value() const {
        return handle.promise().current_value;
    }
};

// 4. The Coroutine Function
Generator<int> count_up_to(int max_value) {
    for (int i = 1; i <= max_value; ++i) {
        co_yield i; // Suspends execution and passes 'i' back to the caller
    }
}

Generator<char> ascii_chars(char first, char last) {
    if (first > last) {
        std::swap(first, last);
    }
    for (char i = first; i <= last; i++) {
        co_yield i;
    }
}

// 5. Execution
int main() {
    std::cout << "Starting coroutine...\n";

    Generator gen = count_up_to(5);

    while (gen.next()) {
        std::cout << "Received value: " << gen.value() << "\n";
    }

    std::cout << "Coroutine finished.\n";

    endln();
    auto g_fn = [](char first, char last) -> Generator<char> {
        if (first > last) {
            std::swap(first, last);
        }
        for (char i = first; i <= last; i++) {
            co_yield i;
        }
        };
    //Generator g = ascii_chars('a', 'f');
    auto g = g_fn('a', 'f');
    while (g.next()) {
        println(" -> {}", g.value());
    }

    return 0;
}

#endif

#if 0

#include <iostream>
#include <coroutine>
#include <utility>

// 1. Define the Return/Object Type that the caller interacts with
struct SimpleGenerator {

    // 2. Define the mandatory promise_type inside your return object
    struct promise_type {
        int current_value;

        // Called when the coroutine is first invoked to create the return object
        SimpleGenerator get_return_object() {
            return SimpleGenerator{ std::coroutine_handle<promise_type>::from_promise(*this) };
        }

        // Controls if the coroutine pauses before executing any of its body
        std::suspend_always initial_suspend() noexcept { return {}; }

        // Controls if the coroutine pauses after it completes its execution
        std::suspend_always final_suspend() noexcept { return {}; }

        // Called when co_yield is invoked inside the coroutine
        std::suspend_always yield_value(int value) noexcept {
            current_value = value;
            return {}; // Suspend the coroutine and return control to the caller
        }

        // Called when co_return is invoked (or when the function ends)
        void return_void() noexcept {}

        // Mandated strategy for dealing with exceptions leaking out of the coroutine
        void unhandled_exception() { std::terminate(); }
    };

    // 3. Manage the lifetime of the coroutine handle
    std::coroutine_handle<promise_type> handle;

    explicit SimpleGenerator(std::coroutine_handle<promise_type> h) : handle(h) {}

    ~SimpleGenerator() {
        if (handle) handle.destroy(); // Free the underlying coroutine frame heap allocation
    }

    // Coroutines are traditionally move-only to avoid duplicate handle ownership
    SimpleGenerator(const SimpleGenerator&) = delete;
    SimpleGenerator& operator=(const SimpleGenerator&) = delete;

    SimpleGenerator(SimpleGenerator&& other) noexcept : handle(other.handle) {
        other.handle = nullptr;
    }
    SimpleGenerator& operator=(SimpleGenerator&& other) noexcept {
        if (this != &other) {
            if (handle) handle.destroy();
            handle = other.handle;
            other.handle = nullptr;
        }
        return *this;
    }

    // Public API to drive the coroutine from the outside
    bool move_next() {
        if (!handle || handle.done()) return false;
        handle.resume(); // Resume execution up to the next co_yield or co_return
        return !handle.done();
    }

    int current_value() const {
        return handle.promise().current_value;
    }
};

// 4. Write the Coroutine function itself
SimpleGenerator count_up_to(int max_value) {
    for (int i = 1; i <= max_value; ++i) {
        co_yield i; // Pauses here, saving state, and gives control back to main()
    }
}

int main() {
    std::cout << "Initializing coroutine...\n";
    // Calling the function initializes the coroutine frame but doesn't run the loop yet
    auto generator = count_up_to(5);

    std::cout << "Starting loop:\n";
    while (generator.move_next()) {
        std::cout << "Received value: " << generator.current_value() << "\n";
    }

    std::cout << "Coroutine finished.\n";
    return 0;
}

#endif

#if 0

#include <iostream>
#include <coroutine>
#include <utility>
#include <thread>
#include <chrono>

// ==========================================
// 1. Define the Awaitable Type
// ==========================================
// This acts as the placeholder for a value that will arrive in the future.
struct SimpleTaskAwaitable {
    std::string result_data;
    bool is_ready = false;
    std::coroutine_handle<> suspended_coroutine = nullptr;

    // Called instantly when `co_await` is used. 
    // Return false to pause execution and yield control.
    bool await_ready() const noexcept {
        return is_ready;
    }

    // Called right after suspension. Passes the parent coroutine's handle
    // so it can be awoken/resumed later when work completes.
    void await_suspend(std::coroutine_handle<> handle) noexcept {
        suspended_coroutine = handle;

        // Simulate kicking off an asynchronous background thread operation
        std::thread([this]() {
            std::this_thread::sleep_for(std::chrono::seconds(2)); // Fake latency
            this->result_data = "Data from background thread!";
            this->is_ready = true;

            // Wake up the suspended coroutine
            if (this->suspended_coroutine) {
                this->suspended_coroutine.resume();
            }
            }).detach();
    }

    // Called right as the coroutine un-suspends. 
    // Whatever this returns becomes the evaluation value of the `co_await` expression.
    std::string await_resume() noexcept {
        return result_data;
    }
};

// ==========================================
// 2. Define the Return Object (The Coroutine Type)
// ==========================================
struct AsyncTask {
    struct promise_type {
        AsyncTask get_return_object() {
            return AsyncTask{ std::coroutine_handle<promise_type>::from_promise(*this) };
        }
        std::suspend_never initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() noexcept {}
        void unhandled_exception() { std::terminate(); }
    };

    std::coroutine_handle<promise_type> handle;
    explicit AsyncTask(std::coroutine_handle<promise_type> h) : handle(h) {}

    ~AsyncTask() {
        if (handle) handle.destroy();
    }

    AsyncTask(const AsyncTask&) = delete;
    AsyncTask& operator=(const AsyncTask&) = delete;
    AsyncTask(AsyncTask&& other) noexcept : handle(other.handle) { other.handle = nullptr; }
};

// ==========================================
// 3. Write and Run the Coroutine
// ==========================================
AsyncTask run_async_workflow() {
    std::cout << "[Coroutine] Starting workflow...\n";

    SimpleTaskAwaitable background_job;

    std::cout << "[Coroutine] Pausing and awaiting background job...\n";
    // 1. Suspends run_async_workflow()
    // 2. Returns execution control directly back to main()
    // 3. Resumes right here when background_job calls handle.resume()
    std::string result = co_await background_job;

    std::cout << "[Coroutine] Resumed! Result received: " << result << "\n";
}

int main() {
    std::cout << "[Main] Calling coroutine function...\n";
    run_async_workflow();

    std::cout << "[Main] Coroutine suspended. Main is completely free to do other work!\n";
    for (int i = 0; i < 4; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(600));
        std::cout << "[Main] Doing concurrent work in main thread...\n";
    }

    std::cout << "[Main] Workflow finished.\n";
    return 0;
}

#endif

