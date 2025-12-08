// Unity CSB Exporter
// Exports Unity scenes to Cocos Studio Binary (.csb) format
//
// Prerequisites:
// 1. Install FlatBuffers package: Google.FlatBuffers (via NuGet or Unity Package Manager)
// 2. Generate C# classes from schema: flatc --csharp CSParseBinary.fbs
// 3. Place generated files in your Unity project

using UnityEngine;
using UnityEditor;
using UnityEngine.UI;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using FlatBuffers;
using flatbuffers; // Generated namespace from CSParseBinary.fbs

public class CSBExporter : EditorWindow
{
    private string exportPath = "Assets/ExportedCSB";
    private bool includeAnimations = true;
    private bool includeInactive = false;
    private float animationFrameRate = 60f;

    [MenuItem("Tools/CSB Exporter")]
    public static void ShowWindow()
    {
        GetWindow<CSBExporter>("CSB Exporter");
    }

    private void OnGUI()
    {
        GUILayout.Label("CSB Export Settings", EditorStyles.boldLabel);

        exportPath = EditorGUILayout.TextField("Export Path", exportPath);
        includeAnimations = EditorGUILayout.Toggle("Include Animations", includeAnimations);
        includeInactive = EditorGUILayout.Toggle("Include Inactive Objects", includeInactive);
        animationFrameRate = EditorGUILayout.FloatField("Animation Frame Rate", animationFrameRate);

        GUILayout.Space(10);

        if (Selection.activeGameObject == null)
        {
            EditorGUILayout.HelpBox("Select a GameObject to export", MessageType.Info);
        }
        else
        {
            EditorGUILayout.LabelField("Selected:", Selection.activeGameObject.name);

            if (GUILayout.Button("Export Selected to CSB"))
            {
                ExportSelected();
            }
        }
    }

    private void ExportSelected()
    {
        var root = Selection.activeGameObject;
        if (root == null) return;

        // Ensure export directory exists
        if (!Directory.Exists(exportPath))
        {
            Directory.CreateDirectory(exportPath);
        }

        string filePath = Path.Combine(exportPath, root.name + ".csb");

        try
        {
            byte[] csbData = BuildCSB(root);
            File.WriteAllBytes(filePath, csbData);
            AssetDatabase.Refresh();
            Debug.Log($"CSB exported successfully: {filePath}");
            EditorUtility.DisplayDialog("Export Complete", $"Exported to:\n{filePath}", "OK");
        }
        catch (System.Exception e)
        {
            Debug.LogError($"CSB export failed: {e.Message}\n{e.StackTrace}");
            EditorUtility.DisplayDialog("Export Failed", e.Message, "OK");
        }
    }

    #region CSB Building

    private byte[] BuildCSB(GameObject root)
    {
        var builder = new FlatBufferBuilder(4096);
        var context = new ExportContext();

        // Collect all textures first
        CollectTextures(root, context);

        // Build texture arrays
        var textureOffsets = context.texturePaths.Select(p => builder.CreateString(p)).ToArray();
        var texturesVector = CSParseBinary.CreateTexturesVector(builder, textureOffsets);
        var texturePngsVector = CSParseBinary.CreateTexturePngsVector(builder, textureOffsets);

        // Build node tree
        var nodeTreeOffset = BuildNodeTree(builder, root, context);

        // Build animations
        Offset<NodeAction>? actionOffset = null;
        VectorOffset animationListVector = default;

        if (includeAnimations)
        {
            var animationData = BuildAnimations(builder, root, context);
            actionOffset = animationData.action;
            animationListVector = animationData.animationList;
        }

        // Build root CSParseBinary
        var versionOffset = builder.CreateString("3.10"); // Cocos Studio version compatibility

        CSParseBinary.StartCSParseBinary(builder);
        CSParseBinary.AddVersion(builder, versionOffset);
        CSParseBinary.AddTextures(builder, texturesVector);
        CSParseBinary.AddTexturePngs(builder, texturePngsVector);
        CSParseBinary.AddNodeTree(builder, nodeTreeOffset);

        if (actionOffset.HasValue)
        {
            CSParseBinary.AddAction(builder, actionOffset.Value);
        }
        if (animationListVector.Value != 0)
        {
            CSParseBinary.AddAnimationList(builder, animationListVector);
        }

        var csbOffset = CSParseBinary.EndCSParseBinary(builder);
        builder.Finish(csbOffset.Value);

        return builder.SizedByteArray();
    }

