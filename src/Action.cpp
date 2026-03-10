#include "Action.h"
#include <sstream>

Action::Action() {}

Action::Action(const std::string &a_strName, const std::string &a_strType)
    : m_strName(a_strName), m_strState(a_strType) {}

Action::Action(const std::string &a_strName, const std::string &a_strType, const VectorPP &a_vMousePos)
    : m_strName(a_strName), m_strState(a_strType), m_vMousePos(a_vMousePos) {}

const std::string &Action::getName() const { return m_strName; }

const std::string &Action::getState() const { return m_strState; }

const VectorPP &Action::getPos() const { return m_vMousePos; }