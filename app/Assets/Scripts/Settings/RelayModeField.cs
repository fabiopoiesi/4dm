using System;
using UnityEngine.UI;
using Reconstruction4D.Settings.BaseField;

namespace Reconstruction4D.Settings
{
    class RelayModeField : SettingField
    {
        public Dropdown selectionDropdown;

        public override void OnOpen(Settings settings)
        {
            selectionDropdown.value = (int)(settings.Type);
        }

        public override string OnSave(Settings settings)
        {
            settings.Type = (Settings.ConnectionType)selectionDropdown.value;
            return "";
        }
    }
}
