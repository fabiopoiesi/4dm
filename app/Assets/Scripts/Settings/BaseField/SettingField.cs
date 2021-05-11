using UnityEngine;
using Reconstruction4D.Settings;

namespace Reconstruction4D.Settings.BaseField
{
    [System.Serializable]
    abstract class SettingField : MonoBehaviour
    {
        public abstract string OnSave(Settings settings);
        public abstract void OnOpen(Settings settings);
    }
}