    #endregion

    #region Texture Collection

    private void CollectTextures(GameObject go, ExportContext context)
    {
        // Check SpriteRenderer
        var spriteRenderer = go.GetComponent<SpriteRenderer>();
        if (spriteRenderer != null && spriteRenderer.sprite != null)
        {
            AddTexture(spriteRenderer.sprite.texture, context);
        }

        // Check UI Image
        var image = go.GetComponent<Image>();
        if (image != null && image.sprite != null)
        {
            AddTexture(image.sprite.texture, context);
        }

        // Check RawImage
        var rawImage = go.GetComponent<RawImage>();
        if (rawImage != null && rawImage.texture != null)
        {
            AddTexture(rawImage.texture, context);
        }

        // Recurse children
        foreach (Transform child in go.transform)
        {
            if (includeInactive || child.gameObject.activeSelf)
            {
                CollectTextures(child.gameObject, context);
            }
        }
    }

    private void AddTexture(Texture texture, ExportContext context)
    {
        if (texture == null) return;

        string path = AssetDatabase.GetAssetPath(texture);
        if (string.IsNullOrEmpty(path)) return;

        // Convert to relative path
        string relativePath = Path.GetFileName(path);

        if (!context.textureMap.ContainsKey(texture))
        {
            context.textureMap[texture] = context.texturePaths.Count;
            context.texturePaths.Add(relativePath);
        }
    }

    #endregion

    #region Node Tree Building

    private Offset<NodeTree> BuildNodeTree(FlatBufferBuilder builder, GameObject go, ExportContext context)
    {
        // Determine node class name and build appropriate options
        string className;
        Offset<Options> optionsOffset;

        var spriteRenderer = go.GetComponent<SpriteRenderer>();
        var image = go.GetComponent<Image>();
        var text = go.GetComponent<Text>();
        var button = go.GetComponent<Button>();

        if (spriteRenderer != null)
        {
            className = "Sprite";
            optionsOffset = BuildSpriteOptions(builder, go, spriteRenderer, context);
        }
        else if (image != null)
        {
            className = "ImageView";
            optionsOffset = BuildImageViewOptions(builder, go, image, context);
        }
        else if (text != null)
        {
            className = "Text";
            optionsOffset = BuildTextOptions(builder, go, text, context);
        }
        else if (button != null)
        {
            className = "Button";
            optionsOffset = BuildButtonOptions(builder, go, button, context);
        }
        else
        {
            className = "Node";
            optionsOffset = BuildNodeOptions(builder, go, context);
        }

        // Build children
        var childOffsets = new List<Offset<NodeTree>>();
        foreach (Transform child in go.transform)
        {
            if (includeInactive || child.gameObject.activeSelf)
            {
                childOffsets.Add(BuildNodeTree(builder, child.gameObject, context));
            }
        }

        var classNameOffset = builder.CreateString(className);
        var customClassNameOffset = builder.CreateString("");

        VectorOffset childrenVector = default;
        if (childOffsets.Count > 0)
        {
            childrenVector = NodeTree.CreateChildrenVector(builder, childOffsets.ToArray());
        }

        NodeTree.StartNodeTree(builder);
        NodeTree.AddClassname(builder, classNameOffset);
        NodeTree.AddOptions(builder, optionsOffset);
        NodeTree.AddCustomClassName(builder, customClassNameOffset);
        if (childOffsets.Count > 0)
        {
            NodeTree.AddChildren(builder, childrenVector);
        }

        return NodeTree.EndNodeTree(builder);
    }

    private Offset<Options> BuildNodeOptions(FlatBufferBuilder builder, GameObject go, ExportContext context)
    {
        var widgetOffset = BuildWidgetOptions(builder, go, context);

        Options.StartOptions(builder);
        Options.AddData(builder, widgetOffset);
        return Options.EndOptions(builder);
    }

