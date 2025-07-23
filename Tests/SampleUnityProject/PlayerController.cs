using UnityEngine;

[RequireComponent(typeof(Rigidbody))]
[RequireComponent(typeof(Collider))]
public class PlayerController : MonoBehaviour
{
    [Header("Movement Settings")]
    [SerializeField] private float moveSpeed = 5.0f;
    [SerializeField] private float jumpForce = 10.0f;
    [SerializeField] private LayerMask groundLayer = 1;
    
    [Header("Input Settings")]
    [SerializeField] private KeyCode jumpKey = KeyCode.Space;
    
    private Rigidbody m_rigidbody;
    private Collider m_collider;
    private bool m_isGrounded;
    private Vector3 m_moveDirection;
    
    void Awake()
    {
        m_rigidbody = GetComponent<Rigidbody>();
        m_collider = GetComponent<Collider>();
        
        if (m_rigidbody == null)
        {
            Debug.LogError("PlayerController requires a Rigidbody component!");
        }
    }
    
    void Start()
    {
        // Initialize player state
        m_isGrounded = false;
        m_moveDirection = Vector3.zero;
    }
    
    void Update()
    {
        HandleInput();
        CheckGroundStatus();
    }
    
    void FixedUpdate()
    {
        ApplyMovement();
    }
    
    private void HandleInput()
    {
        float horizontal = Input.GetAxis("Horizontal");
        float vertical = Input.GetAxis("Vertical");
        
        m_moveDirection = new Vector3(horizontal, 0, vertical).normalized;
        
        if (Input.GetKeyDown(jumpKey) && m_isGrounded)
        {
            Jump();
        }
    }
    
    private void ApplyMovement()
    {
        if (m_moveDirection.magnitude > 0.1f)
        {
            Vector3 targetVelocity = m_moveDirection * moveSpeed;
            targetVelocity.y = m_rigidbody.velocity.y;
            m_rigidbody.velocity = targetVelocity;
        }
    }
    
    private void Jump()
    {
        m_rigidbody.AddForce(Vector3.up * jumpForce, ForceMode.Impulse);
        m_isGrounded = false;
    }
    
    private void CheckGroundStatus()
    {
        float rayDistance = m_collider.bounds.extents.y + 0.1f;
        m_isGrounded = Physics.Raycast(transform.position, Vector3.down, rayDistance, groundLayer);
    }
    
    void OnCollisionEnter(Collision collision)
    {
        if (((1 << collision.gameObject.layer) & groundLayer) != 0)
        {
            m_isGrounded = true;
        }
    }
    
    void OnDrawGizmosSelected()
    {
        if (m_collider != null)
        {
            Gizmos.color = m_isGrounded ? Color.green : Color.red;
            float rayDistance = m_collider.bounds.extents.y + 0.1f;
            Gizmos.DrawRay(transform.position, Vector3.down * rayDistance);
        }
    }
}