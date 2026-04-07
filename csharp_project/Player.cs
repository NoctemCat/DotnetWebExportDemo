using Godot;
using System;
using System.Collections.Generic;
using System.Threading;

#if GODOT_WEB
using System.Runtime.InteropServices.JavaScript;
#endif
using System.Threading.Tasks;

namespace Sample;

public partial class Player : CharacterBody3D
{
    [Export] Node3D Pivot { get; set; } = null!;
    [Export] float Sensitivity { get; set; } = 0.5f;
    [Export] float Speed { get; set; } = 5.0f;
    [Export] float JumpVelocity { get; set; } = 4.5f;

    public Player()
    {
        GD.Print("Player constructor");
        Variant variant = new Node3D();
        if (variant.AsGodotObject() is not null)
        {
            GD.Print("Object not null");
        }
        if (variant.As<GodotObject>() is Node3D node)
        {
            node.QueueFree();
        }
        GD.Print("Player constructor after node");
        using Variant variantFloat = 20.0f;
        if (variantFloat.As<float>() is float floatValue)
        {
            GD.Print($"Read {floatValue}");
        }
        GD.Print("Player constructor exit");
    }

    public override void _Ready()
    {
        GD.Print("Player _Ready");
        // Input.UseAccumulatedInput = false;
        Input.MouseMode = Input.MouseModeEnum.Captured;

        GD.Seed(123);
        GD.Print($"Is main thread {GodotThread.IsMainThread()}");
        GD.PrintS("Main Thread", Thread.CurrentThread.ManagedThreadId);
        TestThread();

        RunCrypro();
#if !GODOT_THREADS_ENABLED
        // Causes too much deadlocks at random with multithreading
        RunHTTP();
#endif
#if GODOT_WEB
        _ = TestAdvancedAsync();
#endif
    }

    public override void _PhysicsProcess(double delta)
    {
        if (GetTree().Paused) { return; }

        ProcessMouse();

        Vector3 velocity = Velocity;

        if (!IsOnFloor())
        {
            velocity += GetGravity() * (float)delta;
        }

        if (Input.IsActionJustPressed("jump") && IsOnFloor())
        {
            velocity.Y = JumpVelocity;
        }

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
        if (@event.IsActionPressed("pause"))
        {
            if (GetTree().Paused)
            {
                Input.MouseMode = Input.MouseModeEnum.Captured;
                GetTree().Paused = false;
            }
            else
            {
                Input.MouseMode = Input.MouseModeEnum.Visible;
                GetTree().Paused = true;
            }
        }

        if (GetTree().Paused) { return; }

        if (@event is InputEventMouseMotion mouseMotion && !mouseMotion.ScreenRelative.IsZeroApprox())
        {
            mouseBuffer.Add(mouseMotion.ScreenRelative);
        }
    }

    private void RunCrypro()
    {
        var bytes = new byte[16];
        System.Security.Cryptography.RandomNumberGenerator.Fill(bytes);
        GD.Print("Crypto result: " + BitConverter.ToString(bytes));
    }

    private async void RunHTTP()
    {
        CancellationTokenSource source = new();
        CancellationToken token = source.Token;
        int timeout = 2000;
        try
        {
            source.CancelAfter(timeout);
            using var client = new System.Net.Http.HttpClient();
            var response = await client.GetStringAsync("https://httpbin.org/get", token);
            GD.Print("HTTP result: " + response[..100]);
        }
        catch (OperationCanceledException)
        {
            GD.Print($"Tasks cancelled: timed out after {timeout}ms.");
        }
        catch (Exception e)
        {
            GD.Print("HTTP error: " + e.GetType().Name + ": " + e.Message);
        }
        finally
        {
            source.Dispose();
        }

    }

#if GODOT_WEB
    private async Task TestAdvancedAsync()
    {
        GD.Print("Hello, World!");

        var rand = new Random();
        GD.Print("Today's lucky number is " + rand.Next(100) + " and " + Guid.NewGuid());

        var start = DateTime.UtcNow;
        var timezonesCount = TimeZoneInfo.GetSystemTimeZones().Count;
        await JsDelay(100);
        var end = DateTime.UtcNow;
        GD.Print($"Found {timezonesCount} timezones in the TZ database in {end - start}");

        TimeZoneInfo utc = TimeZoneInfo.FindSystemTimeZoneById("UTC");
        GD.Print($"{utc.DisplayName} BaseUtcOffset is {utc.BaseUtcOffset}");

        try
        {
            TimeZoneInfo tst = TimeZoneInfo.FindSystemTimeZoneById("Asia/Tokyo");
            GD.Print($"{tst.DisplayName} BaseUtcOffset is {tst.BaseUtcOffset}");
        }
        catch (TimeZoneNotFoundException tznfe)
        {
            GD.Print($"Could not find Asia/Tokyo: {tznfe.Message}");
        }
    }



    [JSImport("Sample.Test.add", "main.js")]
    internal static partial int Add(int a, int b);

    [JSImport("Sample.Test.delay", "main.js")]
    [return: JSMarshalAs<JSType.Promise<JSType.Void>>]
    internal static partial Task JsDelay([JSMarshalAs<JSType.Number>] int ms);

    [JSExport]
    internal static async Task PrintMeaning(Task<int> meaningPromise)
    {
        Console.WriteLine("Meaning of life is " + await meaningPromise);
    }

    [JSExport]
    internal static int TestMeaning()
    {
        var half = 21;
        // call back to JS via [JSImport]
        return Add(half, half);
    }

    [JSExport]
    internal static void SillyLoop()
    {
        Console.WriteLine("UtcNow is " + DateTime.UtcNow.Millisecond);
        // this silly method will generate few sample points for the profiler
        bool breakCond = false;
        for (int i = 1; i <= 500; i++)
        {
            try
            {
                for (int s = 0; s <= 500; s++)
                {
                    try
                    {
                        if (DateTime.UtcNow.Millisecond == i + s)
                        {
                            Console.WriteLine("Time is " + s);
                            breakCond = true;
                            break;
                        }
                    }
                    catch (Exception e)
                    {
                        Console.WriteLine(e.Message);
                    }
                }
                if (breakCond)
                {
                    break;
                }
            }
            catch (Exception e)
            {
                Console.WriteLine(e.Message);
            }
        }
    }

    [JSExport]
    internal static bool IsPrime(int number)
    {
        if (number <= 1) return false;
        if (number == 2) return true;
        if (number % 2 == 0) return false;

        var boundary = (int)Math.Floor(Math.Sqrt(number));

        for (int i = 3; i <= boundary; i += 2)
            if (number % i == 0)
                return false;

        return true;
    }
#endif

    internal void TestThread()
    {
        Task.Run(TestThreadImpl);
    }
    internal async void TestThreadImpl()
    {
        await Task.Delay(2000);
        GD.PrintS("Hello Thread", Thread.CurrentThread.ManagedThreadId);
    }
}
