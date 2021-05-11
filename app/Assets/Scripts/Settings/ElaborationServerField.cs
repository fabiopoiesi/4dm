using Reconstruction4D.Settings.BaseField;
using System;
using UnityEngine.UI;

namespace Reconstruction4D.Settings
{
    class ElaborationServerField : HostnameField
    {
        public override string OnSave(Settings settings)
        {
            string result = base.OnSave(settings);
            if (result != String.Empty)
            {
                return result;
            }
            settings.ElaborationServerAddress = this.inputText.text;
            return "";
        }

        public override void OnOpen(Settings settings)
        {
            this.inputText.text = settings.ElaborationServerAddress;
        }
    }
}
