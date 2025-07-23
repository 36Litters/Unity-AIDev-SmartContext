using UnityEngine;
using UnityEngine.SceneManagement;

public class GameManager : MonoBehaviour
{
    [Header("Game Settings")]
    [SerializeField] private int maxLives = 3;
    [SerializeField] private float gameTimeLimit = 300f; // 5 minutes
    
    [Header("UI References")]
    [SerializeField] private UIController uiController;
    
    [Header("Player References")]
    [SerializeField] private PlayerController playerController;
    
    private static GameManager s_instance;
    
    private int m_currentLives;
    private float m_gameTimer;
    private int m_score;
    private bool m_gameActive;
    private bool m_gamePaused;
    
    public static GameManager Instance
    {
        get
        {
            if (s_instance == null)
            {
                s_instance = FindObjectOfType<GameManager>();
                
                if (s_instance == null)
                {
                    GameObject go = new GameObject("GameManager");
                    s_instance = go.AddComponent<GameManager>();
                    DontDestroyOnLoad(go);
                }
            }
            return s_instance;
        }
    }
    
    public int CurrentLives => m_currentLives;
    public float GameTimer => m_gameTimer;
    public int Score => m_score;
    public bool IsGameActive => m_gameActive;
    public bool IsGamePaused => m_gamePaused;
    
    public event System.Action<int> OnLivesChanged;
    public event System.Action<int> OnScoreChanged;
    public event System.Action<float> OnTimerChanged;
    public event System.Action OnGameOver;
    public event System.Action OnGameWon;
    public event System.Action<bool> OnGamePaused;
    
    void Awake()
    {
        if (s_instance == null)
        {
            s_instance = this;
            DontDestroyOnLoad(gameObject);
        }
        else if (s_instance != this)
        {
            Destroy(gameObject);
            return;
        }
        
        InitializeGame();
    }
    
    void Start()
    {
        if (uiController == null)
        {
            uiController = FindObjectOfType<UIController>();
        }
        
        if (playerController == null)
        {
            playerController = FindObjectOfType<PlayerController>();
        }
        
        StartGame();
    }
    
    void Update()
    {
        if (m_gameActive && !m_gamePaused)
        {
            UpdateGameTimer();
            HandlePauseInput();
        }
        
        if (Input.GetKeyDown(KeyCode.Escape))
        {
            TogglePause();
        }
    }
    
    private void InitializeGame()
    {
        m_currentLives = maxLives;
        m_gameTimer = gameTimeLimit;
        m_score = 0;
        m_gameActive = false;
        m_gamePaused = false;
    }
    
    public void StartGame()
    {
        m_gameActive = true;
        m_gamePaused = false;
        
        OnLivesChanged?.Invoke(m_currentLives);
        OnScoreChanged?.Invoke(m_score);
        OnTimerChanged?.Invoke(m_gameTimer);
        
        Debug.Log("Game Started!");
    }
    
    public void EndGame()
    {
        m_gameActive = false;
        OnGameOver?.Invoke();
        
        Debug.Log("Game Over!");
    }
    
    public void WinGame()
    {
        m_gameActive = false;
        OnGameWon?.Invoke();
        
        Debug.Log("Game Won!");
    }
    
    public void RestartGame()
    {
        InitializeGame();
        SceneManager.LoadScene(SceneManager.GetActiveScene().name);
    }
    
    public void LoadMainMenu()
    {
        Time.timeScale = 1f;
        SceneManager.LoadScene("MainMenu");
    }
    
    public void QuitGame()
    {
        #if UNITY_EDITOR
            UnityEditor.EditorApplication.isPlaying = false;
        #else
            Application.Quit();
        #endif
    }
    
    public void TogglePause()
    {
        m_gamePaused = !m_gamePaused;
        Time.timeScale = m_gamePaused ? 0f : 1f;
        
        OnGamePaused?.Invoke(m_gamePaused);
        
        Debug.Log(m_gamePaused ? "Game Paused" : "Game Resumed");
    }
    
    public void AddScore(int points)
    {
        if (!m_gameActive) return;
        
        m_score += points;
        OnScoreChanged?.Invoke(m_score);
        
        Debug.Log($"Score added: {points}. Total: {m_score}");
    }
    
    public void LoseLife()
    {
        if (!m_gameActive) return;
        
        m_currentLives--;
        OnLivesChanged?.Invoke(m_currentLives);
        
        if (m_currentLives <= 0)
        {
            EndGame();
        }
        else
        {
            RespawnPlayer();
        }
        
        Debug.Log($"Life lost. Lives remaining: {m_currentLives}");
    }
    
    public void AddLife()
    {
        if (!m_gameActive) return;
        
        m_currentLives++;
        OnLivesChanged?.Invoke(m_currentLives);
        
        Debug.Log($"Life gained. Lives: {m_currentLives}");
    }
    
    private void UpdateGameTimer()
    {
        m_gameTimer -= Time.deltaTime;
        OnTimerChanged?.Invoke(m_gameTimer);
        
        if (m_gameTimer <= 0)
        {
            m_gameTimer = 0;
            EndGame();
        }
    }
    
    private void HandlePauseInput()
    {
        if (Input.GetKeyDown(KeyCode.P) || Input.GetKeyDown(KeyCode.Pause))
        {
            TogglePause();
        }
    }
    
    private void RespawnPlayer()
    {
        if (playerController != null)
        {
            // Reset player position and state
            GameObject respawnPoint = GameObject.FindGameObjectWithTag("Respawn");
            if (respawnPoint != null)
            {
                playerController.transform.position = respawnPoint.transform.position;
                playerController.transform.rotation = respawnPoint.transform.rotation;
            }
        }
    }
    
    public void SetTimeLimit(float newTimeLimit)
    {
        gameTimeLimit = newTimeLimit;
        m_gameTimer = gameTimeLimit;
        OnTimerChanged?.Invoke(m_gameTimer);
    }
    
    public void SetMaxLives(int newMaxLives)
    {
        maxLives = newMaxLives;
        m_currentLives = maxLives;
        OnLivesChanged?.Invoke(m_currentLives);
    }
    
    void OnApplicationFocus(bool hasFocus)
    {
        if (!hasFocus && m_gameActive && !m_gamePaused)
        {
            TogglePause();
        }
    }
    
    void OnApplicationPause(bool pauseStatus)
    {
        if (pauseStatus && m_gameActive && !m_gamePaused)
        {
            TogglePause();
        }
    }
}