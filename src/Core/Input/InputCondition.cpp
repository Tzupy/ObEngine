#include <Input/InputCondition.hpp>
#include <Input/InputManager.hpp>
#include <Input/KeyList.hpp>
#include <Utils/StringUtils.hpp>
#include <Utils/VectorUtils.hpp>

namespace obe::Input
{
    bool InputCondition::isKeyAlreadyInCombination(InputButton* button)
    {
        for (InputCombinationElement& element : m_triggerConditions)
        {
            if (element.first->getButton() == button)
            {
                return true;
            }
        }
        return false;
    }

    void InputCondition::setCombinationCode(const std::string& code)
    {
        std::vector<std::string> elements = Utils::String::split(code, "+");
        if (code != "NotAssociated")
        {
            for (std::string element : elements)
            {
                Utils::String::replaceInPlace(element, " ", "");
                std::vector<std::string> stateAndButton =
                    Utils::String::split(element, ":");
                if (stateAndButton.size() == 1 || stateAndButton.size() == 2)
                {
                    if (stateAndButton.size() == 1)
                    {
                        stateAndButton.push_back(stateAndButton[0]);
                        stateAndButton[0] = "Pressed";
                    }

                    if (Utils::Vector::contains(
                            stateAndButton[0],
                            {"Idle", "Hold", "Pressed", "Released"}))
                    {
                        std::vector<std::string> stateList =
                            Utils::String::split(stateAndButton[0], ",");
                        std::vector<InputButtonState> buttonStates;
                        for (std::string& buttonState : stateList)
                        {
                            buttonStates.push_back(
                                stringToInputButtonState(buttonState));
                        }
                        const std::string keyId = stateAndButton[1];
                        if (AllKeys.find(keyId) != AllKeys.end())
                        {
                            InputButton* button = GetKey(keyId);
                            InputButtonMonitorPtr monitor =
                                Monitors::Monitor(button);

                            if (!isKeyAlreadyInCombination(button))
                            {
                                m_enabled = true;
                                m_triggerConditions.emplace_back(monitor,
                                                                 buttonStates);
                            }
                            else
                            {
                                throw aube::ErrorHandler::Raise(
                                    "ObEngine.Input.InputCondition."
                                    "ButtonAlreadyInCombination",
                                    {{"button", button->getName()}});
                            }
                        }
                        else
                        {
                            Debug::Log->warn("<InputCondition> Button not "
                                             "found : '{0}' in code '{1}'",
                                             keyId, code);
                            // throw
                            // aube::ErrorHandler::Raise("ObEngine.Input.InputCondition.ButtonNotFound",
                            // { { "button", keyId } });
                        }
                    }
                    else
                    {
                        throw aube::ErrorHandler::Raise(
                            "ObEngine.Input.InputCondition.UnknownState",
                            {{"state", stateAndButton[0]}});
                    }
                }
            }
        }
    }

    InputCombination InputCondition::getCombination() const
    {
        return m_triggerConditions;
    }

    void InputCondition::addCombinationElement(
        const InputCombinationElement combinationElement)
    {
        m_triggerConditions.push_back(combinationElement);
    }

    void InputCondition::setCombination(const InputCombination& combination)
    {
        m_triggerConditions = combination;
    }

    bool InputCondition::check() const
    {
        if (!m_enabled)
            return false;
        bool conditionOk = true;
        for (const InputCombinationElement& element : m_triggerConditions)
        {
            if (!Utils::Vector::contains(element.first->getState(),
                                         element.second))
            {
                conditionOk = false;
                break;
            }
        }
        return conditionOk;
    }

    void InputCondition::clear()
    {
        m_triggerConditions.clear();
        m_enabled = false;
    }
} // namespace obe::Input