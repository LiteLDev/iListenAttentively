#pragma once
#include <ll/api/event/Cancellable.h>
#include <ll/api/event/EventBus.h>
#include <type_traits>
#include <utility>

namespace ila::inline utils::event_utils {

template <typename Callback, typename EventType>
concept InvocableForEvent = requires(EventType& e) {
    { std::declval<Callback>()(e) };
} || requires {
    { std::declval<Callback>()() };
};

template <typename E>
concept ValidEventType =
    std::derived_from<std::remove_cvref_t<E>, ll::event::Event> && std::is_final_v<std::remove_cvref_t<E>>;

template <typename Cb, typename EventType>
concept ValidCallbackOrNull =
    std::is_same_v<std::remove_cvref_t<Cb>, std::nullptr_t> || InvocableForEvent<Cb, EventType>;

template <typename Ev>
concept ValidEventOrNull = std::is_same_v<std::remove_cvref_t<Ev>, std::nullptr_t> || ValidEventType<Ev>;

template <typename E>
concept CancellableEvent =
    ValidEventType<E>
    && ll::traits::is_derived_from_specialization_of_v<std::remove_cvref_t<E>, ll::event::Cancellable>;

template <typename... Callbacks>
constexpr bool are_callbacks_noexcept_v =
    (...
     && (std::is_same_v<std::remove_cvref_t<Callbacks>, std::nullptr_t> || std::is_nothrow_invocable_v<Callbacks>
         || std::is_nothrow_invocable_v<Callbacks, std::remove_reference_t<Callbacks>&>));

inline static ll::event::EventBus& getEventBus() {
    static auto& vsEventBus = ll::event::EventBus::getInstance();
    return vsEventBus;
}

template <
    typename EventType,
    typename BeforeCb     = std::nullptr_t,
    typename AfterCb      = std::nullptr_t,
    typename CancelCb     = std::nullptr_t,
    typename SuccessCb    = std::nullptr_t,
    typename SuccessEvent = std::nullptr_t,
    typename BeforeEvent  = std::nullptr_t,
    typename AfterEvent   = std::nullptr_t,
    typename CancelEvent  = std::nullptr_t>
class EventPromise {
public:
    using RawEventType = std::remove_reference_t<EventType>;

    static_assert(ValidEventType<RawEventType>, "Invalid event type");
    static_assert(ValidCallbackOrNull<BeforeCb, RawEventType>, "Invalid before callback");
    static_assert(ValidCallbackOrNull<AfterCb, RawEventType>, "Invalid after callback");
    static_assert(ValidCallbackOrNull<CancelCb, RawEventType>, "Invalid cancel callback");
    static_assert(ValidCallbackOrNull<SuccessCb, RawEventType>, "Invalid success callback");
    static_assert(ValidEventOrNull<SuccessEvent>, "Invalid success event type");
    static_assert(ValidEventOrNull<BeforeEvent>, "Invalid before event type");
    static_assert(ValidEventOrNull<AfterEvent>, "Invalid after event type");
    static_assert(ValidEventOrNull<CancelEvent>, "Invalid cancel event type");

public:
    LL_NO_UNIQUE_ADDRESS RawEventType& mEvent;
    LL_NO_UNIQUE_ADDRESS BeforeCb      mBefore;
    LL_NO_UNIQUE_ADDRESS AfterCb       mAfter;
    LL_NO_UNIQUE_ADDRESS CancelCb      mCancel;
    LL_NO_UNIQUE_ADDRESS SuccessCb     mSuccess;
    LL_NO_UNIQUE_ADDRESS SuccessEvent  mSuccessEvent;
    LL_NO_UNIQUE_ADDRESS BeforeEvent   mBeforeEvent;
    LL_NO_UNIQUE_ADDRESS AfterEvent    mAfterEvent;
    LL_NO_UNIQUE_ADDRESS CancelEvent   mCancelEvent;

public:
    [[nodiscard]] inline constexpr explicit EventPromise(
        RawEventType& event,
        BeforeCb      before       = BeforeCb{},
        AfterCb       after        = AfterCb{},
        CancelCb      cancel       = CancelCb{},
        SuccessCb     success      = SuccessCb{},
        SuccessEvent  successEvent = SuccessEvent{},
        BeforeEvent   beforeEvent  = BeforeEvent{},
        AfterEvent    afterEvent   = AfterEvent{},
        CancelEvent   cancelEvent  = CancelEvent{}
    ) noexcept
    : mEvent(event),
      mBefore(std::move(before)),
      mAfter(std::move(after)),
      mCancel(std::move(cancel)),
      mSuccess(std::move(success)),
      mSuccessEvent(std::move(successEvent)),
      mBeforeEvent(std::move(beforeEvent)),
      mAfterEvent(std::move(afterEvent)),
      mCancelEvent(std::move(cancelEvent)) {}

