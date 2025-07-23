using UnityEngine;
using UnityEngine.UI;
using TMPro;

public class UIController : MonoBehaviour
{
    [Header("HUD Elements")]
    [SerializeField] private TextMeshProUGUI scoreText;
    [SerializeField] private TextMeshProUGUI livesText;
    [SerializeField] private TextMeshProUGUI timerText;
    [SerializeField] private Slider healthBar;
    
    [Header("Menu Panels")]
    [SerializeField] private GameObject pauseMenu;
    [SerializeField] private GameObject gameOverPanel;
    [SerializeField] private GameObject gameWonPanel;
    [SerializeField] private GameObject mainHUD;
    
    [Header("Buttons")]
    [SerializeField] private Button resumeButton;
    [SerializeField] private Button restartButton;
    [SerializeField] private Button mainMenuButton;
    [SerializeField] private Button quitButton;
    
    [Header("Settings")]
    [SerializeField] private bool hideHUDOnPause = true;
    
    private GameManager m_gameManager;
    private bool m_isInitialized;
    
    void Awake()
    {
        InitializeReferences();
        SetupButtonListeners();
    }
    
    void Start()
    {
        m_gameManager = GameManager.Instance;
        
        if (m_gameManager != null)
        {
            SubscribeToGameEvents();
            UpdateAllUI();
            m_isInitialized = true;
        }
        else
        {
            Debug.LogError("UIController: GameManager instance not found!");
        }
        
        ShowMainHUD();
    }
    
    void OnDestroy()
    {
        UnsubscribeFromGameEvents();
    }
    
    private void InitializeReferences()
    {
        if (scoreText == null)
            scoreText = GameObject.Find("ScoreText")?.GetComponent<TextMeshProUGUI>();
        
        if (livesText == null)
            livesText = GameObject.Find("LivesText")?.GetComponent<TextMeshProUGUI>();
        
        if (timerText == null)
            timerText = GameObject.Find("TimerText")?.GetComponent<TextMeshProUGUI>();
        
        if (healthBar == null)
            healthBar = GameObject.Find("HealthBar")?.GetComponent<Slider>();
    }
    
    private void SetupButtonListeners()
    {
        if (resumeButton != null)
            resumeButton.onClick.AddListener(ResumeGame);
        
        if (restartButton != null)
            restartButton.onClick.AddListener(RestartGame);
        
        if (mainMenuButton != null)
            mainMenuButton.onClick.AddListener(LoadMainMenu);
        
        if (quitButton != null)
            quitButton.onClick.AddListener(QuitGame);
    }
    
    private void SubscribeToGameEvents()
    {
        if (m_gameManager == null) return;
        
        m_gameManager.OnScoreChanged += UpdateScore;
        m_gameManager.OnLivesChanged += UpdateLives;
        m_gameManager.OnTimerChanged += UpdateTimer;
        m_gameManager.OnGamePaused += HandleGamePause;
        m_gameManager.OnGameOver += ShowGameOverPanel;
        m_gameManager.OnGameWon += ShowGameWonPanel;
    }
    
    private void UnsubscribeFromGameEvents()
    {
        if (m_gameManager == null) return;
        
        m_gameManager.OnScoreChanged -= UpdateScore;
        m_gameManager.OnLivesChanged -= UpdateLives;
        m_gameManager.OnTimerChanged -= UpdateTimer;
        m_gameManager.OnGamePaused -= HandleGamePause;
        m_gameManager.OnGameOver -= ShowGameOverPanel;
        m_gameManager.OnGameWon -= ShowGameWonPanel;
    }
    
    private void UpdateScore(int newScore)
    {
        if (scoreText != null)
        {
            scoreText.text = $"Score: {newScore:N0}";
        }
    }
    
    private void UpdateLives(int newLives)
    {
        if (livesText != null)
        {
            livesText.text = $"Lives: {newLives}";
        }
    }
    
