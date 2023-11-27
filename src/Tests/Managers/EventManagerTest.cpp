#include "Base/Event.h"
#include "Handlers/Handler.h"
#include "Managers/EventManager.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"
#include "Util/Delay.h"

/// \ingroup Tests
class EventManagerTest : public TestBase {
  protected:
    /// Derived Handler class for testing.
    class TestHandler : public Handler {
      public:
        bool   what_to_return = false;
        size_t last_serial    = 0;
        size_t reset_count    = 0;
        float  delay          = 0;

        virtual bool HandleEvent(const Event &event) override {
            last_serial = event.serial;
            if (delay > 0)
                Util::DelayThread(delay);
            return what_to_return;
        }

        virtual void Reset() { ++reset_count; }
    };

    DECL_SHARED_PTR(TestHandler);
};

TEST_F(EventManagerTest, HandleEvents) {
    TestHandlerPtr th(new TestHandler);

    EventManager em;
    em.AppendHandler(th);

    EXPECT_EQ(0U, th->last_serial);

    std::vector<Event> events;

    // HandleEvents() returns true if the application is not exiting.

    th->SetEnabled(false);

    Event event;
    events.push_back(event);

    th->what_to_return = false;
    EXPECT_TRUE(em.HandleEvents(events, false, 0));
    EXPECT_EQ(0U, th->last_serial);

    th->what_to_return = true;
    EXPECT_TRUE(em.HandleEvents(events, false, 0));
    EXPECT_EQ(0U, th->last_serial);

    th->SetEnabled(true);

    th->what_to_return = false;
    EXPECT_TRUE(em.HandleEvents(events, false, 0));
    EXPECT_EQ(2U, th->last_serial);

    th->what_to_return = true;
    EXPECT_TRUE(em.HandleEvents(events, false, 0));
    EXPECT_EQ(3U, th->last_serial);

    // Exit.
    th->what_to_return = false;
    events.back().flags.Set(Event::Flag::kExit);
    EXPECT_FALSE(em.HandleEvents(events, false, 0));
    EXPECT_EQ(4U, th->last_serial);

    EXPECT_EQ(0U, th->reset_count);
    em.Reset();
    EXPECT_EQ(1U, th->reset_count);

    em.ClearHandlers();
    em.Reset();
    EXPECT_EQ(1U, th->reset_count);  // No change after ClearHandlers().
}

TEST_F(EventManagerTest, PendingEvents) {
    // Set a maximum time to handle events and exceed it by delaying.
    TestHandlerPtr th(new TestHandler);

    EventManager em;
    em.AppendHandler(th);

    std::vector<Event> events;
    Event event;
    events.push_back(event);
    events.push_back(event);

    const float kTimeout = .0001f;
    th->delay = 2 * kTimeout;
    EXPECT_TRUE(em.HandleEvents(events, false, kTimeout));
    EXPECT_TRUE(em.HasPendingEvents());
    EXPECT_EQ(0U, th->last_serial);

    events.clear();  // No new events.
    th->delay = 0;
    EXPECT_TRUE(em.HandleEvents(events, false, kTimeout));
    EXPECT_FALSE(em.HasPendingEvents());
    EXPECT_EQ(1U, th->last_serial);
}
