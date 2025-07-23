using UnityEngine;
using UnityEngine.UI;

public class GameManager : MonoBehaviour
{
    public static GameManager Instance { get; private set; }

    [Header("UI References")]
    public Text scoreText;
    public Text healthText;
    public Button pauseButton;

    [Header("Game Settings")]
    [SerializeField]
    private int maxHealth = 100;
    
    [SerializeField]
    private float gameTime = 300.0f;

    private int currentScore;
    private int currentHealth;
    private bool isPaused;
    private PlayerController player;

    void Awake()
    {
        // Singleton pattern
        if (Instance == null)
        {
            Instance = this;
            DontDestroyOnLoad(gameObject);
        }
        else
        {
            Destroy(gameObject);
        }
    }

    void Start()
    {
        InitializeGame();
        FindReferences();
    }

    void Update()
    {
        if (!isPaused)
        {
            UpdateGameTime();
        }

        HandleInput();
    }

    private void InitializeGame()
    {
        currentHealth = maxHealth;
        currentScore = 0;
        isPaused = false;
        
        UpdateUI();
    }

    private void FindReferences()
    {
        player = FindObjectOfType<PlayerController>();
        
        if (pauseButton != null)
        {
            pauseButton.onClick.AddListener(TogglePause);
        }
    }

    private void UpdateGameTime()
    {
        gameTime -= Time.deltaTime;
        
        if (gameTime <= 0)
        {
            GameOver();
        }
    }

    private void HandleInput()
    {
        if (Input.GetKeyDown(KeyCode.Escape))
        {
            TogglePause();
        }
    }

    public void AddScore(int points)
    {
        currentScore += points;
        UpdateUI();
    }

    public void TakeDamage(int damage)
    {
        currentHealth -= damage;
        currentHealth = Mathf.Max(0, currentHealth);
        
        UpdateUI();
        
        if (currentHealth <= 0)
        {
            GameOver();
        }
    }

    private void UpdateUI()
    {
        if (scoreText != null)
            scoreText.text = "Score: " + currentScore;
            
        if (healthText != null)
            healthText.text = "Health: " + currentHealth;
    }

    private void TogglePause()
    {
        isPaused = !isPaused;
        Time.timeScale = isPaused ? 0f : 1f;
    }

    private void GameOver()
    {
        Debug.Log("Game Over! Final Score: " + currentScore);
        Time.timeScale = 0f;
    }
}