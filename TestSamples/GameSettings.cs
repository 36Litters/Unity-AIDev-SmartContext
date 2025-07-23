using UnityEngine;

[CreateAssetMenu(fileName = "GameSettings", menuName = "Game/Settings")]
public class GameSettings : ScriptableObject
{
    [Header("Player Settings")]
    [SerializeField]
    private float defaultMoveSpeed = 5.0f;
    
    [SerializeField]
    private int maxHealth = 100;
    
    [SerializeField]
    private float jumpPower = 10.0f;

    [Header("Game Balance")]
    [Range(0.1f, 5.0f)]
    public float difficultyMultiplier = 1.0f;
    
    [Range(1, 10)]
    public int maxEnemies = 5;

    [Header("Audio Settings")]
    public AudioClip backgroundMusic;
    public AudioClip[] soundEffects;

    [Header("Visual Effects")]
    public GameObject explosionPrefab;
    public Material[] playerMaterials;

    // Public properties for accessing private fields
    public float DefaultMoveSpeed => defaultMoveSpeed;
    public int MaxHealth => maxHealth;
    public float JumpPower => jumpPower;

    // Validation method
    private void OnValidate()
    {
        // Ensure values are within reasonable ranges
        defaultMoveSpeed = Mathf.Max(0.1f, defaultMoveSpeed);
        maxHealth = Mathf.Max(1, maxHealth);
        jumpPower = Mathf.Max(0.1f, jumpPower);
        maxEnemies = Mathf.Clamp(maxEnemies, 1, 20);
    }

    // Configuration methods
    public void ApplyToPlayer(PlayerController player)
    {
        // Apply settings to player - this creates a dependency
        if (player != null)
        {
            // This would be a method call dependency
        }
    }

    public bool IsValidConfiguration()
    {
        return defaultMoveSpeed > 0 && maxHealth > 0 && jumpPower > 0;
    }
}