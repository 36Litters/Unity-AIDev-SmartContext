using UnityEngine;
using UnityEngine.AI;

[RequireComponent(typeof(NavMeshAgent))]
public class EnemyAI : MonoBehaviour
{
    [Header("AI Settings")]
    [SerializeField] private float detectionRange = 10.0f;
    [SerializeField] private float attackRange = 2.0f;
    [SerializeField] private float patrolSpeed = 2.0f;
    [SerializeField] private float chaseSpeed = 4.0f;
    
    [Header("Patrol Settings")]
    [SerializeField] private Transform[] patrolPoints;
    [SerializeField] private float waitTime = 2.0f;
    
    private NavMeshAgent m_agent;
    private Transform m_player;
    private int m_currentPatrolIndex;
    private float m_waitTimer;
    
    private enum AIState
    {
        Patrolling,
        Chasing,
        Attacking,
        Returning
    }
    
    private AIState m_currentState;
    
    void Awake()
    {
        m_agent = GetComponent<NavMeshAgent>();
        m_player = GameObject.FindGameObjectWithTag("Player")?.transform;
        
        if (m_agent == null)
        {
            Debug.LogError("EnemyAI requires a NavMeshAgent component!");
        }
    }
    
    void Start()
    {
        m_currentState = AIState.Patrolling;
        m_currentPatrolIndex = 0;
        m_waitTimer = 0f;
        
        if (patrolPoints.Length > 0)
        {
            SetDestination(patrolPoints[0].position);
        }
        
        SetSpeed(patrolSpeed);
    }
    
    void Update()
    {
        switch (m_currentState)
        {
            case AIState.Patrolling:
                HandlePatrolling();
                break;
            case AIState.Chasing:
                HandleChasing();
                break;
            case AIState.Attacking:
                HandleAttacking();
                break;
            case AIState.Returning:
                HandleReturning();
                break;
        }
        
        CheckPlayerDetection();
    }
    
    private void HandlePatrolling()
    {
        if (!m_agent.pathPending && m_agent.remainingDistance < 0.5f)
        {
            m_waitTimer += Time.deltaTime;
            
            if (m_waitTimer >= waitTime)
            {
                MoveToNextPatrolPoint();
                m_waitTimer = 0f;
            }
        }
    }
    
    private void HandleChasing()
    {
        if (m_player != null)
        {
            float distanceToPlayer = Vector3.Distance(transform.position, m_player.position);
            
            if (distanceToPlayer <= attackRange)
            {
                ChangeState(AIState.Attacking);
            }
            else if (distanceToPlayer > detectionRange)
            {
                ChangeState(AIState.Returning);
            }
            else
            {
                SetDestination(m_player.position);
            }
        }
    }
    
    private void HandleAttacking()
    {
        if (m_player != null)
        {
            transform.LookAt(m_player);
            
            float distanceToPlayer = Vector3.Distance(transform.position, m_player.position);
            
            if (distanceToPlayer > attackRange)
            {
                ChangeState(AIState.Chasing);
            }
        }
    }
    
    private void HandleReturning()
    {
        if (!m_agent.pathPending && m_agent.remainingDistance < 0.5f)
        {
            ChangeState(AIState.Patrolling);
        }
    }
    
    private void CheckPlayerDetection()
    {
        if (m_player != null && m_currentState == AIState.Patrolling)
        {
            float distanceToPlayer = Vector3.Distance(transform.position, m_player.position);
            
            if (distanceToPlayer <= detectionRange)
            {
                ChangeState(AIState.Chasing);
            }
        }
    }
    
    private void ChangeState(AIState newState)
    {
        m_currentState = newState;
        
        switch (newState)
        {
            case AIState.Patrolling:
                SetSpeed(patrolSpeed);
                if (patrolPoints.Length > 0)
                {
                    SetDestination(patrolPoints[m_currentPatrolIndex].position);
                }
                break;
                
            case AIState.Chasing:
                SetSpeed(chaseSpeed);
                break;
                
            case AIState.Attacking:
                m_agent.isStopped = true;
                break;
                
            case AIState.Returning:
                SetSpeed(patrolSpeed);
                if (patrolPoints.Length > 0)
                {
                    SetDestination(patrolPoints[m_currentPatrolIndex].position);
                }
                break;
        }
    }
    
    private void MoveToNextPatrolPoint()
    {
        if (patrolPoints.Length == 0) return;
        
        m_currentPatrolIndex = (m_currentPatrolIndex + 1) % patrolPoints.Length;
        SetDestination(patrolPoints[m_currentPatrolIndex].position);
    }
    
    private void SetDestination(Vector3 destination)
    {
        if (m_agent.isActiveAndEnabled)
        {
            m_agent.isStopped = false;
            m_agent.SetDestination(destination);
        }
    }
    
    private void SetSpeed(float speed)
    {
        if (m_agent != null)
        {
            m_agent.speed = speed;
        }
    }
    
    void OnDrawGizmosSelected()
    {
        // Draw detection range
        Gizmos.color = Color.yellow;
        Gizmos.DrawWireSphere(transform.position, detectionRange);
        
        // Draw attack range
        Gizmos.color = Color.red;
        Gizmos.DrawWireSphere(transform.position, attackRange);
        
        // Draw patrol points
        if (patrolPoints != null)
        {
            Gizmos.color = Color.blue;
            for (int i = 0; i < patrolPoints.Length; i++)
            {
                if (patrolPoints[i] != null)
                {
                    Gizmos.DrawSphere(patrolPoints[i].position, 0.3f);
                    
                    if (i < patrolPoints.Length - 1 && patrolPoints[i + 1] != null)
                    {
                        Gizmos.DrawLine(patrolPoints[i].position, patrolPoints[i + 1].position);
                    }
                    else if (i == patrolPoints.Length - 1 && patrolPoints.Length > 1 && patrolPoints[0] != null)
                    {
                        Gizmos.DrawLine(patrolPoints[i].position, patrolPoints[0].position);
                    }
                }
            }
        }
    }
}