using System;
using System.Drawing;
using System.Windows.Forms;
using VulkanGameEngineLevelEditor.GameEngine;

namespace VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements
{
    public unsafe class LightPropertiesPanel : GroupBox
    {
        private readonly uint _gameObjectId;
        private readonly uint _componentType; // or whatever your type identifier is
        private readonly IntPtr _componentPtr;
        private readonly Type _structType;
        private readonly ToolTip _toolTip;

        // Example controls (adjust field offsets/names to match your LightComponent struct)
        private ComboBox _cmbType;
        private Button _btnColor;
        private TrackBar _trkIntensity;
        private NumericUpDown _numIntensity;
        private NumericUpDown _numRange;
        private NumericUpDown _numSpotAngle;
        private CheckBox _chkEnabled;
        private CheckBox _chkCastShadows;

        public LightPropertiesPanel(uint gameObjectId, uint componentType, IntPtr componentPtr, Type structType, ToolTip toolTip)
        {
            _gameObjectId = gameObjectId;
            _componentType = componentType;
            _componentPtr = componentPtr;
            _structType = structType;
            _toolTip = toolTip;

            Text = "Light Component";
            BackColor = Color.FromArgb(45, 45, 48);
            ForeColor = Color.White;
            Padding = new Padding(10);
            Height = 320; // Adjust as needed

            InitializeLightUI();
            LoadCurrentValues();
        }

        private void InitializeLightUI()
        {
            int y = 10;

            // Type
            Controls.Add(new Label { Text = "Type:", ForeColor = Color.Silver, Location = new Point(10, y), AutoSize = true });
            _cmbType = new ComboBox
            {
                Location = new Point(120, y - 3),
                Width = 180,
                DropDownStyle = ComboBoxStyle.DropDownList
            };
            _cmbType.Items.AddRange(new[] { "Directional", "Point", "Spot", "Area" });
            _cmbType.SelectedIndexChanged += (s, e) => UpdateLightType();
            Controls.Add(_cmbType);
            y += 35;

            // Color
            Controls.Add(new Label { Text = "Color:", ForeColor = Color.Silver, Location = new Point(10, y), AutoSize = true });
            _btnColor = new Button
            {
                Location = new Point(120, y - 4),
                Width = 60,
                Height = 24,
                FlatStyle = FlatStyle.Flat,
                BackColor = Color.White
            };
            _btnColor.Click += (s, e) => PickColor();
            Controls.Add(_btnColor);
            y += 35;

            // Intensity
            Controls.Add(new Label { Text = "Intensity:", ForeColor = Color.Silver, Location = new Point(10, y), AutoSize = true });
            _trkIntensity = new TrackBar
            {
                Location = new Point(120, y - 2),
                Width = 150,
                Minimum = 0,
                Maximum = 1000,
                TickFrequency = 50
            };
            _trkIntensity.ValueChanged += (s, e) => SyncIntensity();
            Controls.Add(_trkIntensity);

            _numIntensity = new NumericUpDown
            {
                Location = new Point(280, y),
                Width = 80,
                DecimalPlaces = 2,
                Minimum = 0,
                Maximum = 10000,
                Increment = 0.1m
            };
            _numIntensity.ValueChanged += (s, e) => SyncIntensityFromNum();
            Controls.Add(_numIntensity);
            y += 40;

            // Range (Point/Spot)
            Controls.Add(new Label { Text = "Range:", ForeColor = Color.Silver, Location = new Point(10, y), AutoSize = true });
            _numRange = new NumericUpDown
            {
                Location = new Point(120, y - 3),
                Width = 100,
                DecimalPlaces = 1,
                Minimum = 0.1m,
                Maximum = 1000,
                Increment = 0.5m
            };
            _numRange.ValueChanged += (s, e) => UpdateLightProperty("Range", (float)_numRange.Value);
            Controls.Add(_numRange);
            y += 35;

            // Spot Angle (Spot only)
            Controls.Add(new Label { Text = "Spot Angle:", ForeColor = Color.Silver, Location = new Point(10, y), AutoSize = true });
            _numSpotAngle = new NumericUpDown
            {
                Location = new Point(120, y - 3),
                Width = 100,
                Minimum = 1,
                Maximum = 179,
                Increment = 1
            };
            _numSpotAngle.ValueChanged += (s, e) => UpdateLightProperty("SpotAngle", (float)_numSpotAngle.Value);
            Controls.Add(_numSpotAngle);
            y += 35;

            // Enabled / Cast Shadows
            _chkEnabled = new CheckBox { Text = "Enabled", Location = new Point(10, y), ForeColor = Color.White, Checked = true };
            _chkEnabled.CheckedChanged += (s, e) => UpdateLightProperty("Enabled", _chkEnabled.Checked);
            Controls.Add(_chkEnabled);

            _chkCastShadows = new CheckBox { Text = "Cast Shadows", Location = new Point(150, y), ForeColor = Color.White };
            _chkCastShadows.CheckedChanged += (s, e) => UpdateLightProperty("CastShadows", _chkCastShadows.Checked);
            Controls.Add(_chkCastShadows);
        }

        private void LoadCurrentValues()
        {
            // TODO: Read from unsafe pointer using your struct layout
            // Example (adjust offsets / use fixed or Marshal):
            // var light = (LightComponent*)_componentPtr.ToPointer();
            // _cmbType.SelectedItem = light->Type.ToString();
            // _btnColor.BackColor = Color.FromArgb((int)(light->Color.X * 255), ...);
            // _numIntensity.Value = (decimal)light->Intensity;
            // etc.
        }

        private void PickColor()
        {
            using var dlg = new ColorDialog { Color = _btnColor.BackColor };
            if (dlg.ShowDialog() == DialogResult.OK)
            {
                _btnColor.BackColor = dlg.Color;
                // Update component: e.g. light->Color = new Vector4(dlg.Color.R/255f, ...);
                MarkDirty();
            }
        }

        private void SyncIntensity()
        {
            _numIntensity.Value = _trkIntensity.Value;
            UpdateLightProperty("Intensity", _trkIntensity.Value);
        }

        private void SyncIntensityFromNum()
        {
            _trkIntensity.Value = (int)Math.Clamp(_numIntensity.Value, _trkIntensity.Minimum, _trkIntensity.Maximum);
            UpdateLightProperty("Intensity", (float)_numIntensity.Value);
        }

        private void UpdateLightType()
        {
            // Hide/show Range/SpotAngle based on type
            bool isSpot = _cmbType.SelectedItem?.ToString() == "Spot";
            bool isPointOrSpot = _cmbType.SelectedItem?.ToString() is "Point" or "Spot";

            _numRange.Enabled = isPointOrSpot;
            _numSpotAngle.Enabled = isSpot;

            UpdateLightProperty("Type", _cmbType.SelectedIndex); // or enum value
            MarkDirty();
        }

        private void UpdateLightProperty(string fieldName, object value)
        {
            // TODO: Write back to the unsafe struct via pointer
            // Example: 
            // var light = (LightComponent*)_componentPtr;
            // switch (fieldName) { case "Intensity": light->Intensity = (float)value; ... }

            // Notify engine to update GPU data / rebuild light list
            MarkDirty();
        }

        private void MarkDirty()
        {
            // Optional: call some engine refresh / light system update
            // GameObjectSystem.MarkComponentDirty(_gameObjectId, _componentType);
        }
    }
}