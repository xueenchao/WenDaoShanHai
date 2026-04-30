/**
 * LifeboundTreasure.h - 本命法宝
 *
 * 唯一、不可替换、不可卸下的终生绑定装备，
 * 随等级成长，有天赋路径选择。
 */

#ifndef LIFEBOUNDTREASURE_H
#define LIFEBOUNDTREASURE_H

#include <string>

enum class Element;

enum class TalentPath { ShaFa, ShouHu, XiaoYao }; // 杀伐/守护/逍遥

struct LifeboundTreasure {
    std::string id;
    std::string name;
    std::string desc;
    Element element;
    int level = 1;
    int maxLevel = 10;
    int atkBonus = 0;
    int defBonus = 0;
    int hpBonus = 0;
    int speedBonus = 0;
    TalentPath talentPath = TalentPath::ShaFa;

    int getScaledAtk() const { return atkBonus + (level - 1) * 2; }
    int getScaledDef() const { return defBonus + (level - 1) * 1; }
    int getScaledHP() const { return hpBonus + (level - 1) * 5; }
    int getScaledSpeed() const { return speedBonus + (level - 1) * 1; }

    void levelUp() { if (level < maxLevel) ++level; }

    const char* talentPathName() const
    {
        switch (talentPath) {
        case TalentPath::ShaFa:  return "杀伐";
        case TalentPath::ShouHu: return "守护";
        case TalentPath::XiaoYao:return "逍遥";
        }
        return "未知";
    }
};

#endif // LIFEBOUNDTREASURE_H
