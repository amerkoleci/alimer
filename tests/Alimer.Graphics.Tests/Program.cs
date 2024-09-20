// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

class Program
{
    public static int Main(string[] args)
    {
        List<string> newArgs = new(args);
        newArgs.Insert(0, AppContext.BaseDirectory);
        int returnCode = Xunit.ConsoleClient.Program.Main(newArgs.ToArray());
        Console.WriteLine("Tests finished. Press any key to exit.");
        if (!Console.IsInputRedirected)
        {
            Console.ReadKey(true);
        }
        return returnCode;
    }
}
