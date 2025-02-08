#include <juce_core/juce_core.h>

namespace
{
bool cmpStr (const juce::String& a, const juce::String& b)
{
    return a.compare (b) == 0;
}
} // namespace

/**
 * @brief A set of flags with defaults to use for testing.
 *
 */
class UnitTestFlags : public cello::utils::Flags
{
public:
    UnitTestFlags (cello::Object* root)
    : cello::utils::Flags (root)
    {
    }

    MAKE_VALUE_MEMBER (bool, test1, false);
    MAKE_VALUE_MEMBER (bool, test2, false);
    MAKE_VALUE_MEMBER (bool, test3, false);
    MAKE_VALUE_MEMBER (juce::String, test4, "test4");
};

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

        test ("condition: minimum",
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

        test ("condition: maximum",
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

        test ("condition: min/max range",
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

        test ("condition: allowed",
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

        test ("condition: disallowed",
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

        test ("condition: exact value",
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

        test ("condition: multiple tests per condition",
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

        test ("condition: custom result returns",
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

        setup (
            [this] ()
            {
                flags   = std::make_unique<UnitTestFlags> (nullptr);
                context = std::make_unique<cello::utils::Context> ();
                context->setattr ("cohort", 2);
                context->setattr ("type", juce::String { "alpha" });
            });

        tearDown (
            [this] ()
            {
                flags   = nullptr;
                context = nullptr;
            });

        test ("flags: test simple flags",
              [this] ()
              {
                  expect (!flags->test1);
                  expect (!flags->test2);
                  expect (!flags->test3);
                  expect (cmpStr (flags->test4, "test4"));

                  // define the list of rules:
                  // clang-format off
                  juce::ValueTree rules { "rules", {},
                    {
                        { "test1", {}, 
                            { { "condition", {}, 
                                { { "cohort", { { "min", 3}, { "max", 5 } } }, } } 
                            }
                        },

                        { "test2", {}, 
                            { { "condition", {}, 
                                { { "cohort", { { "min", 1}, { "max", 3 } } }, } } 
                            }
                        }
                    }
                  };
                  // clang-format on

                  cello::utils::Rules ruleSet { rules };
                  ruleSet.evaluate (*context, *flags);
                  expect (!flags->test1);
                  expect (flags->test2);
                  expect (!flags->test3);
                  expect (cmpStr (flags->test4, "test4"));

                  context->setattr ("cohort", 4);
                  // reset this one...
                  flags->test2 = false;
                  ruleSet.evaluate (*context, *flags);
                  expect (flags->test1);
                  expect (!flags->test2);
                  expect (!flags->test3);
                  expect (cmpStr (flags->test4, "test4"));
              });

        test ("flags: test complex flags",
              [this] ()
              {
                  // clang-format off
                  juce::ValueTree rules { "rules", {},
                    {
                        { "test1", {}, {
                            { "condition", {}, {
                                { "cohort", { { "min", 3}, { "max", 5 } } },
                                { "type", { { "allowed", "dev,int" } } }
                            }},
                            { "condition", {}, {
                                { "type", { { "allowed", "beta" } } },
                                { "cohort", { { "min", 1}, { "max", 2 } } }
                            }}
                        }},
                        { "test2", { { "released", true } }, {} },
                        { "test4", {}, {
                            { "condition", { { "result", "customValue" } }, {
                                { "type", { { "disallowed", "alpha,prod" } } }
                            }}
                        }}
                    }
                  };
                  // clang-format on

                  cello::utils::Rules ruleSet { rules };

                  // Test first condition path (dev/int user, cohort 3-5)
                  context->setattr ("type", juce::String ("dev"));
                  context->setattr ("cohort", 4);
                  ruleSet.evaluate (*context, *flags);
                  expect (flags->test1);
                  expect (flags->test2);
                  expect (cmpStr (flags->test4, "customValue"));
                  // Test second condition path (beta user, cohort 1-2)
                  flags = std::make_unique<UnitTestFlags> (nullptr);
                  context->setattr ("type", juce::String ("beta"));
                  context->setattr ("cohort", 1);
                  flags->test1 = false;
                  ruleSet.evaluate (*context, *flags);
                  expect (flags->test1);
                  expect (flags->test2);
                  expect (flags->test4 == juce::String ("customValue"));

                  // Test failing both conditions
                  flags = std::make_unique<UnitTestFlags> (nullptr);
                  context->setattr ("type", juce::String ("beta"));
                  context->setattr ("cohort", 4);
                  flags->test1 = false;
                  ruleSet.evaluate (*context, *flags);
                  expect (!flags->test1);
                  expect (flags->test2);
                  expect (flags->test4 == juce::String ("customValue"));

                  // Test disallowed types
                  flags = std::make_unique<UnitTestFlags> (nullptr);
                  context->setattr<juce::String> ("type", "prod");
                  ruleSet.evaluate (*context, *flags);
                  expect (!flags->test1);
                  expect (flags->test2);
                  expect (flags->test4 != juce::String ("customValue"));
              });
    }

private:
    std::unique_ptr<UnitTestFlags> flags;
    std::unique_ptr<cello::utils::Context> context;
};

static Test_Flags testFlags;
