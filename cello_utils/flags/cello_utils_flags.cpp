/*
    Copyright (c) 2025 Brett g Porter
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:
    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#include <JuceHeader.h>

#include "cello_utils_flags.h"

namespace
{
const juce::Identifier allowedID { "allowed" };
const juce::Identifier conditionID { "condition" };
const juce::Identifier disallowedID { "disallowed" };
const juce::Identifier maxID { "max" };
const juce::Identifier minID { "min" };
const juce::Identifier resultID { "result" };
const juce::Identifier releasedID { "released" };
const juce::Identifier typeID { "type" };
const juce::Identifier valueID { "value" };
} // namespace

namespace cello::utils
{

void Rules::evaluate (const Context& context, Flags& flags) const
{
    // our children are a list of flag names, each of which contains
    // 1 or more conditions.
    for (const auto& flagRule : data)
    {
        // NOTE that the type of the `flagRule` tree may be any valid
        // juce::Identifier; it joins this set of conditions to the corresponding
        // flag in the `flags` object.

        // if this flag has been released, we don't need to evaluate it.
        // Note that we don't just check for the presence of the property,
        // but also that it is set to true -- if the XML is "released='0'",
        // we should treat that the same as if the property were not present.
        if (const auto released = flagRule.getProperty (releasedID, false); released)
        {
            // set the flag to true (default) or a custom result value if
            // one is provided.
            flags.setattr (flagRule.getType (), flagRule.getProperty (resultID, true));
            continue;
        }

        // iterate through the conditions. The first one that passes
        // will be used to update the state of the current flag. If none
        // pass, the flag will be left in its current/default state.
        for (const auto& conditionTree : flagRule)
        {
            if (conditionTree.getType () != conditionID)
            {
                // we shouldn't have any children that aren't conditions.
                jassertfalse;
                continue;
            }
            Condition condition { conditionTree };
            if (const auto result { condition.evaluate (context) }; !result.isVoid ())
            {
                flags.setattr (flagRule.getType (), result);
                break;
            }
        }
    }
}

juce::var Condition::evaluate (const Context& context) const
{
    juce::ValueTree contextTree { context };
    for (const auto& child : data)
    {
        const auto propertyCount { child.getNumProperties () };
        for (int i = 0; i < propertyCount; ++i)
        {
            const auto propertyName { child.getPropertyName (i) };
            const auto propertyValue { child.getProperty (propertyName) };
            const auto contextValue { contextTree.getProperty (child.getType ()) };
            bool testResult { false };
            if (propertyName == minID)
                testResult = isAboveMin (propertyValue, contextValue);
            else if (propertyName == maxID)
                testResult = isBelowMax (propertyValue, contextValue);
            else if (propertyName == allowedID)
                testResult = isAllowed (propertyValue, contextValue);
            else if (propertyName == disallowedID)
                testResult = !isAllowed (propertyValue, contextValue);
            else if (propertyName == valueID)
                testResult = (propertyValue == contextValue);
            else
            {
                // we looked for an attribute that doesn't exist --assert and
                // indicate that the condition is not met.
                jassertfalse;
                testResult = false;
            }

            if (!testResult)
                return juce::var ();
        }
    }
    return result ();
}

bool Condition::isAboveMin (const juce::var& test, const juce::var& actual) const
{
    if (test.isInt () && actual.isInt ())
        return static_cast<int> (actual) >= static_cast<int> (test);
    return actual.toString ().compareIgnoreCase (test.toString ()) >= 0;
}

bool Condition::isBelowMax (const juce::var& test, const juce::var& actual) const
{
    if (test.isInt () && actual.isInt ())
        return static_cast<int> (actual) < static_cast<int> (test);
    return actual.toString ().compareIgnoreCase (test.toString ()) < 0;
}

bool Condition::isAllowed (const juce::var& test, const juce::var& actual) const
{
    // the test value will be a comma-separated lists of strings; actual is a single string.
    const auto testValues { juce::StringArray::fromTokens (test.toString (), ",", "") };
    return testValues.contains (actual.toString ());
}
} // namespace cello::utils

#if RUN_UNIT_TESTS
#include "test/test_cello_utils_flags.inl"
#endif
