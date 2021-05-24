using System;
using UnityEngine;
using UnityEngine.UI;

namespace Reconstruction4D.Settings.BaseField
{
    abstract class IntegerNumberField : SettingField
    {
        public InputField inputText;

        public int minValue;

        public int maxValue;

        protected int inputInserted;

        public override string OnSave(Settings settings)
        {
            if (this.inputText.text == String.Empty)
            {
                return "";
            }
            if (!int.TryParse(inputText.text, out inputInserted))
            {
                return "Inserted number isn't valid";
            }
            if (inputInserted < minValue)
            {
                Debug.Log("Min value is: " + minValue + " inputInserted: " + inputInserted);
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
