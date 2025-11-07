using GlmSharp;
using System;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using VulkanGameEngineLevelEditor.GameEngine.Structs;
using VulkanGameEngineLevelEditor.GameEngine.Systems;
using VulkanGameEngineLevelEditor.LevelEditor.Attributes;

public class SpriteModel : INotifyPropertyChanged
{
    private Sprite sprite = new Sprite();

    public event PropertyChangedEventHandler PropertyChanged;

    [Category("Sprite")]
    [Tooltip("The unique ID of the game object this sprite belongs to.")]
    public uint GameObjectId
    {
        get => sprite.GameObjectId;
        private set
        {
            if (sprite.GameObjectId != value)
            {
                sprite.GameObjectId = value;
                OnPropertyChanged();
            }
        }
    }

    [Category("Sprite")]
    [Tooltip("The unique ID of the sprite asset.")]
    public uint SpriteID
    {
        get => sprite.SpriteID;
        set
        {
            if (sprite.SpriteID != value)
            {
                sprite.SpriteID = value;
                OnPropertyChanged();
            }
        }
    }

    [Category("Sprite")]
    [Tooltip("The current animation ID being played.")]
    public uint CurrentAnimationID
    {
        get => sprite.CurrentAnimationID;
        set
        {
            if (sprite.CurrentAnimationID != value)
            {
                sprite.CurrentAnimationID = value;
                OnPropertyChanged();
            }
        }
    }

    [Category("Sprite")]
    [Tooltip("The current frame of the animation.")]
    public uint CurrentFrame
    {
        get => sprite.CurrentFrame;
        set
        {
            if (sprite.CurrentFrame != value)
            {
                sprite.CurrentFrame = value;
                OnPropertyChanged();
            }
        }
    }

    [Category("Sprite")]
    [Tooltip("The VRAM ID for the sprite texture.")]
    public Guid SpriteVramId
    {
        get => sprite.SpriteVramId;
        set
        {
            if (sprite.SpriteVramId != value)
            {
                sprite.SpriteVramId = value;
                OnPropertyChanged();
            }
        }
    }

    [Category("Sprite")]
    [Tooltip("The time elapsed on the current frame (hidden from UI).")]
    [Browsable(false)]
    public float CurrentFrameTime
    {
        get => sprite.CurrentFrameTime;
        set
        {
            if (sprite.CurrentFrameTime != value)
            {
                sprite.CurrentFrameTime = value;
                OnPropertyChanged();
            }
        }
    }

    [Category("Sprite")]
    [Tooltip("Whether the sprite is active.")]
    public bool SpriteAlive
    {
        get => sprite.SpriteAlive;
        set
        {
            if (sprite.SpriteAlive != value)
            {
                sprite.SpriteAlive = value;
                OnPropertyChanged();
            }
        }
    }

    [Category("Sprite")]
    [Tooltip("Flip direction of the sprite (0 or 1).")]
    public ivec2 FlipSprite
    {
        get => sprite.FlipSprite;
        set
        {
            if (sprite.FlipSprite != value)
            {
                sprite.FlipSprite = value;
                OnPropertyChanged();
            }
        }
    }

    [Category("Sprite")]
    [Tooltip("The last recorded position (hidden from UI).")]
    [Browsable(false)]
    public vec2 LastSpritePosition
    {
        get => sprite.LastSpritePosition;
        set
        {
            if (sprite.LastSpritePosition != value)
            {
                sprite.LastSpritePosition = value;
                OnPropertyChanged();
            }
        }
    }

    [Category("Sprite")]
    [Tooltip("The last recorded rotation (hidden from UI).")]
    [Browsable(false)]
    public vec2 LastSpriteRotation
    {
        get => sprite.LastSpriteRotation;
        set
        {
            if (sprite.LastSpriteRotation != value)
            {
                sprite.LastSpriteRotation = value;
                OnPropertyChanged();
            }
        }
    }

    [Category("Sprite")]
    [Tooltip("The last recorded scale (hidden from UI).")]
    [Browsable(false)]
    public vec2 LastSpriteScale
    {
        get => sprite.LastSpriteScale;
        set
        {
            if (sprite.LastSpriteScale != value)
            {
                sprite.LastSpriteScale = value;
                OnPropertyChanged();
            }
        }
    }

    [Category("Sprite")]
    [Tooltip("The current position of the sprite.")]
    public vec2 SpritePosition
    {
        get => sprite.SpritePosition;
        set
        {
            if (sprite.SpritePosition != value)
            {
                sprite.SpritePosition = value;
                OnPropertyChanged();
            }
        }
    }

    [Category("Sprite")]
    [Tooltip("The current rotation of the sprite in degrees.")]
    [Range(-360f, 360f)]
    public vec2 SpriteRotation
    {
        get => sprite.SpriteRotation;
        set
        {
            if (sprite.SpriteRotation != value)
            {
                sprite.SpriteRotation = value;
                OnPropertyChanged();
            }
        }
    }

    [Category("Sprite")]
    [Tooltip("The current scale of the sprite.")]
    [Range(0.1f, 10f)]
    public vec2 SpriteScale
    {
        get => sprite.SpriteScale;
        set
        {
            if (sprite.SpriteScale != value)
            {
                sprite.SpriteScale = value;
                OnPropertyChanged();
            }
        }
    }

    public SpriteModel()
    {
    }

    protected void OnPropertyChanged([CallerMemberName] string propertyName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
    }
}