    inline constexpr ~EventPromise()             = default;
    EventPromise(const EventPromise&)            = delete;
    EventPromise& operator=(const EventPromise&) = delete;

public:
    template <typename NewBefore>
        requires InvocableForEvent<NewBefore, RawEventType>
    [[nodiscard]] inline constexpr auto onBefore(NewBefore&& callback) && noexcept {
        return EventPromise<
            EventType,
            NewBefore,
            AfterCb,
            CancelCb,
            SuccessCb,
            SuccessEvent,
            BeforeEvent,
            AfterEvent,
            CancelEvent>(
            mEvent,
            std::forward<NewBefore>(callback),
            std::move(mAfter),
            std::move(mCancel),
            std::move(mSuccess),
            std::move(mSuccessEvent),
            std::move(mBeforeEvent),
            std::move(mAfterEvent),
            std::move(mCancelEvent)
        );
    }

    template <typename NewBeforeEvent>
        requires ValidEventType<NewBeforeEvent>
    [[nodiscard]] inline constexpr auto onBeforeEvent(NewBeforeEvent&& event) && noexcept {
        return EventPromise<
            EventType,
            BeforeCb,
            AfterCb,
            CancelCb,
            SuccessCb,
            SuccessEvent,
            NewBeforeEvent,
            AfterEvent,
            CancelEvent>(
            mEvent,
            std::move(mBefore),
            std::move(mAfter),
            std::move(mCancel),
            std::move(mSuccess),
            std::move(mSuccessEvent),
            std::forward<NewBeforeEvent>(event),
            std::move(mAfterEvent),
            std::move(mCancelEvent)
        );
    }

    template <typename NewCancel>
        requires CancellableEvent<RawEventType> && InvocableForEvent<NewCancel, RawEventType>
    [[nodiscard]] inline constexpr auto onCancel(NewCancel&& callback) && noexcept {
        return EventPromise<
            EventType,
            BeforeCb,
            AfterCb,
            NewCancel,
            SuccessCb,
            SuccessEvent,
            BeforeEvent,
            AfterEvent,
            CancelEvent>(
            mEvent,
            std::move(mBefore),
            std::move(mAfter),
            std::forward<NewCancel>(callback),
            std::move(mSuccess),
            std::move(mSuccessEvent),
            std::move(mBeforeEvent),
            std::move(mAfterEvent),
            std::move(mCancelEvent)
        );
    }

    template <typename NewCancelEvent>
        requires CancellableEvent<RawEventType> && ValidEventType<NewCancelEvent>
    [[nodiscard]] inline constexpr auto onCancelEvent(NewCancelEvent&& event) && noexcept {
        return EventPromise<
            EventType,
            BeforeCb,
            AfterCb,
            CancelCb,
            SuccessCb,
            SuccessEvent,
            BeforeEvent,
            AfterEvent,
            NewCancelEvent>(
            mEvent,
            std::move(mBefore),
            std::move(mAfter),
            std::move(mCancel),
            std::move(mSuccess),
            std::move(mSuccessEvent),
            std::move(mBeforeEvent),
            std::move(mAfterEvent),
            std::forward<NewCancelEvent>(event)
        );
    }

    template <typename NewSuccess>
        requires CancellableEvent<RawEventType> && InvocableForEvent<NewSuccess, RawEventType>
    [[nodiscard]] inline constexpr auto onSuccess(NewSuccess&& callback) && noexcept {
        return EventPromise<
            EventType,
            BeforeCb,
            AfterCb,
            CancelCb,
            NewSuccess,
            SuccessEvent,
            BeforeEvent,
            AfterEvent,
            CancelEvent>(
            mEvent,
            std::move(mBefore),
            std::move(mAfter),
            std::move(mCancel),
            std::forward<NewSuccess>(callback),
            std::move(mSuccessEvent),
            std::move(mBeforeEvent),
            std::move(mAfterEvent),
            std::move(mCancelEvent)
        );
    }

