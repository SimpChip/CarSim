//
// Created by Mella on 27.11.2025.
//

#ifndef CARSIM_EVENTBUS_HPP
#define CARSIM_EVENTBUS_HPP

#include <functional>
#include <vector>

class EventBus {
public:
    using CallbackId = std::size_t;

    template<typename Event>
    static CallbackId subscribe(std::function<void(const Event&)> cb) {
        auto& vec = getSubscribers<Event>();
        CallbackId id = ++nextId_;
        vec.emplace_back(id, std::move(cb));
        return id;
    }

    template<typename Event>
    static void unsubscribe(CallbackId id) {
        auto& vec = getSubscribers<Event>();
        vec.erase(
            std::remove_if(vec.begin(), vec.end(),
                [id](auto& pair){ return pair.first == id; }),
            vec.end()
        );
    }

    template<typename Event>
    static void publish(const Event& e) {
        auto& vec = getSubscribers<Event>();
        for (auto& [id, cb] : vec) {
            cb(e);
        }
    }

private:
    template<typename Event>
    static std::vector<std::pair<CallbackId, std::function<void(const Event&)>>>& getSubscribers() {
        static std::vector<std::pair<CallbackId, std::function<void(const Event&)>>> subs;
        return subs;
    }

    static inline CallbackId nextId_{0};
};

#endif //CARSIM_EVENTBUS_HPP