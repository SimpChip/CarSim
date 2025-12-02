
#ifndef CARSIM_EVENTBUS_HPP
#define CARSIM_EVENTBUS_HPP

#include <functional>
#include <vector>
#include <any>
#include <ranges>
#include <unordered_map>
#include <typeindex>

// EventBus: Event bus implementation
// Initial structure generated with AI assistance, adapted for project needs

class EventBus {
public:
    using CallbackId = std::size_t;

    template<typename Event>
    CallbackId subscribe(std::function<void(const Event&)> callback) {
        auto& vec = subscribers_[std::type_index(typeid(Event))];
        CallbackId id = ++nextId_;
        vec.emplace_back(id, [callback](const void* event) {
            callback(*static_cast<const Event*>(event));
        });
        return id;
    }

    template<typename Event>
    void unsubscribe(CallbackId id) {
        auto& vec = subscribers_[std::type_index(typeid(Event))];
        vec.erase(
            std::remove_if(vec.begin(), vec.end(),
                [id](auto& pair){ return pair.first == id; }),
            vec.end()
        );
    }

    template<typename Event>
    void publish(const Event& event) {
        const auto it = subscribers_.find(std::type_index(typeid(Event)));
        if (it != subscribers_.end()) {
            for (auto &callback: it->second | std::views::values) {
                callback(&event);
            }
        }
    }

private:
    using AnyCallback = std::function<void(const void*)>;
    std::unordered_map<std::type_index,
        std::vector<std::pair<CallbackId, AnyCallback>>> subscribers_;
    CallbackId nextId_{0};
};

#endif //CARSIM_EVENTBUS_HPP