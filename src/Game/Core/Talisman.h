/**
 * Talisman.h - 符箓
 *
 * 一次性神通卷轴，不占装备槽。
 * 每战最多携带5张，使用后消耗。
 */

#ifndef TALISMAN_H
#define TALISMAN_H

#include <string>

enum class Element;

enum class TalismanType { Attack, Defense, Support, Summon };

struct Talisman {
    std::string id;
    std::string name;
    std::string desc;
    TalismanType type = TalismanType::Attack;
    int value = 0;
    Element element;
    int quantity = 1;

    const char* typeName() const
    {
        switch (type) {
        case TalismanType::Attack:  return "攻击";
        case TalismanType::Defense: return "防御";
        case TalismanType::Support: return "辅助";
        case TalismanType::Summon:  return "召唤";
        }
        return "未知";
    }
};

#endif // TALISMAN_H