    private void UpdateTimer(float timeRemaining)
    {
        if (timerText != null)
        {
            int minutes = Mathf.FloorToInt(timeRemaining / 60f);
            int seconds = Mathf.FloorToInt(timeRemaining % 60f);
            timerText.text = $"Time: {minutes:00}:{seconds:00}";
            
            // Change color when time is running low
            if (timeRemaining <= 30f)
            {
                timerText.color = Color.red;
            }
            else if (timeRemaining <= 60f)
            {
                timerText.color = Color.yellow;
            }
            else
            {
                timerText.color = Color.white;
            }
        }
    }
    
    public void UpdateHealth(float healthPercentage)
    {
        if (healthBar != null)
        {
            healthBar.value = healthPercentage;
            
            // Change health bar color based on health level
            Image fillImage = healthBar.fillRect.GetComponent<Image>();
            if (fillImage != null)
            {
                if (healthPercentage > 0.6f)
                    fillImage.color = Color.green;
                else if (healthPercentage > 0.3f)
                    fillImage.color = Color.yellow;
                else
                    fillImage.color = Color.red;
            }
        }
    }
    
    private void HandleGamePause(bool isPaused)
    {
        if (isPaused)
        {
            ShowPauseMenu();
        }
        else
        {
            HidePauseMenu();
        }
    }
    
    private void ShowMainHUD()
    {
        SetPanelActive(mainHUD, true);
        SetPanelActive(pauseMenu, false);
        SetPanelActive(gameOverPanel, false);
        SetPanelActive(gameWonPanel, false);
    }
    
    private void ShowPauseMenu()
    {
        SetPanelActive(pauseMenu, true);
        
        if (hideHUDOnPause)
        {
            SetPanelActive(mainHUD, false);
        }
    }
    
    private void HidePauseMenu()
    {
        SetPanelActive(pauseMenu, false);
        SetPanelActive(mainHUD, true);
    }
    
    private void ShowGameOverPanel()
    {
        SetPanelActive(gameOverPanel, true);
        SetPanelActive(mainHUD, false);
        SetPanelActive(pauseMenu, false);
    }
    
    private void ShowGameWonPanel()
    {
        SetPanelActive(gameWonPanel, true);
        SetPanelActive(mainHUD, false);
        SetPanelActive(pauseMenu, false);
    }
    
    private void SetPanelActive(GameObject panel, bool active)
    {
        if (panel != null)
        {
            panel.SetActive(active);
        }
    }
    
    private void UpdateAllUI()
    {
        if (m_gameManager == null) return;
        
        UpdateScore(m_gameManager.Score);
        UpdateLives(m_gameManager.CurrentLives);
        UpdateTimer(m_gameManager.GameTimer);
    }
    
    // Button event handlers
    private void ResumeGame()
    {
        if (m_gameManager != null)
        {
            m_gameManager.TogglePause();
        }
    }
    
    private void RestartGame()
    {
        if (m_gameManager != null)
        {
            m_gameManager.RestartGame();
        }
    }
    
    private void LoadMainMenu()
    {
        if (m_gameManager != null)
        {
            m_gameManager.LoadMainMenu();
        }
    }
    
    private void QuitGame()
    {
        if (m_gameManager != null)
        {
            m_gameManager.QuitGame();
        }
    }
    
    // Public methods for external scripts
    public void ShowNotification(string message, float duration = 3f)
    {
        StartCoroutine(ShowNotificationCoroutine(message, duration));
    }
    
    private System.Collections.IEnumerator ShowNotificationCoroutine(string message, float duration)
    {
        // Create temporary notification UI element
        GameObject notification = new GameObject("Notification");
        notification.transform.SetParent(transform);
        
        TextMeshProUGUI notificationText = notification.AddComponent<TextMeshProUGUI>();
        notificationText.text = message;
        notificationText.fontSize = 24;
        notificationText.color = Color.white;
        notificationText.alignment = TextAlignmentOptions.Center;
        
        RectTransform rectTransform = notification.GetComponent<RectTransform>();
        rectTransform.anchoredPosition = Vector2.zero;
        rectTransform.sizeDelta = new Vector2(400, 100);
        
        yield return new WaitForSeconds(duration);
        
        Destroy(notification);
    }
    
    public void SetHUDVisibility(bool visible)
    {
        SetPanelActive(mainHUD, visible);
    }
}