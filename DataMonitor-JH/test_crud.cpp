#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstdio>
#include "MemberRepo.h"

using ::testing::_;
using ::testing::Return;

// ── 테스트용 임시 파일 경로 ────────────────────────────────
static const std::string TEST_FILE = "test_crud_members.json";

// ── 테스트 픽스처 ─────────────────────────────────────────
class MemberRepoTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::remove(TEST_FILE.c_str());
    }
    void TearDown() override {
        std::remove(TEST_FILE.c_str());
    }
};

// ═══════════════════════════════════════════════════════════
// CRUD 기본 테스트
// ═══════════════════════════════════════════════════════════

TEST_F(MemberRepoTest, Create_SetsIdAndFields) {
    MemberRepository repo(TEST_FILE);
    Member m = repo.create("Kim", "kim@test.com", "010-1111-2222", 30);

    EXPECT_EQ(m.id, 1);
    EXPECT_EQ(m.name, "Kim");
    EXPECT_EQ(m.email, "kim@test.com");
    EXPECT_EQ(m.phone, "010-1111-2222");
    EXPECT_EQ(m.age, 30);
    EXPECT_FALSE(m.created_at.empty());
}

TEST_F(MemberRepoTest, Create_AutoIncrementsId) {
    MemberRepository repo(TEST_FILE);
    Member m1 = repo.create("Kim", "kim@test.com", "010-0000-0001", 25);
    Member m2 = repo.create("Lee", "lee@test.com", "010-0000-0002", 30);
    Member m3 = repo.create("Park", "park@test.com", "010-0000-0003", 35);

    EXPECT_EQ(m1.id, 1);
    EXPECT_EQ(m2.id, 2);
    EXPECT_EQ(m3.id, 3);
}

TEST_F(MemberRepoTest, FindAll_ReturnsAllMembers) {
    MemberRepository repo(TEST_FILE);
    repo.create("Kim", "kim@test.com", "010-0000-0001", 25);
    repo.create("Lee", "lee@test.com", "010-0000-0002", 30);

    auto list = repo.findAll();
    EXPECT_EQ(list.size(), 2u);
}

TEST_F(MemberRepoTest, FindById_ReturnsCorrectMember) {
    MemberRepository repo(TEST_FILE);
    repo.create("Kim", "kim@test.com", "010-0000-0001", 25);
    repo.create("Lee", "lee@test.com", "010-0000-0002", 30);

    auto opt = repo.findById(2);
    ASSERT_TRUE(opt.has_value());
    EXPECT_EQ(opt->name, "Lee");
}

TEST_F(MemberRepoTest, FindById_ReturnsNulloptWhenNotFound) {
    MemberRepository repo(TEST_FILE);
    auto opt = repo.findById(999);
    EXPECT_FALSE(opt.has_value());
}

TEST_F(MemberRepoTest, FindByName_ReturnsPartialMatch) {
    MemberRepository repo(TEST_FILE);
    repo.create("Kim Cheol", "cheol@test.com", "010-0000-0001", 25);
    repo.create("Kim Young", "young@test.com", "010-0000-0002", 28);
    repo.create("Lee Jun",   "jun@test.com",   "010-0000-0003", 32);

    auto results = repo.findByName("Kim");
    EXPECT_EQ(results.size(), 2u);
}

TEST_F(MemberRepoTest, Update_ModifiesExistingMember) {
    MemberRepository repo(TEST_FILE);
    Member m = repo.create("Kim", "kim@test.com", "010-0000-0001", 25);

    m.email = "updated@test.com";
    m.age   = 26;
    bool ok = repo.update(m);

    EXPECT_TRUE(ok);
    auto found = repo.findById(m.id);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->email, "updated@test.com");
    EXPECT_EQ(found->age, 26);
}

TEST_F(MemberRepoTest, Update_ReturnsFalseForNonExistent) {
    MemberRepository repo(TEST_FILE);
    Member fake;
    fake.id = 999;
    EXPECT_FALSE(repo.update(fake));
}

TEST_F(MemberRepoTest, Remove_DeletesMember) {
    MemberRepository repo(TEST_FILE);
    Member m = repo.create("Kim", "kim@test.com", "010-0000-0001", 25);

    bool ok = repo.remove(m.id);
    EXPECT_TRUE(ok);

    auto found = repo.findById(m.id);
    EXPECT_FALSE(found.has_value());
    EXPECT_EQ(repo.findAll().size(), 0u);
}

