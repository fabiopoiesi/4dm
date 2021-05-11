namespace Reconstruction4D.Settings
{
    using System.Collections.Generic;
    using UnityEngine;
    using UnityEngine.UI;
    using Reconstruction4D.Settings.BaseField;

    class SettingsManager : MonoBehaviour
    {
        public Text SnackbarText;

        public GameObject SettingsPanel;

        public GameObject SettingsButton;

        public List<SettingField> Fields = new List<SettingField>();

        private string SnackbarTextBeforeDialog = "";

        public void Start()
        {
            // SettingsPanel.SetActive(false);
        }

        public void OnSettingsButtonClick()
        {
            SettingsButton.SetActive(false);
            SettingsPanel.SetActive(true);
            foreach(SettingField field in Fields)
            {
                field.OnOpen(Settings.Instance);
            }
            this.SnackbarTextBeforeDialog = SnackbarText.text;
        }

        public void OnDoneButtonClick()
        {
            foreach (SettingField field in Fields)
            {
                string result = field.OnSave(Settings.Instance);
                if (result != string.Empty)
                {
                    SnackbarText.text = result;
                    return;
                }
            }
            Settings.Instance.SaveConfig();
            SnackbarText.text = SnackbarTextBeforeDialog;
            SettingsButton.SetActive(true);
            SettingsPanel.SetActive(false);
        }
    }
}
