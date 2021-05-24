using System;
using UnityEngine.UI;
using Reconstruction4D.Settings.BaseField;

namespace Reconstruction4D.Settings
{
    class RelayField : HostnameField
    {
        public override string OnSave(Settings settings)
        {
            string result = base.OnSave(settings);
            if(result != String.Empty)
            {
                return result;
            }
            settings.RelayAddress = this.inputText.text;
            return "";
        }

        public override void OnOpen(Settings settings)
        {
            this.inputText.text = settings.RelayAddress;
        }
    }
}