TEST_F(MemberRepoTest, Remove_ReturnsFalseForNonExistent) {
    MemberRepository repo(TEST_FILE);
    EXPECT_FALSE(repo.remove(999));
}

// ═══════════════════════════════════════════════════════════
// 영속화(Persistence) 테스트 — 앱 재시작 시뮬레이션
// ═══════════════════════════════════════════════════════════

TEST_F(MemberRepoTest, Persistence_DataSurvivesRestart) {
    {
        MemberRepository repo(TEST_FILE);
        repo.create("Kim", "kim@test.com", "010-1111-0001", 25);
        repo.create("Lee", "lee@test.com", "010-1111-0002", 30);
    }

    MemberRepository repo2(TEST_FILE);
    auto list = repo2.findAll();

    ASSERT_EQ(list.size(), 2u);
    EXPECT_EQ(list[0].name, "Kim");
    EXPECT_EQ(list[1].name, "Lee");
}

TEST_F(MemberRepoTest, Persistence_IdContinuesAfterRestart) {
    {
        MemberRepository repo(TEST_FILE);
        repo.create("Kim", "kim@test.com", "010-1111-0001", 25);
        repo.create("Lee", "lee@test.com", "010-1111-0002", 30);
    }

    MemberRepository repo2(TEST_FILE);
    Member m3 = repo2.create("Park", "park@test.com", "010-1111-0003", 35);

    EXPECT_EQ(m3.id, 3);
}

TEST_F(MemberRepoTest, Persistence_UpdateSurvivesRestart) {
    int savedId = 0;
    {
        MemberRepository repo(TEST_FILE);
        Member m = repo.create("Kim", "kim@test.com", "010-0000-0001", 25);
        savedId  = m.id;
        m.email  = "new_kim@test.com";
        repo.update(m);
    }

    MemberRepository repo2(TEST_FILE);
    auto found = repo2.findById(savedId);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->email, "new_kim@test.com");
}

TEST_F(MemberRepoTest, Persistence_DeleteSurvivesRestart) {
    {
        MemberRepository repo(TEST_FILE);
        repo.create("Kim", "kim@test.com", "010-0000-0001", 25);
        repo.create("Lee", "lee@test.com", "010-0000-0002", 30);
        repo.remove(1);
    }

    MemberRepository repo2(TEST_FILE);
    EXPECT_EQ(repo2.findAll().size(), 1u);
    EXPECT_FALSE(repo2.findById(1).has_value());
}

// ═══════════════════════════════════════════════════════════
// gmock 데모 — 이벤트 관찰자(Observer) 패턴
// ═══════════════════════════════════════════════════════════

class IMemberObserver {
public:
    virtual ~IMemberObserver() = default;
    virtual void onCreated(int id, const std::string& name) = 0;
    virtual void onUpdated(int id) = 0;
    virtual void onDeleted(int id) = 0;
};

class MockMemberObserver : public IMemberObserver {
public:
    MOCK_METHOD(void, onCreated, (int id, const std::string& name), (override));
    MOCK_METHOD(void, onUpdated, (int id), (override));
    MOCK_METHOD(void, onDeleted, (int id), (override));
};

TEST_F(MemberRepoTest, GMock_ObserverReceivesExpectedCalls) {
    MockMemberObserver observer;

    EXPECT_CALL(observer, onCreated(1, "Kim")).Times(1);
    EXPECT_CALL(observer, onUpdated(1)).Times(1);
    EXPECT_CALL(observer, onDeleted(1)).Times(1);

    MemberRepository repo(TEST_FILE);
    Member m = repo.create("Kim", "kim@test.com", "010-0000-0001", 25);
    observer.onCreated(m.id, m.name);

    m.email = "updated@test.com";
    repo.update(m);
    observer.onUpdated(m.id);

    repo.remove(m.id);
    observer.onDeleted(m.id);
}

TEST_F(MemberRepoTest, GMock_NoDeleteCallWhenMemberNotFound) {
    MockMemberObserver observer;

    EXPECT_CALL(observer, onDeleted(_)).Times(0);

    MemberRepository repo(TEST_FILE);
    bool removed = repo.remove(999);

    if (removed)
        observer.onDeleted(999);
}

// ═══════════════════════════════════════════════════════════
// 메인 진입점
// ═══════════════════════════════════════════════════════════

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