    private Offset<WidgetOptions> BuildWidgetOptions(FlatBufferBuilder builder, GameObject go, ExportContext context)
    {
        var nameOffset = builder.CreateString(go.name);

        // Get transform data
        Vector3 position = go.transform.localPosition;
        Vector3 scale = go.transform.localScale;
        float rotation = go.transform.localEulerAngles.z;

        // Get RectTransform data if available
        Vector2 size = new Vector2(100, 100);
        Vector2 anchor = new Vector2(0.5f, 0.5f);

        var rectTransform = go.GetComponent<RectTransform>();
        if (rectTransform != null)
        {
            size = rectTransform.sizeDelta;
            anchor = rectTransform.pivot;
            position = rectTransform.anchoredPosition3D;
        }

        // Get color/alpha
        Color color = Color.white;
        byte alpha = 255;

        var spriteRenderer = go.GetComponent<SpriteRenderer>();
        if (spriteRenderer != null)
        {
            color = spriteRenderer.color;
            alpha = (byte)(color.a * 255);
        }

        var image = go.GetComponent<Image>();
        if (image != null)
        {
            color = image.color;
            alpha = (byte)(color.a * 255);
        }

        // Assign action tag for animation binding
        int actionTag = go.GetInstanceID();
        context.actionTagMap[go] = actionTag;

        // Create struct data
        var rotationSkew = RotationSkew.CreateRotationSkew(builder, rotation, rotation);
        var positionStruct = Position.CreatePosition(builder, position.x, position.y);
        var scaleStruct = Scale.CreateScale(builder, scale.x, scale.y);
        var anchorStruct = AnchorPoint.CreateAnchorPoint(builder, anchor.x, anchor.y);
        var colorStruct = flatbuffers.Color.CreateColor(builder, alpha,
            (byte)(color.r * 255), (byte)(color.g * 255), (byte)(color.b * 255));
        var sizeStruct = FlatSize.CreateFlatSize(builder, size.x, size.y);

        // Build WidgetOptions
        WidgetOptions.StartWidgetOptions(builder);
        WidgetOptions.AddName(builder, nameOffset);
        WidgetOptions.AddActionTag(builder, actionTag);
        WidgetOptions.AddRotationSkew(builder, rotationSkew);
        WidgetOptions.AddZOrder(builder, go.transform.GetSiblingIndex());
        WidgetOptions.AddVisible(builder, go.activeSelf);
        WidgetOptions.AddAlpha(builder, alpha);
        WidgetOptions.AddTag(builder, go.GetInstanceID());
        WidgetOptions.AddPosition(builder, positionStruct);
        WidgetOptions.AddScale(builder, scaleStruct);
        WidgetOptions.AddAnchorPoint(builder, anchorStruct);
        WidgetOptions.AddColor(builder, colorStruct);
        WidgetOptions.AddSize(builder, sizeStruct);
        WidgetOptions.AddFlipX(builder, spriteRenderer?.flipX ?? false);
        WidgetOptions.AddFlipY(builder, spriteRenderer?.flipY ?? false);
        WidgetOptions.AddTouchEnabled(builder, go.GetComponent<Button>() != null);

        return WidgetOptions.EndWidgetOptions(builder);
    }

    private Offset<Options> BuildSpriteOptions(FlatBufferBuilder builder, GameObject go,
        SpriteRenderer spriteRenderer, ExportContext context)
    {
        var widgetOffset = BuildWidgetOptions(builder, go, context);

        // Build ResourceData for texture
        Offset<ResourceData> fileNameDataOffset = default;
        if (spriteRenderer.sprite != null)
        {
            fileNameDataOffset = BuildResourceData(builder, spriteRenderer.sprite.texture, context);
        }

        // Build BlendFunc (simplified)
        var blendFunc = BlendFunc.CreateBlendFunc(builder, 1, 771); // GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA

        // For Sprite, we store the options in the generic Options.data field
        // The actual SpriteOptions would need its own table, but CSB format stores it differently
        // For compatibility, we embed sprite-specific data in the widget options

        Options.StartOptions(builder);
        Options.AddData(builder, widgetOffset);
        return Options.EndOptions(builder);
    }

