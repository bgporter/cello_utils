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

#pragma once

#include <cello/cello/cello_object.h>
namespace cello::utils
{
/**
 * @brief Base class for your set of flags and runtime config options.
 *
 * You'll want to derive a class from this that adds a cello::Value
 * for each of the things you need to set at runtime.
 */
class Flags : public cello::Object
{
public:
    Flags (cello::Object* root, const juce::String& type = "flags")
    : cello::Object { type, root }
    {
    }
};

/**
 * @brief Context class that contains user and platform information
 * that will be used to convert the flag rules into a set of flags
 * for the current execution of the application.
 */
class Context : public cello::Object
{
public:
    Context (const juce::ValueTree& tree = {})
    : cello::Object { "context", tree }
    {
    }
};

/**
 * @brief The current set of rules to be used to determine the settings
 * of each flag based on the current user, platform, date, and other
 * dynamic factors.
 *
 */
class Rules : public cello::Object
{
public:
    Rules (const juce::ValueTree& tree)
    : cello::Object { "rules", tree }
    {
    }

    /**
     * @brief Evaluate the rules in the context of the current runtime
     * user data.
     * After processing, the flags object will contain the current set
     * of flags that should be used for the current execution of the
     * application based on the combination of the rules and the current
     * application context.
     *
     * @param context
     * @param flags
     */
    void evaluate (const Context& context, Flags& flags) const;
};

class Condition : public cello::Object
{
public:
    /**
     * @brief Wrap the tree object, which should/must be of type "Condition"
     * e.g.:
     *
     * 	<condition>
     *      <!-- these types of users get access -->
     *      <type allowed="dev,int,beta" />
     *  </condition>
     *  <condition>
     *       <!-- 1st half of users get access for a week starting jan 1 -->
     *       <cohort min="0" max="50"/>
     *       <time min="2024-01-01" max="2024-01-07"/>
     *  </condition>
     *
     * @param tree
     */
    Condition (const juce::ValueTree& tree)
    : cello::Object { "condition", tree }
    {
    }

    /**
     * @brief Evaluate this condition in the context of the current
     * set of runtime user data.
     *
     * @param context
     * @return juce::var -- will return a void var to indicate that the condition
     * test failed, and processing should continue.
     */
    juce::var evaluate (const Context& context) const;

private:
    /**
     * @brief if this condition includes a custom 'result' value, return that,
     * otherwise just return true.
     *
     * @return juce::var
     */
    juce::var result () const { return data.getProperty ("result", true); }

    bool isAboveMin (const juce::var& test, const juce::var& actual) const;
    bool isBelowMax (const juce::var& test, const juce::var& actual) const;
    bool isAllowed (const juce::var& test, const juce::var& actual) const;
};

} // namespace cello::utils
