// CSBNodeData.cs
// Attach this component to GameObjects to specify CSB-specific metadata
// that doesn't have a direct Unity equivalent.

using UnityEngine;

/// <summary>
/// Stores CSB-specific metadata for export.
/// Attach to any GameObject that needs custom CSB properties.
/// </summary>
public class CSBNodeData : MonoBehaviour
{
    [Header("User Data")]
    [Tooltip("Custom property string exported as WidgetOptions.customProperty (Cocos Studio 'User Data')")]
    [TextArea(2, 5)]
    public string customProperty = "";

    [Header("Project Node (Nested CSB)")]
    [Tooltip("If set, this node will be exported as ProjectNode referencing another .csb file")]
    public string projectNodePath = "";

    [Tooltip("File name without path (e.g., 'ButtonPrefab.csb')")]
    public string projectNodeFileName = "";

    [Header("Node Type Override")]
    [Tooltip("Force a specific CSB node type instead of auto-detection")]
    public CSBNodeType forceNodeType = CSBNodeType.Auto;

    [Header("Action Tag")]
    [Tooltip("Animation action tag for this node (used in NodeAction.tag)")]
    public int actionTag = -1;

    [Header("Touch/Interaction")]
    [Tooltip("Enable touch for this widget")]
    public bool touchEnabled = true;

    [Header("Callback")]
    [Tooltip("Callback name for button/interaction events (exported as callBackName)")]
    public string callbackName = "";

    [Tooltip("Callback type: 0=None, 1=Touch, 2=Click")]
    public int callbackType = 0;

    /// <summary>
    /// Returns true if this node should be exported as a ProjectNode
    /// </summary>
    public bool IsProjectNode => !string.IsNullOrEmpty(projectNodePath) || !string.IsNullOrEmpty(projectNodeFileName);

    /// <summary>
    /// Gets the full resource path for ProjectNode export
    /// </summary>
    public string GetProjectNodeResourcePath()
    {
        if (!string.IsNullOrEmpty(projectNodePath))
        {
            return projectNodePath;
        }
        return projectNodeFileName;
    }
}

/// <summary>
/// CSB node types that can be forced via CSBNodeData
/// </summary>
public enum CSBNodeType
{
    Auto,           // Auto-detect from Unity components
    Node,           // Basic node (SingleNodeOptions)
    Sprite,         // SpriteOptions
    ImageView,      // ImageViewOptions (9-slice)
    Text,           // TextOptions
    Button,         // ButtonOptions
    CheckBox,       // CheckBoxOptions
    TextField,      // TextFieldOptions
    Slider,         // SliderOptions
    ScrollView,     // ScrollViewOptions
    ListView,       // ListViewOptions
    PageView,       // PageViewOptions
    Panel,          // PanelOptions
    ProjectNode,    // ProjectNodeOptions (nested CSB)
    LoadingBar      // LoadingBarOptions
}