    private Offset<Options> BuildImageViewOptions(FlatBufferBuilder builder, GameObject go,
        Image image, ExportContext context)
    {
        var widgetOffset = BuildWidgetOptions(builder, go, context);

        Options.StartOptions(builder);
        Options.AddData(builder, widgetOffset);
        return Options.EndOptions(builder);
    }

    private Offset<Options> BuildTextOptions(FlatBufferBuilder builder, GameObject go,
        Text text, ExportContext context)
    {
        var widgetOffset = BuildWidgetOptions(builder, go, context);

        // Text-specific options would go here
        // For POC, we just use widget options

        Options.StartOptions(builder);
        Options.AddData(builder, widgetOffset);
        return Options.EndOptions(builder);
    }

    private Offset<Options> BuildButtonOptions(FlatBufferBuilder builder, GameObject go,
        Button button, ExportContext context)
    {
        var widgetOffset = BuildWidgetOptions(builder, go, context);

        Options.StartOptions(builder);
        Options.AddData(builder, widgetOffset);
        return Options.EndOptions(builder);
    }

    private Offset<ResourceData> BuildResourceData(FlatBufferBuilder builder, Texture texture, ExportContext context)
    {
        string path = "";
        if (texture != null && context.textureMap.TryGetValue(texture, out int index))
        {
            path = context.texturePaths[index];
        }

        var pathOffset = builder.CreateString(path);
        var plistOffset = builder.CreateString("");

        ResourceData.StartResourceData(builder);
        ResourceData.AddPath(builder, pathOffset);
        ResourceData.AddPlistFile(builder, plistOffset);
        ResourceData.AddResourceType(builder, 0); // 0 = normal file
        return ResourceData.EndResourceData(builder);
    }

    #endregion

    #region Animation Building

    private (Offset<NodeAction>? action, VectorOffset animationList) BuildAnimations(
        FlatBufferBuilder builder, GameObject root, ExportContext context)
    {
        // Find all Animation/Animator components
        var animations = root.GetComponentsInChildren<Animation>(includeInactive);
        var animators = root.GetComponentsInChildren<Animator>(includeInactive);

        var timelineOffsets = new List<Offset<TimeLine>>();
        var animationInfoOffsets = new List<Offset<AnimationInfo>>();
        int totalDuration = 0;

        // Process legacy Animation components
        foreach (var anim in animations)
        {
            foreach (AnimationState state in anim)
            {
                var clip = state.clip;
                if (clip == null) continue;

                var clipTimelines = BuildClipTimelines(builder, anim.gameObject, clip, context);
                timelineOffsets.AddRange(clipTimelines);

                int clipFrames = Mathf.RoundToInt(clip.length * animationFrameRate);
                totalDuration = Mathf.Max(totalDuration, clipFrames);

                // Add animation info
                var nameOffset = builder.CreateString(clip.name);
                AnimationInfo.StartAnimationInfo(builder);
                AnimationInfo.AddName(builder, nameOffset);
                AnimationInfo.AddStartIndex(builder, 0);
                AnimationInfo.AddEndIndex(builder, clipFrames);
                animationInfoOffsets.Add(AnimationInfo.EndAnimationInfo(builder));
            }
        }

        // Process Animator components (simplified - uses default clip)
        foreach (var animator in animators)
        {
            if (animator.runtimeAnimatorController == null) continue;

            var clips = animator.runtimeAnimatorController.animationClips;
            foreach (var clip in clips)
            {
                var clipTimelines = BuildClipTimelines(builder, animator.gameObject, clip, context);
                timelineOffsets.AddRange(clipTimelines);

                int clipFrames = Mathf.RoundToInt(clip.length * animationFrameRate);
                totalDuration = Mathf.Max(totalDuration, clipFrames);

                var nameOffset = builder.CreateString(clip.name);
                AnimationInfo.StartAnimationInfo(builder);
                AnimationInfo.AddName(builder, nameOffset);
                AnimationInfo.AddStartIndex(builder, 0);
                AnimationInfo.AddEndIndex(builder, clipFrames);
                animationInfoOffsets.Add(AnimationInfo.EndAnimationInfo(builder));
            }
        }

        if (timelineOffsets.Count == 0)
        {
            return (null, default);
        }

        // Build NodeAction
        var timelinesVector = NodeAction.CreateTimeLinesVector(builder, timelineOffsets.ToArray());
        var currentAnimNameOffset = builder.CreateString(animationInfoOffsets.Count > 0 ? "Default" : "");

        NodeAction.StartNodeAction(builder);
        NodeAction.AddDuration(builder, totalDuration);
        NodeAction.AddSpeed(builder, 1.0f);
        NodeAction.AddTimeLines(builder, timelinesVector);
        NodeAction.AddCurrentAnimationName(builder, currentAnimNameOffset);
        var actionOffset = NodeAction.EndNodeAction(builder);

        // Build animation list
        VectorOffset animListVector = default;
        if (animationInfoOffsets.Count > 0)
        {
            animListVector = CSParseBinary.CreateAnimationListVector(builder, animationInfoOffsets.ToArray());
        }

        return (actionOffset, animListVector);
    }

