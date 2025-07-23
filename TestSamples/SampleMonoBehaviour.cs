using UnityEngine;
using System.Collections.Generic;

public class PlayerController : MonoBehaviour
{
    [SerializeField]
    private float moveSpeed = 5.0f;
    
    [SerializeField]
    private Rigidbody rb;
    
    private Vector3 movement;
    private bool isGrounded;

    void Awake()
    {
        rb = GetComponent<Rigidbody>();
    }

    void Start()
    {
        Debug.Log("Player initialized");
    }

    void Update()
    {
        HandleInput();
        Move();
    }

    void FixedUpdate()
    {
        ApplyMovement();
    }

    void OnCollisionEnter(Collision collision)
    {
        if (collision.gameObject.CompareTag("Ground"))
        {
            isGrounded = true;
        }
    }

    private void HandleInput()
    {
        float horizontal = Input.GetAxis("Horizontal");
        float vertical = Input.GetAxis("Vertical");
        
        movement = new Vector3(horizontal, 0, vertical).normalized;
    }

    private void Move()
    {
        if (movement.magnitude > 0.1f)
        {
            transform.position += movement * moveSpeed * Time.deltaTime;
        }
    }

    private void ApplyMovement()
    {
        if (rb != null)
        {
            rb.velocity = new Vector3(movement.x * moveSpeed, rb.velocity.y, movement.z * moveSpeed);
        }
    }
}