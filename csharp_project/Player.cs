using Godot;
using System;
using System.Collections.Generic;

public partial class Player : CharacterBody3D
{
    [Export] Node3D Pivot { get; set; } = null!;
    [Export] float Sensitivity { get; set; } = 0.5f;
    [Export] float Speed { get; set; } = 5.0f;
    [Export] float JumpVelocity { get; set; } = 4.5f;

    public override void _Ready()
    {
        Input.MouseMode = Input.MouseModeEnum.Captured;
    }

    public override void _PhysicsProcess(double delta)
    {
        if (Input.IsActionJustPressed("pause"))
        {
            if (Input.MouseMode == Input.MouseModeEnum.Captured)
            {
                Input.MouseMode = Input.MouseModeEnum.Visible;
                GetTree().Paused = true;
            }
            else
            {
                Input.MouseMode = Input.MouseModeEnum.Captured;
                GetTree().Paused = false;
            }
        }

        if (GetTree().Paused) { return; }

        ProcessMouse();

        Vector3 velocity = Velocity;

        // Add the gravity.
        if (!IsOnFloor())
        {
            velocity += GetGravity() * (float)delta;
        }

        // Handle Jump.
        if (Input.IsActionJustPressed("jump") && IsOnFloor())
        {
            velocity.Y = JumpVelocity;
        }

        // Get the input direction and handle the movement/deceleration.
        // As good practice, you should replace UI actions with custom gameplay actions.
        Vector2 inputDir = Input.GetVector("left", "right", "forward", "backward");
        Vector3 direction = (Transform.Basis * new Vector3(inputDir.X, 0, inputDir.Y)).Normalized();
        if (direction != Vector3.Zero)
        {
            velocity.X = direction.X * Speed;
            velocity.Z = direction.Z * Speed;
        }
        else
        {
            velocity.X = Mathf.MoveToward(Velocity.X, 0, Speed);
            velocity.Z = Mathf.MoveToward(Velocity.Z, 0, Speed);
        }

        Velocity = velocity;
        MoveAndSlide();
    }

    List<Vector2> mouseBuffer = [];

    private void ProcessMouse()
    {
        Vector2 mouseMotion = Vector2.Zero;
        foreach (var motion in mouseBuffer)
        {
            mouseMotion += motion;
        }
        mouseBuffer.Clear();

        RotateY(Mathf.DegToRad(-mouseMotion.X * Sensitivity));
        Pivot.RotateX(Mathf.DegToRad(-mouseMotion.Y * Sensitivity));
        Pivot.Rotation = Pivot.Rotation with { X = Mathf.Clamp(Pivot.Rotation.X, -Mathf.Pi / 2.0f, Mathf.Pi / 2.0f) };
    }

    public override void _UnhandledInput(InputEvent @event)
    {
        if (GetTree().Paused) { return; }

        if (@event is InputEventMouseMotion mouseMotion && !mouseMotion.ScreenRelative.IsZeroApprox())
        {
            mouseBuffer.Add(mouseMotion.ScreenRelative);
        }
    }
}
