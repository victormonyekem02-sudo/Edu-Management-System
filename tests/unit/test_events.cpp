// UT-EVT: Observer pattern - SubmissionEventPublisher.
#include <gtest/gtest.h>

#include <vector>

#include "lms/events/SubmissionEvents.h"

using namespace lms;

namespace {

class RecordingObserver : public ISubmissionObserver {
public:
    void onSubmissionEvent(const SubmissionEvent& e) override { received.push_back(e); }
    std::vector<SubmissionEvent> received;
};

TEST(UT_EVT, PublishesToEverySubscriber) {
    SubmissionEventPublisher publisher;
    RecordingObserver a;
    RecordingObserver b;
    publisher.subscribe(a);
    publisher.subscribe(b);
    SubmissionEvent e;
    e.type = SubmissionEventType::Marked;
    e.submissionId = 42;
    publisher.publish(e);
    ASSERT_EQ(a.received.size(), 1u);
    ASSERT_EQ(b.received.size(), 1u);
    EXPECT_EQ(a.received[0].submissionId, 42);
    EXPECT_EQ(b.received[0].type, SubmissionEventType::Marked);
}

TEST(UT_EVT, DuplicateSubscriptionIsIgnored) {
    SubmissionEventPublisher publisher;
    RecordingObserver a;
    publisher.subscribe(a);
    publisher.subscribe(a);
    EXPECT_EQ(publisher.observerCount(), 1u);
    publisher.publish({});
    EXPECT_EQ(a.received.size(), 1u);
}

TEST(UT_EVT, UnsubscribedObserverReceivesNothing) {
    SubmissionEventPublisher publisher;
    RecordingObserver a;
    publisher.subscribe(a);
    publisher.unsubscribe(a);
    publisher.publish({});
    EXPECT_TRUE(a.received.empty());
    EXPECT_EQ(publisher.observerCount(), 0u);
}

}  // namespace
