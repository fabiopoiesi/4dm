using System;
using Reconstruction4D.Settings.BaseField;

namespace Reconstruction4D.Settings
{
    class StartDelayField : IntegerNumberField
    {
        public override string OnSave(Settings settings)
        {
            string result = base.OnSave(settings);
            if(result != String.Empty)
            {
                return result;
            }
            settings.StartDelay = this.inputInserted;
            return "";
        }

        public override void OnOpen(Settings settings)
        {
            this.inputText.text = Convert.ToString(settings.StartDelay);
        }
    }
}
