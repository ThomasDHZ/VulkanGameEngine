namespace VulkanGameEngineLevelEditor.GameEngine.Structs
{
    public struct Pixel3
    {
        public byte Red { get; set; }
        public byte Green { get; set; }
        public byte Blue { get; set; }


        public Pixel3(byte red, byte green, byte blue)
        {
            Red = red;
            Green = green;
            Blue = blue;
        }

        public static bool operator ==(Pixel3 lhs, Pixel3 rhs)
        {
            return lhs.Red == rhs.Red &&
                   lhs.Green == rhs.Green &&
                   lhs.Blue == rhs.Blue;
        }

        public static bool operator !=(Pixel3 lhs, Pixel3 rhs)
        {
            return !(lhs == rhs);
        }

        public override bool Equals(object obj)
        {
            if (obj is Pixel3 pixel)
            {
                return this == pixel;
            }
            return false;
        }

        public override int GetHashCode()
        {
            return (Red, Green, Blue).GetHashCode();
        }
    }

    public struct Pixel
    {
        public byte Red { get; set; }
        public byte Green { get; set; }
        public byte Blue { get; set; }
        public byte Alpha { get; set; }

        public Pixel() : this(0x00, 0x00, 0x00, 0xFF) { }
        public Pixel(byte value) : this(value, value, value, 0xFF) { }
        public Pixel(byte redGreen, byte blue) : this(redGreen, redGreen, blue, 0xFF) { }
        public Pixel(byte red, byte green, byte blue) : this(red, green, blue, 0xFF) { }
        public Pixel(byte red, byte green, byte blue, byte alpha)
        {
            Red = red;
            Green = green;
            Blue = blue;
            Alpha = alpha;
        }

        public static bool operator ==(Pixel lhs, Pixel rhs)
        {
            return lhs.Red == rhs.Red &&
                   lhs.Green == rhs.Green &&
                   lhs.Blue == rhs.Blue;
        }

        public static bool operator !=(Pixel lhs, Pixel rhs)
        {
            return !(lhs == rhs);
        }

        public override bool Equals(object obj)
        {
            if (obj is Pixel pixel)
            {
                return this == pixel;
            }
            return false;
        }

        public override int GetHashCode()
        {
            return (Red, Green, Blue).GetHashCode();
        }
    }

    public struct Pixel32
    {
        public float Red { get; set; }
        public float Green { get; set; }
        public float Blue { get; set; }
        public float Alpha { get; set; }

        public Pixel32() : this(0f, 0f, 0f, 1f) { }
        public Pixel32(float value) : this(value, value, value, 1f) { }
        public Pixel32(float redGreen, float blue) : this(redGreen, redGreen, blue, 1f) { }
        public Pixel32(float red, float green, float blue) : this(red, green, blue, 1f) { }
        public Pixel32(float red, float green, float blue, float alpha)
        {
            Red = red;
            Green = green;
            Blue = blue;
            Alpha = alpha;
        }

        public static bool operator ==(Pixel32 lhs, Pixel32 rhs)
        {
            return lhs.Red == rhs.Red &&
                   lhs.Green == rhs.Green &&
                   lhs.Blue == rhs.Blue;
        }

        public static bool operator !=(Pixel32 lhs, Pixel32 rhs)
        {
            return !(lhs == rhs);
        }

        public override bool Equals(object obj)
        {
            if (obj is Pixel32 pixel32)
            {
                return this == pixel32;
            }
            return false;
        }

        public override int GetHashCode()
        {
            return (Red, Green, Blue).GetHashCode();
        }
    }

    public static class PixelConstants
    {
        public static readonly Pixel NullPixel = new Pixel(0x00, 0x00, 0x00, 0xFF);
        public static readonly Pixel ClearPixel = new Pixel(0x00, 0x00, 0x00, 0x00);
    }
}
