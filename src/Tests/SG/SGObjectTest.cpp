#include "Parser/Registry.h"
#include "SG/Node.h"
#include "SG/Object.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"

// Named this with SG prefix to distinguish tests from Parser::Object.
class SGObjectTest : public TestBaseWithTypes {
  protected:
    // Derived SG::Object that makes ChangeName() and the Observe API public
    // and implements ProcessChange() to increment a count.
    class TestObject : public SG::Object {
      public:
        size_t change_count = 0;
        using SG::Object::ChangeName;
        using SG::Object::Observe;
        using SG::Object::Unobserve;
        using SG::Object::IsObserving;

        // Causes ProcessChange() to be called.
        void ChangeMe() { ProcessChange(SG::Change::kGeometry, *this); }

      protected:
        TestObject() {}
        virtual bool ProcessChange(SG::Change change,
                                   const SG::Object &obj) override {
            if (! SG::Object::ProcessChange(change, obj))
                return false;
            ++change_count;
            return true;
        }
        friend class Parser::Registry;
    };
};

TEST_F(SGObjectTest, ChangeName) {
    Parser::Registry::AddType<TestObject>("TestObject");

    auto obj = CreateObject<TestObject>("Blue");
    EXPECT_EQ("Blue", obj->GetName());

    obj->ChangeName("Red");
    EXPECT_EQ("Red", obj->GetName());
}

TEST_F(SGObjectTest, Change) {
    auto node = CreateObject<SG::Node>();

    bool was_changed = false;

    auto func = [&](SG::Change change, const SG::Object &obj){
        if (&obj == node.get())
            was_changed = true;
    };
    node->GetChanged().AddObserver("key", func);
    EXPECT_FALSE(was_changed);
    node->SetUniformScale(5);
    EXPECT_TRUE(was_changed);
}

TEST_F(SGObjectTest, Observers) {
    Parser::Registry::AddType<TestObject>("TestObject");

    auto a = CreateObject<TestObject>("A");
    auto b = CreateObject<TestObject>("B");
    EXPECT_EQ(0U, a->change_count);
    EXPECT_EQ(0U, b->change_count);

    a->Observe(*b);
    EXPECT_EQ(0U, a->change_count);
    EXPECT_EQ(0U, b->change_count);
    EXPECT_TRUE(a->IsObserving(*b));
    EXPECT_FALSE(b->IsObserving(*a));

    a->ChangeMe();
    EXPECT_EQ(1U, a->change_count);
    EXPECT_EQ(0U, b->change_count);

    b->ChangeMe();
    EXPECT_EQ(2U, a->change_count);
    EXPECT_EQ(1U, b->change_count);

    b->SetNotifyEnabled(false);
    b->ChangeMe();
    EXPECT_EQ(2U, a->change_count);
    EXPECT_EQ(1U, b->change_count);
    b->SetNotifyEnabled(true);
    b->ChangeMe();
    EXPECT_EQ(3U, a->change_count);
    EXPECT_EQ(2U, b->change_count);

    a->Unobserve(*b);
    b->ChangeMe();
    EXPECT_EQ(3U, a->change_count);
    EXPECT_EQ(3U, b->change_count);
    EXPECT_FALSE(a->IsObserving(*b));
    EXPECT_FALSE(b->IsObserving(*a));

    // Test errors.
    TEST_THROW(a->Unobserve(*b), AssertException, "Failed to Unobserve");
    a->Observe(*b);
    TEST_THROW(a->Observe(*b),   AssertException, "Failed to Observe");
}