    template <typename NewSuccessEvent>
        requires CancellableEvent<RawEventType> && ValidEventType<NewSuccessEvent>
    [[nodiscard]] inline constexpr auto onSuccessEvent(NewSuccessEvent&& event) && noexcept {
        return EventPromise<
            EventType,
            BeforeCb,
            AfterCb,
            CancelCb,
            SuccessCb,
            NewSuccessEvent,
            BeforeEvent,
            AfterEvent,
            CancelEvent>(
            mEvent,
            std::move(mBefore),
            std::move(mAfter),
            std::move(mCancel),
            std::move(mSuccess),
            std::forward<NewSuccessEvent>(event),
            std::move(mBeforeEvent),
            std::move(mAfterEvent),
            std::move(mCancelEvent)
        );
    }

    template <typename NewAfter>
        requires InvocableForEvent<NewAfter, RawEventType>
    [[nodiscard]] inline constexpr auto onAfter(NewAfter&& callback) && noexcept {
        return EventPromise<
            EventType,
            BeforeCb,
            NewAfter,
            CancelCb,
            SuccessCb,
            SuccessEvent,
            BeforeEvent,
            AfterEvent,
            CancelEvent>(
            mEvent,
            std::move(mBefore),
            std::forward<NewAfter>(callback),
            std::move(mCancel),
            std::move(mSuccess),
            std::move(mSuccessEvent),
            std::move(mBeforeEvent),
            std::move(mAfterEvent),
            std::move(mCancelEvent)
        );
    }

    template <typename NewAfterEvent>
        requires ValidEventType<NewAfterEvent>
    [[nodiscard]] inline constexpr auto onAfterEvent(NewAfterEvent&& event) && noexcept {
        return EventPromise<
            EventType,
            BeforeCb,
            AfterCb,
            CancelCb,
            SuccessCb,
            SuccessEvent,
            BeforeEvent,
            NewAfterEvent,
            CancelEvent>(
            mEvent,
            std::move(mBefore),
            std::move(mAfter),
            std::move(mCancel),
            std::move(mSuccess),
            std::move(mSuccessEvent),
            std::move(mBeforeEvent),
            std::forward<NewAfterEvent>(event),
            std::move(mCancelEvent)
        );
    }

public:
    inline constexpr auto publish() && noexcept(are_callbacks_noexcept_v<BeforeCb, AfterCb, CancelCb, SuccessCb>) {
        invokeAndPublish(mBefore, std::move(mBeforeEvent));
        getEventBus().publish(mEvent);
        if constexpr (CancellableEvent<RawEventType>) {
            if (mEvent.isCancelled()) {
                invokeAndPublish(mCancel, std::move(mCancelEvent));
            } else {
                invokeAndPublish(mSuccess, std::move(mSuccessEvent));
            }
        }
        invokeAndPublish(mAfter, std::move(mAfterEvent));

        return EventPromise<
            EventType,
            BeforeCb,
            AfterCb,
            CancelCb,
            SuccessCb,
            SuccessEvent,
            BeforeEvent,
            AfterEvent,
            CancelEvent>(
            mEvent,
            std::move(mBefore),
            std::move(mAfter),
            std::move(mCancel),
            std::move(mSuccess),
            std::move(mSuccessEvent),
            std::move(mBeforeEvent),
            std::move(mAfterEvent),
            std::move(mCancelEvent)
        );
    }

private:
    template <typename Callback, typename PublishEvent>
    inline constexpr void invokeAndPublish(Callback&& cb, PublishEvent&& pubEvent) const noexcept {
        if constexpr (!std::is_same_v<std::remove_cvref_t<Callback>, std::nullptr_t>) {
            if constexpr (requires { cb(mEvent); }) {
                cb(mEvent);
            } else if constexpr (requires { cb(); }) {
                cb();
            }
        }
        if constexpr (!std::is_same_v<std::remove_cvref_t<PublishEvent>, std::nullptr_t>) {
            getEventBus().publish(std::forward<PublishEvent>(pubEvent));
        }
    }

public:
    [[nodiscard]] inline constexpr explicit operator bool() const noexcept {
        static_assert(CancellableEvent<RawEventType>, "Event type is not cancellable");
        return mEvent.isCancelled();
    }

    inline constexpr RawEventType&       operator*() noexcept { return mEvent; }
    inline constexpr const RawEventType& operator*() const noexcept { return mEvent; }
    inline constexpr RawEventType*       operator->() noexcept { return &mEvent; }
    inline constexpr const RawEventType* operator->() const noexcept { return &mEvent; }
};

template <typename T>
    requires requires { typename EventPromise<std::remove_cvref_t<T>>; }
[[nodiscard]] inline constexpr auto eventPromise(T&& event) {
    return EventPromise<std::remove_cvref_t<T>>(event);
}

} // namespace ila::inline utils::event_utils