
#ifndef CARSIM_EVENTBUS_HPP
#define CARSIM_EVENTBUS_HPP

#include <functional>
#include <vector>
#include <any>
#include <unordered_map>
#include <typeindex>


class EventBus {
public:
    using CallbackId = std::size_t;

    template<typename Event>
    CallbackId subscribe(std::function<void(const Event&)> cb) {
        auto& vec = subscribers_[std::type_index(typeid(Event))];
        CallbackId id = ++nextId_;
        vec.emplace_back(id, [cb](const void* e) {
            cb(*static_cast<const Event*>(e));
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
    void publish(const Event& e) {
        auto it = subscribers_.find(std::type_index(typeid(Event)));
        if (it != subscribers_.end()) {
            for (auto& [id, cb] : it->second) {
                cb(&e);
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