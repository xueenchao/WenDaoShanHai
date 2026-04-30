/**
 * Item.h - 物品系统
 *
 * 定义物品类型、装备槽位和物品数据结构。
 */

#ifndef ITEM_H
#define ITEM_H

#include <string>

enum class ItemType { Consumable, Equipment, Material, Key };
enum class EquipSlot { Weapon, Armor, Accessory };

struct Item {
    std::string id;
    std::string name;
    std::string desc;
    ItemType type = ItemType::Consumable;
    int quantity = 1;
    int maxStack = 99;

    EquipSlot equipSlot = EquipSlot::Weapon;
    int atkBonus = 0;
    int defBonus = 0;
    int hpBonus = 0;

    int healHP = 0;
    int healSP = 0;
};

#endif // ITEM_H