    private List<Offset<TimeLine>> BuildClipTimelines(FlatBufferBuilder builder, GameObject target,
        AnimationClip clip, ExportContext context)
    {
        var timelines = new List<Offset<TimeLine>>();

        // Get action tag for target
        if (!context.actionTagMap.TryGetValue(target, out int actionTag))
        {
            actionTag = target.GetInstanceID();
        }

        // Get all curve bindings
        var bindings = AnimationUtility.GetCurveBindings(clip);

        // Group by property type
        var positionXCurve = bindings.FirstOrDefault(b => b.propertyName == "m_LocalPosition.x");
        var positionYCurve = bindings.FirstOrDefault(b => b.propertyName == "m_LocalPosition.y");
        var scaleXCurve = bindings.FirstOrDefault(b => b.propertyName == "m_LocalScale.x");
        var scaleYCurve = bindings.FirstOrDefault(b => b.propertyName == "m_LocalScale.y");
        var rotationZCurve = bindings.FirstOrDefault(b => b.propertyName == "localEulerAnglesRaw.z");
        var colorRCurve = bindings.FirstOrDefault(b => b.propertyName.Contains("m_Color.r"));
        var colorGCurve = bindings.FirstOrDefault(b => b.propertyName.Contains("m_Color.g"));
        var colorBCurve = bindings.FirstOrDefault(b => b.propertyName.Contains("m_Color.b"));
        var colorACurve = bindings.FirstOrDefault(b => b.propertyName.Contains("m_Color.a"));

        // Build Position timeline
        if (positionXCurve.propertyName != null || positionYCurve.propertyName != null)
        {
            var xCurve = positionXCurve.propertyName != null ?
                AnimationUtility.GetEditorCurve(clip, positionXCurve) : null;
            var yCurve = positionYCurve.propertyName != null ?
                AnimationUtility.GetEditorCurve(clip, positionYCurve) : null;

            var timeline = BuildPositionTimeline(builder, actionTag, xCurve, yCurve, clip.length);
            if (timeline.HasValue)
            {
                timelines.Add(timeline.Value);
            }
        }

        // Build Scale timeline
        if (scaleXCurve.propertyName != null || scaleYCurve.propertyName != null)
        {
            var xCurve = scaleXCurve.propertyName != null ?
                AnimationUtility.GetEditorCurve(clip, scaleXCurve) : null;
            var yCurve = scaleYCurve.propertyName != null ?
                AnimationUtility.GetEditorCurve(clip, scaleYCurve) : null;

            var timeline = BuildScaleTimeline(builder, actionTag, xCurve, yCurve, clip.length);
            if (timeline.HasValue)
            {
                timelines.Add(timeline.Value);
            }
        }

        // Build Color timeline
        if (colorRCurve.propertyName != null || colorACurve.propertyName != null)
        {
            var rCurve = colorRCurve.propertyName != null ?
                AnimationUtility.GetEditorCurve(clip, colorRCurve) : null;
            var gCurve = colorGCurve.propertyName != null ?
                AnimationUtility.GetEditorCurve(clip, colorGCurve) : null;
            var bCurve = colorBCurve.propertyName != null ?
                AnimationUtility.GetEditorCurve(clip, colorBCurve) : null;
            var aCurve = colorACurve.propertyName != null ?
                AnimationUtility.GetEditorCurve(clip, colorACurve) : null;

            var timeline = BuildColorTimeline(builder, actionTag, rCurve, gCurve, bCurve, aCurve, clip.length);
            if (timeline.HasValue)
            {
                timelines.Add(timeline.Value);
            }
        }

        return timelines;
    }

