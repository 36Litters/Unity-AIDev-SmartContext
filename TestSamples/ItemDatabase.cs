using UnityEngine;
using System.Collections.Generic;

[CreateAssetMenu(fileName = "ItemDatabase", menuName = "Game/ItemDatabase")]
public class ItemDatabase : ScriptableObject
{
    [System.Serializable]
    public class ItemInfo
    {
        public string itemName;
        public int itemId;
        public Sprite icon;
        public GameObject prefab;
        public float value;
        public string description;
        
        [Header("Item Properties")]
        public ItemType type;
        public Rarity rarity;
        
        public bool IsValid()
        {
            return !string.IsNullOrEmpty(itemName) && itemId > 0;
        }
    }

    public enum ItemType
    {
        Weapon,
        Armor,
        Consumable,
        Quest,
        Misc
    }

    public enum Rarity
    {
        Common,
        Uncommon,
        Rare,
        Epic,
        Legendary
    }

    [Header("Item Configuration")]
    public List<ItemInfo> allItems = new List<ItemInfo>();
    
    [Header("Loot Tables")]
    public ItemInfo[] commonItems;
    public ItemInfo[] rareItems;

    // Lookup methods
    public ItemInfo GetItemById(int id)
    {
        foreach (var item in allItems)
        {
            if (item.itemId == id)
                return item;
        }
        return null;
    }

    public ItemInfo GetItemByName(string name)
    {
        foreach (var item in allItems)
        {
            if (item.itemName == name)
                return item;
        }
        return null;
    }

    public ItemInfo[] GetItemsByType(ItemType type)
    {
        var results = new List<ItemInfo>();
        foreach (var item in allItems)
        {
            if (item.type == type)
                results.Add(item);
        }
        return results.ToArray();
    }

    public ItemInfo GetRandomItemByRarity(Rarity rarity)
    {
        var candidates = GetItemsByRarity(rarity);
        if (candidates.Length > 0)
        {
            return candidates[Random.Range(0, candidates.Length)];
        }
        return null;
    }

    private ItemInfo[] GetItemsByRarity(Rarity rarity)
    {
        var results = new List<ItemInfo>();
        foreach (var item in allItems)
        {
            if (item.rarity == rarity)
                results.Add(item);
        }
        return results.ToArray();
    }

    // Validation
    private void OnValidate()
    {
        // Remove invalid items
        allItems.RemoveAll(item => item == null || !item.IsValid());
        
        // Ensure unique IDs
        var usedIds = new HashSet<int>();
        foreach (var item in allItems)
        {
            if (usedIds.Contains(item.itemId))
            {
                Debug.LogWarning($"Duplicate item ID found: {item.itemId}");
            }
            usedIds.Add(item.itemId);
        }
    }
}