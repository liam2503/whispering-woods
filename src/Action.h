#pragma once

#include <string>
#include "VectorPP.h"

class Action
{
private:
    std::string m_strName;
    std::string m_strState; // START, END
    VectorPP m_vMousePos = VectorPP(0, 0);

public:
    // --- Constructors ---
    Action();
    Action(const std::string &a_strName, const std::string &a_strType);
    Action(const std::string &a_strName, const std::string &a_strType, const VectorPP &a_vMousePos);

    // --- Getters ---
    const std::string &getName() const;
    const std::string &getState() const;
    const VectorPP &getPos() const;
};