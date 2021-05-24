using System;
using UnityEngine.UI;

namespace Reconstruction4D.Settings.BaseField
{
    abstract class BoolField : SettingField
    {
        public Toggle inputToggle;

        public override string OnSave(Settings settings)
        {
            return "";
        }
    }
}
