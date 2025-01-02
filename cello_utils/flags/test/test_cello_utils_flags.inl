

#include <juce_core/juce_core.h>

class Test_Flags : public TestSuite
{
public:
    Test_Flags ()
    : TestSuite ("Flags", "Cello Utilities")
    {
    }

    void runTest () override
    {
        beginTest ("Feature flag tests");

        test ("minimum",
              [this] ()
              {
                  // clang-format off
                  juce::ValueTree tree { "condition", {},
                      { { "cohort", { { "min", 5 } } }, }
                  };
                  // clang-format on
                  cello::utils::Condition condition { tree };
                  cello::utils::Context context;
                  // test less than min, equal to min, and greater than min
                  context.setattr ("cohort", 4);
                  expect (condition.evaluate (context).isVoid ());
                  context.setattr ("cohort", 5);
                  expect (!condition.evaluate (context).isVoid ());
                  context.setattr ("cohort", 6);
                  expect (!condition.evaluate (context).isVoid ());
              });

        test ("maximum",
              [this] ()
              {
                  // clang-format off
                  juce::ValueTree tree { "condition", {},
                    { { "cohort", { { "max", 5 } } }, }
                  };
                  // clang-format on
                  cello::utils::Condition condition { tree };
                  cello::utils::Context context;
                  // test less than max, equal to max, and greater than max
                  context.setattr ("cohort", 4);
                  expect (static_cast<bool> (condition.evaluate (context)));
                  context.setattr ("cohort", 5);
                  expect (!static_cast<bool> (condition.evaluate (context)));
                  context.setattr ("cohort", 6);
                  expect (!static_cast<bool> (condition.evaluate (context)));
              });

        test ("min/max range",
              [this] ()
              {
                  // clang-format off
                  juce::ValueTree tree { "condition", {},
                    { { "cohort", { {"min", 3}, { "max", 5 } } }, }
                  };
                  // clang-format on
                  cello::utils::Condition condition { tree };
                  cello::utils::Context context;
                  // test less than min, inside range, above max
                  context.setattr ("cohort", 2);
                  expect (!static_cast<bool> (condition.evaluate (context)));
                  context.setattr ("cohort", 3);
                  expect (static_cast<bool> (condition.evaluate (context)));
                  context.setattr ("cohort", 4);
                  expect (static_cast<bool> (condition.evaluate (context)));
                  context.setattr ("cohort", 5);
                  expect (!static_cast<bool> (condition.evaluate (context)));
              });

        test ("allowed",
              [this] ()
              {
                  // clang-format off
                  juce::ValueTree tree { "condition", {},
                    { { "type", { {"allowed", "dev,int,beta"} } }, }
                  };
                  // clang-format on
                  cello::utils::Condition condition { tree };
                  cello::utils::Context context;
                  context.setattr ("type", juce::String { "dev" });
                  expect (static_cast<bool> (condition.evaluate (context)));
                  context.setattr ("type", juce::String { "int" });
                  expect (static_cast<bool> (condition.evaluate (context)));
                  context.setattr ("type", juce::String { "beta" });
                  expect (static_cast<bool> (condition.evaluate (context)));
                  context.setattr ("type", juce::String { "alpha" });
                  expect (!static_cast<bool> (condition.evaluate (context)));
              });

        test ("disallowed",
              [this] ()
              {
                  // clang-format off
                  juce::ValueTree tree { "condition", {},
                    { { "type", { {"disallowed", "alpha,beta"} } }, }
                  };
                  // clang-format on
                  cello::utils::Condition condition { tree };
                  cello::utils::Context context;
                  // anything NOT listed in disallowed passes.
                  context.setattr ("type", juce::String { "dev" });
                  expect (static_cast<bool> (condition.evaluate (context)));
                  context.setattr ("type", juce::String { "int" });
                  expect (static_cast<bool> (condition.evaluate (context)));
                  context.setattr ("type", juce::String { "alpha" });
                  expect (!static_cast<bool> (condition.evaluate (context)));
                  context.setattr ("type", juce::String { "beta" });
                  expect (!static_cast<bool> (condition.evaluate (context)));
              });

        test ("exact value",
              [this] ()
              {
                  // clang-format off
                  juce::ValueTree tree { "condition", {},
                    { { "type", { {"value", "100"} } }, }
                  };
                  // clang-format on
                  cello::utils::Condition condition { tree };
                  cello::utils::Context context;
                  context.setattr ("type", 1);
                  expect (condition.evaluate (context).isVoid ());
                  context.setattr ("type", 100);
                  expect (!condition.evaluate (context).isVoid ());
              });

        test ("multiple tests per condition",
              [this] ()
              {
                  // clang-format off
                     juce::ValueTree tree { "condition", {},
                         { { "cohort", { {"min", 3}, { "max", 5 } } },
                           { "type", { {"allowed", "dev,int,beta"} } }, }
                     };
                  // clang-format on
                  cello::utils::Condition condition { tree };
                  cello::utils::Context context;
                  // test less than min, inside range, above max
                  context.setattr ("cohort", 2);
                  context.setattr ("type", juce::String { "dev" });
                  expect (!static_cast<bool> (condition.evaluate (context)));
                  context.setattr ("cohort", 3);
                  context.setattr ("type", juce::String { "dev" });
                  expect (static_cast<bool> (condition.evaluate (context)));
                  context.setattr ("cohort", 4);
                  context.setattr ("type", juce::String { "alpha" });
                  expect (condition.evaluate (context).isVoid ());
                  context.setattr ("cohort", 5);
                  context.setattr ("type", juce::String { "dev" });
                  expect (!static_cast<bool> (condition.evaluate (context)));
              });

        test ("custom result returns",
              [this] ()
              {
                  // clang-format off
                  juce::ValueTree tree { "condition", {{"result", "customResult"}},
                  { { "type", { {"allowed", "dev,int,beta"} } }, }
                  };
                  // clang-format on
                  cello::utils::Condition condition { tree };
                  cello::utils::Context context;
                  context.setattr ("type", juce::String { "dev" });
                  const auto result { condition.evaluate (context) };
                  expect (result.toString () == juce::String { "customResult" });
                  context.setattr ("type", juce::String { "alpha" });
                  expect (condition.evaluate (context).isVoid ());
              });
    }

private:
    // !!! test class member vars here...
};

static Test_Flags testFlags;
