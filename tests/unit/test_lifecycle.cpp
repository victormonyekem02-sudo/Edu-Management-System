// UT-LIF: submission state machine (state-submission diagram, BR-06, BR-07).
#include <gtest/gtest.h>

#include "lms/common/Errors.h"
#include "lms/domain/Entities.h"
#include "lms/domain/SubmissionStatus.h"

using namespace lms;
using S = SubmissionStatus;

namespace {

struct TransitionCase {
    S from;
    S to;
    bool allowed;
};

class TransitionTable : public ::testing::TestWithParam<TransitionCase> {};

TEST_P(TransitionTable, MatchesStateMachineDiagram) {
    const TransitionCase& c = GetParam();
    EXPECT_EQ(SubmissionLifecycle::canTransition(c.from, c.to), c.allowed)
        << statusToCode(c.from) << " -> " << statusToCode(c.to);
    if (c.allowed) {
        EXPECT_NO_THROW(SubmissionLifecycle::requireTransition(c.from, c.to));
    } else {
        EXPECT_THROW(SubmissionLifecycle::requireTransition(c.from, c.to), BusinessRuleError);
    }
}

std::vector<TransitionCase> allTransitions() {
    const S states[] = {S::Submitted, S::Late, S::Marked, S::Moderated, S::Released};
    const auto allowed = [](S from, S to) {
        return (from == S::Submitted && (to == S::Submitted || to == S::Marked)) ||
               (from == S::Late && (to == S::Submitted || to == S::Marked)) ||
               (from == S::Marked && (to == S::Marked || to == S::Moderated)) ||
               (from == S::Moderated && (to == S::Moderated || to == S::Released));
    };
    std::vector<TransitionCase> cases;
    for (S from : states) {
        for (S to : states) {
            cases.push_back({from, to, allowed(from, to)});
        }
    }
    return cases;
}

INSTANTIATE_TEST_SUITE_P(UT_LIF, TransitionTable, ::testing::ValuesIn(allTransitions()));

TEST(UT_LIF, OnlyUnmoderatedWorkIsMarkable) {
    EXPECT_TRUE(SubmissionLifecycle::isMarkable(S::Submitted));
    EXPECT_TRUE(SubmissionLifecycle::isMarkable(S::Late));
    EXPECT_TRUE(SubmissionLifecycle::isMarkable(S::Marked));
    EXPECT_FALSE(SubmissionLifecycle::isMarkable(S::Moderated));
    EXPECT_FALSE(SubmissionLifecycle::isMarkable(S::Released));
}

TEST(UT_LIF, OnlyReleasedMarksAreVisibleToStudents) {
    for (S s : {S::Submitted, S::Late, S::Marked, S::Moderated}) {
        EXPECT_FALSE(SubmissionLifecycle::isVisibleToStudent(s));
    }
    EXPECT_TRUE(SubmissionLifecycle::isVisibleToStudent(S::Released));
}

TEST(UT_LIF, StatusCodesRoundTrip) {
    for (S s : {S::Submitted, S::Late, S::Marked, S::Moderated, S::Released}) {
        EXPECT_EQ(statusFromCode(statusToCode(s)), s);
        EXPECT_FALSE(statusDisplayName(s).empty());
    }
    EXPECT_THROW(statusFromCode("DRAFT"), ValidationError);
}

TEST(UT_LIF, AuditActionCodesRoundTrip) {
    for (AuditAction a : {AuditAction::Mark, AuditAction::Remark, AuditAction::ModerationApprove,
                          AuditAction::ModerationAdjust, AuditAction::Release}) {
        EXPECT_EQ(auditActionFromCode(auditActionToCode(a)), a);
    }
    EXPECT_THROW(auditActionFromCode("DELETE"), ValidationError);
}

}  // namespace
