#include "Skill.h"

Skill Skill::makeFireball()
{
    Skill s;
    s.mName = "火球术";
    s.mDescription = "释放火球攻击单体敌人";
    s.mSpiritCost = 2;
    s.mRange = 4;
    s.mAreaType = SkillAreaType::Single;
    s.mBaseDamage = 30;
    s.mCooldown = 1;
    s.mElementId = static_cast<int>(Element::Huo);
    return s;
}

Skill Skill::makeIceShard()
{
    Skill s;
    s.mName = "冰锥术";
    s.mDescription = "凝聚冰锥攻击单体敌人";
    s.mSpiritCost = 2;
    s.mRange = 3;
    s.mAreaType = SkillAreaType::Single;
    s.mBaseDamage = 25;
    s.mCooldown = 0;
    s.mElementId = static_cast<int>(Element::Shui);
    return s;
}

Skill Skill::makeSwordSlash()
{
    Skill s;
    s.mName = "剑气斩";
    s.mDescription = "剑气横扫十字范围";
    s.mSpiritCost = 3;
    s.mRange = 1;
    s.mAreaType = SkillAreaType::Cross;
    s.mBaseDamage = 35;
    s.mCooldown = 2;
    s.mElementId = static_cast<int>(Element::Jin);
    return s;
}

Skill Skill::makeRockSmash()
{
    Skill s;
    s.mName = "岩崩";
    s.mDescription = "召唤岩石攻击3x3区域";
    s.mSpiritCost = 4;
    s.mRange = 3;
    s.mAreaType = SkillAreaType::Square;
    s.mBaseDamage = 20;
    s.mCooldown = 3;
    s.mElementId = static_cast<int>(Element::Tu);
    return s;
}

Skill Skill::makeVineWhip()
{
    Skill s;
    s.mName = "藤鞭";
    s.mDescription = "木系单体攻击";
    s.mSpiritCost = 1;
    s.mRange = 2;
    s.mAreaType = SkillAreaType::Single;
    s.mBaseDamage = 18;
    s.mCooldown = 0;
    s.mElementId = static_cast<int>(Element::Mu);
    return s;
}
