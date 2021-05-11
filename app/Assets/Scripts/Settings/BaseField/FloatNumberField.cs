using System;
using UnityEngine;
using UnityEngine.UI;

namespace Reconstruction4D.Settings.BaseField
{
    abstract class FloatNumberField : SettingField
    {
        public InputField inputText;

        public float minValue;

        public float maxValue;

        protected float inputInserted;

        public override string OnSave(Settings settings)
        {
            if (this.inputText.text == String.Empty)
            {
                return "";
            }
            if (!float.TryParse(inputText.text, out inputInserted))
            {
                return "Inserted number isn't valid";
            }
            if (inputInserted < minValue)
            {
                return "Inserted number is below the min value";
            }
            if (inputInserted > maxValue)
            {
                return "Inserted number is above the max value";
            }
            return "";
        }
    }
}
