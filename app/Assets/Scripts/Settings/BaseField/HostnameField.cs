using System;
using UnityEngine.UI;

namespace Reconstruction4D.Settings.BaseField
{
    abstract class HostnameField : SettingField
    {
        public InputField inputText;

        public override string OnSave(Settings settings)
        {
            if (this.inputText.text == String.Empty)
            {
                return "";
            }
            UriHostNameType host = Uri.CheckHostName(this.inputText.text);
            if (host == UriHostNameType.Unknown)
            {
                return "Inserted host isn't valid";
            }
            if (host == UriHostNameType.IPv6)
            {
                return "IPv6 address aren't supported";
            }
            return "";
        }
    }
}
