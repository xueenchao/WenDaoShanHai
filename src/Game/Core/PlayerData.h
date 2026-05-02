/**
 * PlayerData.h - 玩家持久数据
 *
 * 玩家数据在场景间共享（大世界↔战斗），
 * 保存在main.cpp的全局变量中，通过指针传递给各场景。
 */

#ifndef PLAYERDATA_H
#define PLAYERDATA_H

#include "GameDataTypes.h"
#include <string>
#include <vector>

struct PlayerData {
    std::string name = "青云修士";
    int level = 1;
    int xp = 0;

    int hp = 120;
    int maxHP = 120;
    int attack = 20;
    int defense = 8;
    int speed = 12;
    int spiritPower = 6;
    int maxSpiritPower = 6;
    int movementPoints = 4;
    int maxMovementPoints = 4;

    Element element = Element::Jin;
    std::vector<Skill> skills;

    std::vector<Item> inventory;
    static constexpr int MAX_INVENTORY = 20;

    Item* weapon = nullptr;
    Item* armor = nullptr;
    Item* accessory = nullptr;

    // 本命法宝（唯一，终身绑定）
    LifeboundTreasure* lifeboundTreasure = nullptr;

    // 符箓
    std::vector<Talisman> talismanInventory;
    static constexpr int MAX_TALISMANS = 20;
    std::vector<Talisman> equippedTalismans;
    static constexpr int MAX_EQUIPPED_TALISMANS = 5;

    // 门派
    std::string sectId;
    std::string sectName;

    int getEffectiveAttack() const
    {
        int atk = attack;
        if (weapon) atk += weapon->atkBonus;
        return atk;
    }

    int getEffectiveDefense() const
    {
        int def = defense;
        if (armor) def += armor->defBonus;
        return def;
    }

    int getEffectiveMaxHP() const
    {
        int hp = maxHP;
        if (armor) hp += armor->hpBonus;
        if (accessory) hp += accessory->hpBonus;
        return hp;
    }

    bool addItem(const Item& item)
    {
        for (auto& invItem : inventory) {
            if (invItem.id == item.id && invItem.type != ItemType::Equipment
                && invItem.quantity < invItem.maxStack) {
                invItem.quantity += item.quantity;
                return true;
            }
        }
        if (static_cast<int>(inventory.size()) >= MAX_INVENTORY) {
            return false; // 背包已满
        }
        inventory.push_back(item);
        return true;
    }

    void removeItem(int index)
    {
        if (index >= 0 && index < static_cast<int>(inventory.size())) {
            inventory.erase(inventory.begin() + index);
        }
    }

    void equipItem(int index)
    {
        if (index < 0 || index >= static_cast<int>(inventory.size())) return;
        Item& item = inventory[index];
        if (item.type != ItemType::Equipment) return;

        Item** slot = nullptr;
        switch (item.equipSlot) {
        case EquipSlot::Weapon:   slot = &weapon; break;
        case EquipSlot::Armor:    slot = &armor; break;
        case EquipSlot::Accessory:slot = &accessory; break;
        }
        if (!slot) return;

        // 卸下旧装备
        if (*slot) {
            addItem(**slot);
            *slot = nullptr;
        }
        *slot = new Item(item);
        removeItem(index);
    }

    void unequipSlot(EquipSlot slot)
    {
        Item** ptr = nullptr;
        switch (slot) {
        case EquipSlot::Weapon:   ptr = &weapon; break;
        case EquipSlot::Armor:    ptr = &armor; break;
        case EquipSlot::Accessory:ptr = &accessory; break;
        }
        if (ptr && *ptr) {
            addItem(**ptr);
            delete *ptr;
            *ptr = nullptr;
        }
    }

    void heal(int amount)
    {
        hp += amount;
        if (hp > maxHP) hp = maxHP;
    }

    void restoreSP(int amount)
    {
        spiritPower += amount;
        if (spiritPower > maxSpiritPower) spiritPower = maxSpiritPower;
    }

    int getEffectiveSpeed() const
    {
        int spd = speed;
        if (lifeboundTreasure) spd += lifeboundTreasure->getScaledSpeed();
        return spd;
    }

    bool bindLifeboundTreasure(const LifeboundTreasure& lt)
    {
        if (lifeboundTreasure) return false; // 已绑定，不可更换
        lifeboundTreasure = new LifeboundTreasure(lt);
        lifeboundTreasure->level = 1;
        return true;
    }

    bool addTalisman(const Talisman& t)
    {
        for (auto& invT : talismanInventory) {
            if (invT.id == t.id && invT.quantity < 99) {
                invT.quantity += t.quantity;
                return true;
            }
        }
        if (static_cast<int>(talismanInventory.size()) >= MAX_TALISMANS) {
            return false;
        }
        talismanInventory.push_back(t);
        return true;
    }

    void removeTalisman(int index)
    {
        if (index >= 0 && index < static_cast<int>(talismanInventory.size())) {
            talismanInventory.erase(talismanInventory.begin() + index);
        }
    }
};

#endif // PLAYERDATA_H
