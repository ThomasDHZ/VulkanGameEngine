﻿using RGiesecke.DllExport;
using System;
using System.Runtime.InteropServices;

namespace MyExportedFunctions
{
    public class ExportedMethods
    {
        [DllExport("Add", CallingConvention = CallingConvention.StdCall)]
        public static int Add(int a, int b)
        {
            return a + b;
        }

        [DllExport("Multiply", CallingConvention = CallingConvention.StdCall)]
        public static int Multiply(int a, int b)
        {
            return a * b;
        }
    }
}