    private Offset<TimeLine>? BuildPositionTimeline(FlatBufferBuilder builder, int actionTag,
        AnimationCurve xCurve, AnimationCurve yCurve, float clipLength)
    {
        var frameOffsets = new List<Offset<Frame>>();

        // Sample keyframes
        var keyframeTimes = new HashSet<float>();
        if (xCurve != null) foreach (var key in xCurve.keys) keyframeTimes.Add(key.time);
        if (yCurve != null) foreach (var key in yCurve.keys) keyframeTimes.Add(key.time);

        foreach (float time in keyframeTimes.OrderBy(t => t))
        {
            int frameIndex = Mathf.RoundToInt(time * animationFrameRate);
            float x = xCurve?.Evaluate(time) ?? 0;
            float y = yCurve?.Evaluate(time) ?? 0;

            var posStruct = Position.CreatePosition(builder, x, y);
            var easingOffset = BuildEasingData(builder, EasingType.Linear);

            PointFrame.StartPointFrame(builder);
            PointFrame.AddFrameIndex(builder, frameIndex);
            PointFrame.AddTween(builder, true);
            PointFrame.AddPosition(builder, posStruct);
            PointFrame.AddEasingData(builder, easingOffset);
            var pointFrameOffset = PointFrame.EndPointFrame(builder);

            Frame.StartFrame(builder);
            Frame.AddPointFrame(builder, pointFrameOffset);
            frameOffsets.Add(Frame.EndFrame(builder));
        }

        if (frameOffsets.Count == 0) return null;

        var propertyOffset = builder.CreateString("Position");
        var framesVector = TimeLine.CreateFramesVector(builder, frameOffsets.ToArray());

        TimeLine.StartTimeLine(builder);
        TimeLine.AddProperty(builder, propertyOffset);
        TimeLine.AddActionTag(builder, actionTag);
        TimeLine.AddFrames(builder, framesVector);
        return TimeLine.EndTimeLine(builder);
    }

    private Offset<TimeLine>? BuildScaleTimeline(FlatBufferBuilder builder, int actionTag,
        AnimationCurve xCurve, AnimationCurve yCurve, float clipLength)
    {
        var frameOffsets = new List<Offset<Frame>>();

        var keyframeTimes = new HashSet<float>();
        if (xCurve != null) foreach (var key in xCurve.keys) keyframeTimes.Add(key.time);
        if (yCurve != null) foreach (var key in yCurve.keys) keyframeTimes.Add(key.time);

        foreach (float time in keyframeTimes.OrderBy(t => t))
        {
            int frameIndex = Mathf.RoundToInt(time * animationFrameRate);
            float x = xCurve?.Evaluate(time) ?? 1;
            float y = yCurve?.Evaluate(time) ?? 1;

            var scaleStruct = Scale.CreateScale(builder, x, y);
            var easingOffset = BuildEasingData(builder, EasingType.Linear);

            ScaleFrame.StartScaleFrame(builder);
            ScaleFrame.AddFrameIndex(builder, frameIndex);
            ScaleFrame.AddTween(builder, true);
            ScaleFrame.AddScale(builder, scaleStruct);
            ScaleFrame.AddEasingData(builder, easingOffset);
            var scaleFrameOffset = ScaleFrame.EndScaleFrame(builder);

            Frame.StartFrame(builder);
            Frame.AddScaleFrame(builder, scaleFrameOffset);
            frameOffsets.Add(Frame.EndFrame(builder));
        }

        if (frameOffsets.Count == 0) return null;

        var propertyOffset = builder.CreateString("Scale");
        var framesVector = TimeLine.CreateFramesVector(builder, frameOffsets.ToArray());

        TimeLine.StartTimeLine(builder);
        TimeLine.AddProperty(builder, propertyOffset);
        TimeLine.AddActionTag(builder, actionTag);
        TimeLine.AddFrames(builder, framesVector);
        return TimeLine.EndTimeLine(builder);
    }

