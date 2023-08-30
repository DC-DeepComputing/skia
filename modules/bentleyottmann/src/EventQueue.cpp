// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.


#include "modules/bentleyottmann/include/EventQueue.h"

namespace bentleyottmann {

// -- EventQueue -----------------------------------------------------------------------------------
std::optional<EventQueue> EventQueue::Make(SkSpan<const Segment> segments) {
    Queue queue;

    int32_t left   = Point::Largest().x,
            top    = Point::Largest().y,
            right  = Point::Smallest().x,
            bottom = Point::Smallest().y;

    for(const Segment& s : segments) {
        auto [l, t, r, b] = s.bounds();
        left   = std::min(l, left);
        top    = std::min(t, top);
        right  = std::max(r, right);
        bottom = std::max(b, bottom);

        queue.insert(Event{s.upper(), Upper{s}});
    }

    // If min max difference is too large, fail.
    if (Point::DifferenceTooBig(Point{left, top}, Point{right, bottom})) {
        return std::nullopt;
    }

    return EventQueue{std::move(queue)};
}

EventQueue::EventQueue(EventQueue::Queue&& queue) : fQueue{std::move(queue)} { }

void EventQueue::add(const Event& event) {
    // New events must be up stream from the current event.
    if (!fQueue.empty()) {
        SkASSERT(fQueue.begin()->where < event.where);
    }

    auto [_, success] = fQueue.insert(event);
    SkASSERT_RELEASE(success);
}

void EventQueue::addCrossing(const bentleyottmann::Event& e) {
    SkASSERT(std::holds_alternative<Cross>(e.type));
    fCrossings.insert(e);
}

bool EventQueue::hasMoreEvents() const {
    return !fQueue.empty();
}

Event EventQueue::nextEvent() {
    SkASSERT(this->hasMoreEvents());

    auto firstElement = fQueue.begin();

    // Extract event at the beginning of the queue.
    Event event = *firstElement;

    // Remove the beginning element from the queue.
    fQueue.erase(firstElement);

    return event;
}

template<class... Ts>
struct Visitor : Ts... { using Ts::operator()...; };
template<class... Ts>
Visitor(Ts...) -> Visitor<Ts...>;

void EventQueue::handleNextEventPoint(bentleyottmann::EventQueue::EventHandler* handler) {
    // Clear temp segment buffers.
    fDeleteSet.clear();
    fInsertionSet.clear();

    // Set up the visitors for the different event types.
    auto handleLower = [](const Lower& l) {};

    auto handleCross = [this](const Cross& c) {
        fDeleteSet.insert(c.s0);
        fDeleteSet.insert(c.s1);
        fInsertionSet.insert(c.s0);
        fInsertionSet.insert(c.s1);
    };

    auto handleUpper = [this](const Upper& u) {
        fInsertionSet.insert(u.s);
        // Add the delete event for the inserted segment. Make sure we are not adding more events
        // on this eventPoint.
        SkASSERT(u.s.lower() != u.s.upper());
        this->add(Event{u.s.lower(), Lower{}});
    };

    Visitor visitor{handleLower, handleCross, handleUpper};

    auto cursor = fQueue.begin();
    Event event = *cursor;
    const Point eventPoint = event.where;

    do {
        std::visit(visitor, event.type);
        fQueue.erase(cursor);
        if (fQueue.empty()) {
            break;
        }
        cursor = fQueue.begin();
        event = *cursor;
    } while(event.where == eventPoint);

    handler->handle(eventPoint, fDeleteSet, fInsertionSet, this);
}

bool EventQueue::OrderBySlope::operator()(const bentleyottmann::Segment& s0,
                                          const bentleyottmann::Segment& s1) const {
    return compareSlopes(s0, s1) < 0;
}
}  // namespace bentleyottmann
