//
// Created by bison on 20-02-25.
//

#ifndef SANDBOX_EVENTBUS_H
#define SANDBOX_EVENTBUS_H

#include <iostream>
#include <functional>
#include <unordered_map>
#include <vector>
#include <memory>
#include <typeindex>
#include <algorithm>

class EventBus {
public:
    using HandlerID = std::size_t;
    using Priority = int;

    template<typename EventType>
    using Handler = std::function<void(const EventType&)>;

    template<typename EventType>
    HandlerID subscribe(Handler<EventType> handler, Priority priority = 0) {
        auto& handlers = handlers_[typeid(EventType)];
        HandlerID id = nextHandlerID_++;
        handlers.push_back(HandlerEntry{id, priority, [handler](void* event) {
            handler(*static_cast<EventType*>(event));
        }});

        // Sort handlers by priority (higher first)
        std::sort(handlers.begin(), handlers.end(), EventBus::comparePriority);

        return id;
    }

    template<typename EventType>
    void unsubscribe(HandlerID id) {
        auto it = handlers_.find(typeid(EventType));
        if (it != handlers_.end()) {
            std::vector<HandlerEntry>& vec = it->second;
            vec.erase(std::remove_if(vec.begin(), vec.end(), CompareID(id)), vec.end());
        }
    }

    template<typename EventType>
    void publish(const EventType& event) {
        auto it = handlers_.find(typeid(EventType));
        if (it != handlers_.end()) {
            for (size_t i = 0; i < it->second.size(); ++i) {
                it->second[i].handler((void*)&event);
            }
        }
    }

private:
    struct HandlerEntry {
        HandlerID id;
        Priority priority;
        std::function<void(void*)> handler;
    };

    struct CompareID {
        HandlerID id;
        explicit CompareID(HandlerID id) : id(id) {}
        bool operator()(const HandlerEntry& entry) const {
            return entry.id == id;
        }
    };

    static bool comparePriority(const HandlerEntry& a, const HandlerEntry& b) {
        return a.priority > b.priority;
    }

    std::unordered_map<std::type_index, std::vector<HandlerEntry>> handlers_;
    HandlerID nextHandlerID_ = 0;
};

#endif //SANDBOX_EVENTBUS_H
