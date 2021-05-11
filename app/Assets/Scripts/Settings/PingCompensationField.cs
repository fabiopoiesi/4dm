using System;
using UnityEngine.UI;
using Reconstruction4D.Settings.BaseField;
using UnityEngine;

namespace Reconstruction4D.Settings
{
    class PingCompensationField : BoolField
    {
        public override string OnSave(Settings settings)
        {
            string result = base.OnSave(settings);
            if(result != String.Empty)
            {
                return result;
            }
            settings.PingCompesationActive = this.inputToggle.isOn;
            Debug.Log("Settings ping compensation: " + settings.PingCompesationActive);
            return "";
        }

        public override void OnOpen(Settings settings)
        {
            this.inputToggle.isOn = settings.PingCompesationActive;
        }
    }
}
