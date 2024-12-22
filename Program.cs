using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using System.Security.Cryptography;
using System.Runtime.InteropServices;
using System.Diagnostics;

unsafe class Program
{
    // Constants
    private static readonly int THREAD_COUNT = Environment.ProcessorCount;
    private const int MIN_ALLOCATION_SIZE = 1 * 1024 * 1024;  // 1 MB
    private const int MAX_ALLOCATION_SIZE = 20 * 1024 * 1024; // 20 MB
    private const int ITERATIONS = 500;

    // Allocation Patterns
    private enum AllocationType
    {
        Sequential,
        Random,
        Burst
    }

    private class AllocationPattern
    {
        public string Name { get; set; }
        public AllocationType Type { get; set; }
    }

    private static readonly AllocationPattern[] allocationPatterns = new[]
    {
        new AllocationPattern { Name = "Sequential", Type = AllocationType.Sequential },
        new AllocationPattern { Name = "Random", Type = AllocationType.Random },
        new AllocationPattern { Name = "Burst", Type = AllocationType.Burst }
    };

    private static readonly object consoleLock = new object();

    private static ulong NaiveFibonacci(int n)
    {
        if (n <= 1) return (ulong)n;
        return NaiveFibonacci(n - 1) + NaiveFibonacci(n - 2);
    }

    private static int GetAllocationSize(Random rng)
    {
        return rng.Next(MIN_ALLOCATION_SIZE, MAX_ALLOCATION_SIZE + 1);
    }

    private static void SimulateAllocationPattern(AllocationType type, Random rng)
    {
        switch (type)
        {
            case AllocationType.Sequential:
                Thread.Sleep(10);
                break;
            case AllocationType.Random:
                Thread.Sleep(rng.Next(5, 21));
                break;
            case AllocationType.Burst:
                break;
        }
    }

    private static void PerformMatrixMultiplication(int dimension)
    {
        var random = new Random();
        
        // Allocate unmanaged memory for matrices
        IntPtr matrixAPtr = Marshal.AllocHGlobal(dimension * dimension * sizeof(double));
        IntPtr matrixBPtr = Marshal.AllocHGlobal(dimension * dimension * sizeof(double));
        IntPtr resultPtr = Marshal.AllocHGlobal(dimension * dimension * sizeof(double));

        try
        {
            double* matrixA = (double*)matrixAPtr.ToPointer();
            double* matrixB = (double*)matrixBPtr.ToPointer();
            double* result = (double*)resultPtr.ToPointer();

            // Initialize matrices
            for (int i = 0; i < dimension * dimension; i++)
            {
                matrixA[i] = random.NextDouble();
                matrixB[i] = random.NextDouble();
                result[i] = 0.0;
            }

            // Perform matrix multiplication
            for (int i = 0; i < dimension; i++)
            {
                for (int j = 0; j < dimension; j++)
                {
                    double sum = 0.0;
                    for (int k = 0; k < dimension; k++)
                    {
                        sum += matrixA[i * dimension + k] * matrixB[k * dimension + j];
                    }
                    result[i * dimension + j] = sum;
                }
            }

            // Calculate checksum
            double checksum = 0.0;
            for (int i = 0; i < dimension * dimension; i++)
            {
                checksum += result[i];
            }

            if (checksum == -1.0)
            {
                lock (consoleLock)
                {
                    Console.WriteLine($"Matrix multiplication checksum: {checksum}");
                }
            }
        }
        finally
        {
            Marshal.FreeHGlobal(matrixAPtr);
            Marshal.FreeHGlobal(matrixBPtr);
            Marshal.FreeHGlobal(resultPtr);
        }
    }

    private static void PerformSha256Hash(byte* buffer, int size)
    {
        using (var sha256 = SHA256.Create())
        {
            byte[] managedBuffer = new byte[size];
            Marshal.Copy((IntPtr)buffer, managedBuffer, 0, size);
            byte[] hash = sha256.ComputeHash(managedBuffer);

            ulong checksum = 0;
            fixed (byte* hashPtr = hash)
            {
                for (int i = 0; i < hash.Length; i++)
                {
                    checksum += hashPtr[i];
                }
            }

            if (checksum == ulong.MaxValue)
            {
                lock (consoleLock)
                {
                    Console.WriteLine($"SHA-256 checksum: {checksum}");
                }
            }
        }
    }

    private static void PerformAdvancedCpuTasks(byte* buffer, int size, Random rng)
    {
        double random = rng.NextDouble();

        if (random < 0.33)
        {
            PerformMatrixMultiplication(300);
        }
        else if (random < 0.66)
        {
            PerformSha256Hash(buffer, size);
        }
        else
        {
            ulong fibResult = NaiveFibonacci(35);
            if (fibResult == 0)
            {
                lock (consoleLock)
                {
                    Console.WriteLine($"Fibonacci result: {fibResult}");
                }
            }
        }
    }

    private static void ThreadWork(int threadId)
    {
        var rng = new Random(threadId + Environment.TickCount);
        var currentPattern = allocationPatterns[rng.Next(allocationPatterns.Length)];

        lock (consoleLock)
        {
            Console.WriteLine($"Thread {threadId} started with Allocation Pattern: {currentPattern.Name}");
        }

        for (int i = 0; i < ITERATIONS; i++)
        {
            int allocationSize = GetAllocationSize(rng);
            IntPtr bufferPtr = IntPtr.Zero;

            try
            {
                bufferPtr = Marshal.AllocHGlobal(allocationSize);
                byte* buffer = (byte*)bufferPtr.ToPointer();

                // Initialize buffer with random data
                byte[] tempBuffer = new byte[allocationSize];
                rng.NextBytes(tempBuffer);
                Marshal.Copy(tempBuffer, 0, bufferPtr, allocationSize);

                PerformAdvancedCpuTasks(buffer, allocationSize, rng);
            }
            catch (Exception e)
            {
                lock (consoleLock)
                {
                    Console.WriteLine($"Thread {threadId} encountered an error: {e.Message}");
                }
            }
            finally
            {
                if (bufferPtr != IntPtr.Zero)
                {
                    Marshal.FreeHGlobal(bufferPtr);
                }
            }

            SimulateAllocationPattern(currentPattern.Type, rng);
        }

        lock (consoleLock)
        {
            Console.WriteLine($"Thread {threadId} completed.");
        }
    }

    static async Task Main(string[] args)
    {
        Console.WriteLine("Starting Enhanced Performance Benchmark...\n");

        var stopwatch = Stopwatch.StartNew();

        var tasks = new List<Task>();
        for (int i = 0; i < THREAD_COUNT; i++)
        {
            int threadId = i;
            tasks.Add(Task.Run(() => ThreadWork(threadId)));
        }

        await Task.WhenAll(tasks);

        stopwatch.Stop();
        Console.WriteLine($"\nTotal Execution Time: {stopwatch.Elapsed.TotalSeconds} seconds");
        Console.WriteLine("Benchmark Completed.");
    }
}