    private Offset<TimeLine>? BuildColorTimeline(FlatBufferBuilder builder, int actionTag,
        AnimationCurve rCurve, AnimationCurve gCurve, AnimationCurve bCurve, AnimationCurve aCurve, float clipLength)
    {
        var frameOffsets = new List<Offset<Frame>>();

        var keyframeTimes = new HashSet<float>();
        if (rCurve != null) foreach (var key in rCurve.keys) keyframeTimes.Add(key.time);
        if (gCurve != null) foreach (var key in gCurve.keys) keyframeTimes.Add(key.time);
        if (bCurve != null) foreach (var key in bCurve.keys) keyframeTimes.Add(key.time);
        if (aCurve != null) foreach (var key in aCurve.keys) keyframeTimes.Add(key.time);

        foreach (float time in keyframeTimes.OrderBy(t => t))
        {
            int frameIndex = Mathf.RoundToInt(time * animationFrameRate);
            byte r = (byte)((rCurve?.Evaluate(time) ?? 1) * 255);
            byte g = (byte)((gCurve?.Evaluate(time) ?? 1) * 255);
            byte b = (byte)((bCurve?.Evaluate(time) ?? 1) * 255);
            byte a = (byte)((aCurve?.Evaluate(time) ?? 1) * 255);

            var colorStruct = flatbuffers.Color.CreateColor(builder, a, r, g, b);
            var easingOffset = BuildEasingData(builder, EasingType.Linear);

            ColorFrame.StartColorFrame(builder);
            ColorFrame.AddFrameIndex(builder, frameIndex);
            ColorFrame.AddTween(builder, true);
            ColorFrame.AddColor(builder, colorStruct);
            ColorFrame.AddEasingData(builder, easingOffset);
            var colorFrameOffset = ColorFrame.EndColorFrame(builder);

            Frame.StartFrame(builder);
            Frame.AddColorFrame(builder, colorFrameOffset);
            frameOffsets.Add(Frame.EndFrame(builder));
        }

        if (frameOffsets.Count == 0) return null;

        var propertyOffset = builder.CreateString("Color");
        var framesVector = TimeLine.CreateFramesVector(builder, frameOffsets.ToArray());

        TimeLine.StartTimeLine(builder);
        TimeLine.AddProperty(builder, propertyOffset);
        TimeLine.AddActionTag(builder, actionTag);
        TimeLine.AddFrames(builder, framesVector);
        return TimeLine.EndTimeLine(builder);
    }

    private Offset<EasingData> BuildEasingData(FlatBufferBuilder builder, EasingType type)
    {
        EasingData.StartEasingData(builder);
        EasingData.AddType(builder, (int)type);
        return EasingData.EndEasingData(builder);
    }

    #endregion

    #region Helper Classes

    private class ExportContext
    {
        public List<string> texturePaths = new List<string>();
        public Dictionary<Texture, int> textureMap = new Dictionary<Texture, int>();
        public Dictionary<GameObject, int> actionTagMap = new Dictionary<GameObject, int>();
    }

    // CSB Easing types (matches Cocos Studio)
    private enum EasingType
    {
        Linear = 0,
        EaseInSine = 1,
        EaseOutSine = 2,
        EaseInOutSine = 3,
        EaseInQuad = 4,
        EaseOutQuad = 5,
        EaseInOutQuad = 6,
        EaseInCubic = 7,
        EaseOutCubic = 8,
        EaseInOutCubic = 9,
        EaseInQuart = 10,
        EaseOutQuart = 11,
        EaseInOutQuart = 12,
        EaseInQuint = 13,
        EaseOutQuint = 14,
        EaseInOutQuint = 15,
        EaseInExpo = 16,
        EaseOutExpo = 17,
        EaseInOutExpo = 18,
        EaseInCirc = 19,
        EaseOutCirc = 20,
        EaseInOutCirc = 21,
        EaseInElastic = 22,
        EaseOutElastic = 23,
        EaseInOutElastic = 24,
        EaseInBack = 25,
        EaseOutBack = 26,
        EaseInOutBack = 27,
        EaseInBounce = 28,
        EaseOutBounce = 29,
        EaseInOutBounce = 30,
    }

    #endregion
}
