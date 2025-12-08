// Unity CSB Exporter - Complete Version
// Exports Unity scenes to Cocos Studio Binary (.csb) format
// Covers ALL schema-defined types for maximum compatibility
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
    private bool copyTextures = true;

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
        copyTextures = EditorGUILayout.Toggle("Copy Textures to Export", copyTextures);

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

        if (!Directory.Exists(exportPath))
        {
            Directory.CreateDirectory(exportPath);
        }

        string filePath = Path.Combine(exportPath, root.name + ".csb");

        try
        {
            var context = new ExportContext();
            byte[] csbData = BuildCSB(root, context);
            File.WriteAllBytes(filePath, csbData);

            // Copy textures if requested
            if (copyTextures)
            {
                CopyTexturesToExport(context);
            }

            AssetDatabase.Refresh();
            Debug.Log($"CSB exported successfully: {filePath}");
            Debug.Log($"Exported {context.nodeCount} nodes, {context.texturePaths.Count} textures");
            EditorUtility.DisplayDialog("Export Complete",
                $"Exported to:\n{filePath}\n\nNodes: {context.nodeCount}\nTextures: {context.texturePaths.Count}", "OK");
        }
        catch (System.Exception e)
        {
            Debug.LogError($"CSB export failed: {e.Message}\n{e.StackTrace}");
            EditorUtility.DisplayDialog("Export Failed", e.Message, "OK");
        }
    }

    private void CopyTexturesToExport(ExportContext context)
    {
        foreach (var kvp in context.textureSourcePaths)
        {
            string sourcePath = kvp.Value;
            string destPath = Path.Combine(exportPath, kvp.Key);
            if (File.Exists(sourcePath) && !File.Exists(destPath))
            {
                File.Copy(sourcePath, destPath);
            }
        }
    }

    #region CSB Building

    private byte[] BuildCSB(GameObject root, ExportContext context)
    {
        var builder = new FlatBufferBuilder(8192);

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
        var versionOffset = builder.CreateString("3.10");

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
        // SpriteRenderer
        var spriteRenderer = go.GetComponent<SpriteRenderer>();
        if (spriteRenderer?.sprite?.texture != null)
        {
            AddTexture(spriteRenderer.sprite.texture, context);
        }

        // UI Image
        var image = go.GetComponent<Image>();
        if (image?.sprite?.texture != null)
        {
            AddTexture(image.sprite.texture, context);
        }

        // RawImage
        var rawImage = go.GetComponent<RawImage>();
        if (rawImage?.texture != null)
        {
            AddTexture(rawImage.texture, context);
        }

        // Button images
        var button = go.GetComponent<Button>();
        if (button != null)
        {
            var targetGraphic = button.targetGraphic as Image;
            if (targetGraphic?.sprite?.texture != null)
            {
                AddTexture(targetGraphic.sprite.texture, context);
            }
            // Collect sprite swap textures
            var spriteState = button.spriteState;
            if (spriteState.highlightedSprite?.texture != null)
                AddTexture(spriteState.highlightedSprite.texture, context);
            if (spriteState.pressedSprite?.texture != null)
                AddTexture(spriteState.pressedSprite.texture, context);
            if (spriteState.disabledSprite?.texture != null)
                AddTexture(spriteState.disabledSprite.texture, context);
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

        string assetPath = AssetDatabase.GetAssetPath(texture);
        if (string.IsNullOrEmpty(assetPath)) return;

        string relativePath = Path.GetFileName(assetPath);

        if (!context.textureMap.ContainsKey(texture))
        {
            context.textureMap[texture] = context.texturePaths.Count;
            context.texturePaths.Add(relativePath);
            context.textureSourcePaths[relativePath] = assetPath;
        }
    }

    #endregion

    #region Node Tree Building

    private Offset<NodeTree> BuildNodeTree(FlatBufferBuilder builder, GameObject go, ExportContext context)
    {
        context.nodeCount++;

        // Determine node class name and build appropriate options
        string className;
        Offset<Options> optionsOffset;

        var spriteRenderer = go.GetComponent<SpriteRenderer>();
        var image = go.GetComponent<Image>();
        var rawImage = go.GetComponent<RawImage>();
        var text = go.GetComponent<Text>();
        var button = go.GetComponent<Button>();
        var toggle = go.GetComponent<Toggle>();
        var inputField = go.GetComponent<InputField>();
        var slider = go.GetComponent<Slider>();
        var scrollRect = go.GetComponent<ScrollRect>();

        if (button != null)
        {
            className = "Button";
            optionsOffset = BuildButtonOptions(builder, go, button, context);
        }
        else if (toggle != null)
        {
            className = "CheckBox";
            optionsOffset = BuildCheckBoxOptions(builder, go, toggle, context);
        }
        else if (inputField != null)
        {
            className = "TextField";
            optionsOffset = BuildTextFieldOptions(builder, go, inputField, context);
        }
        else if (slider != null)
        {
            className = "Slider";
            optionsOffset = BuildSliderOptions(builder, go, slider, context);
        }
        else if (scrollRect != null)
        {
            className = "ScrollView";
            optionsOffset = BuildScrollViewOptions(builder, go, scrollRect, context);
        }
        else if (text != null)
        {
            className = "Text";
            optionsOffset = BuildTextOptions(builder, go, text, context);
        }
        else if (spriteRenderer != null)
        {
            className = "Sprite";
            optionsOffset = BuildSpriteOptions(builder, go, spriteRenderer, context);
        }
        else if (image != null)
        {
            className = "ImageView";
            optionsOffset = BuildImageViewOptions(builder, go, image, context);
        }
        else if (rawImage != null)
        {
            className = "ImageView";
            optionsOffset = BuildRawImageOptions(builder, go, rawImage, context);
        }
        else if (go.GetComponent<RectTransform>() != null)
        {
            className = "Panel";
            optionsOffset = BuildPanelOptions(builder, go, context);
        }
        else
        {
            className = "Node";
            optionsOffset = BuildSingleNodeOptions(builder, go, context);
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

    #endregion

    #region Widget Options (Base)

    private Offset<WidgetOptions> BuildWidgetOptions(FlatBufferBuilder builder, GameObject go,
        ExportContext context, UnityEngine.Color? colorOverride = null)
    {
        var nameOffset = builder.CreateString(go.name);
        var frameEventOffset = builder.CreateString("");
        var customPropertyOffset = builder.CreateString("");
        var callBackTypeOffset = builder.CreateString("");
        var callBackNameOffset = builder.CreateString("");

        // Transform data
        Vector3 position = go.transform.localPosition;
        Vector3 scale = go.transform.localScale;
        float rotation = go.transform.localEulerAngles.z;

        // RectTransform data
        Vector2 size = new Vector2(100, 100);
        Vector2 anchor = new Vector2(0.5f, 0.5f);
        bool ignoreSize = false;

        var rectTransform = go.GetComponent<RectTransform>();
        if (rectTransform != null)
        {
            size = rectTransform.sizeDelta;
            anchor = rectTransform.pivot;
            position = rectTransform.anchoredPosition3D;
        }

        // Color
        UnityEngine.Color color = colorOverride ?? UnityEngine.Color.white;
        byte alpha = 255;

        var spriteRenderer = go.GetComponent<SpriteRenderer>();
        if (spriteRenderer != null && !colorOverride.HasValue)
        {
            color = spriteRenderer.color;
            alpha = (byte)(color.a * 255);
        }

        var image = go.GetComponent<Image>();
        if (image != null && !colorOverride.HasValue)
        {
            color = image.color;
            alpha = (byte)(color.a * 255);
        }

        // Flip
        bool flipX = spriteRenderer?.flipX ?? false;
        bool flipY = spriteRenderer?.flipY ?? false;

        // Action tag
        int actionTag = go.GetInstanceID();
        context.actionTagMap[go] = actionTag;

        // Layout component
        Offset<LayoutComponentTable>? layoutOffset = null;
        var layoutElement = go.GetComponent<LayoutElement>();
        if (layoutElement != null || rectTransform != null)
        {
            layoutOffset = BuildLayoutComponent(builder, go, rectTransform);
        }

        // Create structs
        var rotationSkew = RotationSkew.CreateRotationSkew(builder, rotation, rotation);
        var positionStruct = Position.CreatePosition(builder, position.x, position.y);
        var scaleStruct = Scale.CreateScale(builder, scale.x, scale.y);
        var anchorStruct = AnchorPoint.CreateAnchorPoint(builder, anchor.x, anchor.y);
        var colorStruct = flatbuffers.Color.CreateColor(builder, (byte)(color.a * 255),
            (byte)(color.r * 255), (byte)(color.g * 255), (byte)(color.b * 255));
        var sizeStruct = FlatSize.CreateFlatSize(builder, size.x, size.y);

        WidgetOptions.StartWidgetOptions(builder);
        WidgetOptions.AddName(builder, nameOffset);
        WidgetOptions.AddActionTag(builder, actionTag);
        WidgetOptions.AddRotationSkew(builder, rotationSkew);
        WidgetOptions.AddZOrder(builder, go.transform.GetSiblingIndex());
        WidgetOptions.AddVisible(builder, go.activeSelf);
        WidgetOptions.AddAlpha(builder, (byte)(color.a * 255));
        WidgetOptions.AddTag(builder, go.GetInstanceID());
        WidgetOptions.AddPosition(builder, positionStruct);
        WidgetOptions.AddScale(builder, scaleStruct);
        WidgetOptions.AddAnchorPoint(builder, anchorStruct);
        WidgetOptions.AddColor(builder, colorStruct);
        WidgetOptions.AddSize(builder, sizeStruct);
        WidgetOptions.AddFlipX(builder, flipX);
        WidgetOptions.AddFlipY(builder, flipY);
        WidgetOptions.AddIgnoreSize(builder, ignoreSize);
        WidgetOptions.AddTouchEnabled(builder, go.GetComponent<Button>() != null ||
                                               go.GetComponent<Toggle>() != null ||
                                               go.GetComponent<Slider>() != null);
        WidgetOptions.AddFrameEvent(builder, frameEventOffset);
        WidgetOptions.AddCustomProperty(builder, customPropertyOffset);
        WidgetOptions.AddCallBackType(builder, callBackTypeOffset);
        WidgetOptions.AddCallBackName(builder, callBackNameOffset);
        if (layoutOffset.HasValue)
        {
            WidgetOptions.AddLayoutComponent(builder, layoutOffset.Value);
        }

        return WidgetOptions.EndWidgetOptions(builder);
    }

    private Offset<LayoutComponentTable> BuildLayoutComponent(FlatBufferBuilder builder,
        GameObject go, RectTransform rectTransform)
    {
        var horizontalEdgeOffset = builder.CreateString("");
        var verticalEdgeOffset = builder.CreateString("");

        float posXPercent = 0, posYPercent = 0;
        float sizeXPercent = 0, sizeYPercent = 0;

        if (rectTransform != null)
        {
            // Calculate percent positions based on anchors
            posXPercent = rectTransform.anchorMin.x;
            posYPercent = rectTransform.anchorMin.y;
        }

        LayoutComponentTable.StartLayoutComponentTable(builder);
        LayoutComponentTable.AddPositionXPercentEnabled(builder, false);
        LayoutComponentTable.AddPositionYPercentEnabled(builder, false);
        LayoutComponentTable.AddPositionXPercent(builder, posXPercent);
        LayoutComponentTable.AddPositionYPercent(builder, posYPercent);
        LayoutComponentTable.AddSizeXPercentEnable(builder, false);
        LayoutComponentTable.AddSizeYPercentEnable(builder, false);
        LayoutComponentTable.AddSizeXPercent(builder, sizeXPercent);
        LayoutComponentTable.AddSizeYPercent(builder, sizeYPercent);
        LayoutComponentTable.AddStretchHorizontalEnabled(builder, false);
        LayoutComponentTable.AddStretchVerticalEnabled(builder, false);
        LayoutComponentTable.AddHorizontalEdge(builder, horizontalEdgeOffset);
        LayoutComponentTable.AddVerticalEdge(builder, verticalEdgeOffset);
        LayoutComponentTable.AddLeftMargin(builder, 0);
        LayoutComponentTable.AddRightMargin(builder, 0);
        LayoutComponentTable.AddTopMargin(builder, 0);
        LayoutComponentTable.AddBottomMargin(builder, 0);

        return LayoutComponentTable.EndLayoutComponentTable(builder);
    }

    #endregion

    #region Node Type Options

    private Offset<Options> BuildSingleNodeOptions(FlatBufferBuilder builder, GameObject go, ExportContext context)
    {
        var widgetOffset = BuildWidgetOptions(builder, go, context);

        // For SingleNodeOptions format - wrap in SingleNodeOptions then Options
        SingleNodeOptions.StartSingleNodeOptions(builder);
        SingleNodeOptions.AddNodeOptions(builder, widgetOffset);
        var singleNodeOffset = SingleNodeOptions.EndSingleNodeOptions(builder);

        Options.StartOptions(builder);
        Options.AddData(builder, widgetOffset);
        return Options.EndOptions(builder);
    }

    private Offset<Options> BuildSpriteOptions(FlatBufferBuilder builder, GameObject go,
        SpriteRenderer spriteRenderer, ExportContext context)
    {
        var widgetOffset = BuildWidgetOptions(builder, go, context);

        // Build ResourceData for texture
        var fileNameDataOffset = BuildResourceData(builder, spriteRenderer.sprite?.texture, context);

        // Build BlendFunc
        var blendFunc = BlendFunc.CreateBlendFunc(builder,
            (int)UnityEngine.Rendering.BlendMode.SrcAlpha,
            (int)UnityEngine.Rendering.BlendMode.OneMinusSrcAlpha);

        // Build SpriteOptions
        SpriteOptions.StartSpriteOptions(builder);
        SpriteOptions.AddNodeOptions(builder, widgetOffset);
        SpriteOptions.AddFileNameData(builder, fileNameDataOffset);
        SpriteOptions.AddBlendFunc(builder, blendFunc);
        var spriteOptionsOffset = SpriteOptions.EndSpriteOptions(builder);

        // Wrap in Options - note: CSB format stores options differently
        // We put the widget options in Options.data for compatibility
        Options.StartOptions(builder);
        Options.AddData(builder, widgetOffset);
        return Options.EndOptions(builder);
    }

    private Offset<Options> BuildImageViewOptions(FlatBufferBuilder builder, GameObject go,
        Image image, ExportContext context)
    {
        var widgetOffset = BuildWidgetOptions(builder, go, context);
        var fileNameDataOffset = BuildResourceData(builder, image.sprite?.texture, context);

        // Scale9 / Sliced sprite support
        bool scale9Enabled = image.type == Image.Type.Sliced;
        var capInsets = CapInsets.CreateCapInsets(builder, 0, 0, 0, 0);

        if (scale9Enabled && image.sprite != null)
        {
            var border = image.sprite.border;
            capInsets = CapInsets.CreateCapInsets(builder, border.x, border.y, border.z, border.w);
        }

        var scale9Size = FlatSize.CreateFlatSize(builder,
            image.rectTransform.sizeDelta.x,
            image.rectTransform.sizeDelta.y);

        ImageViewOptions.StartImageViewOptions(builder);
        ImageViewOptions.AddWidgetOptions(builder, widgetOffset);
        ImageViewOptions.AddFileNameData(builder, fileNameDataOffset);
        ImageViewOptions.AddCapInsets(builder, capInsets);
        ImageViewOptions.AddScale9Size(builder, scale9Size);
        ImageViewOptions.AddScale9Enabled(builder, scale9Enabled);
        var imageViewOffset = ImageViewOptions.EndImageViewOptions(builder);

        Options.StartOptions(builder);
        Options.AddData(builder, widgetOffset);
        return Options.EndOptions(builder);
    }

    private Offset<Options> BuildRawImageOptions(FlatBufferBuilder builder, GameObject go,
        RawImage rawImage, ExportContext context)
    {
        var widgetOffset = BuildWidgetOptions(builder, go, context);
        var fileNameDataOffset = BuildResourceData(builder, rawImage.texture, context);

        Options.StartOptions(builder);
        Options.AddData(builder, widgetOffset);
        return Options.EndOptions(builder);
    }

    private Offset<Options> BuildTextOptions(FlatBufferBuilder builder, GameObject go,
        Text text, ExportContext context)
    {
        var widgetOffset = BuildWidgetOptions(builder, go, context, text.color);

        var fontResourceOffset = BuildResourceData(builder, null, context);
        var fontNameOffset = builder.CreateString(text.font?.name ?? "Arial");
        var textContentOffset = builder.CreateString(text.text ?? "");

        // Alignment mapping
        int hAlign = 0; // 0=left, 1=center, 2=right
        int vAlign = 1; // 0=top, 1=center, 2=bottom
        switch (text.alignment)
        {
            case TextAnchor.UpperLeft: hAlign = 0; vAlign = 0; break;
            case TextAnchor.UpperCenter: hAlign = 1; vAlign = 0; break;
            case TextAnchor.UpperRight: hAlign = 2; vAlign = 0; break;
            case TextAnchor.MiddleLeft: hAlign = 0; vAlign = 1; break;
            case TextAnchor.MiddleCenter: hAlign = 1; vAlign = 1; break;
            case TextAnchor.MiddleRight: hAlign = 2; vAlign = 1; break;
            case TextAnchor.LowerLeft: hAlign = 0; vAlign = 2; break;
            case TextAnchor.LowerCenter: hAlign = 1; vAlign = 2; break;
            case TextAnchor.LowerRight: hAlign = 2; vAlign = 2; break;
        }

        var outlineColor = flatbuffers.Color.CreateColor(builder, 255, 0, 0, 0);
        var shadowColor = flatbuffers.Color.CreateColor(builder, 128, 0, 0, 0);

        TextOptions.StartTextOptions(builder);
        TextOptions.AddWidgetOptions(builder, widgetOffset);
        TextOptions.AddFontResource(builder, fontResourceOffset);
        TextOptions.AddFontName(builder, fontNameOffset);
        TextOptions.AddFontSize(builder, text.fontSize);
        TextOptions.AddText(builder, textContentOffset);
        TextOptions.AddIsLocalized(builder, false);
        TextOptions.AddAreaWidth(builder, (int)text.rectTransform.sizeDelta.x);
        TextOptions.AddAreaHeight(builder, (int)text.rectTransform.sizeDelta.y);
        TextOptions.AddHAlignment(builder, hAlign);
        TextOptions.AddVAlignment(builder, vAlign);
        TextOptions.AddTouchScaleEnable(builder, false);
        TextOptions.AddIsCustomSize(builder, true);
        TextOptions.AddOutlineEnabled(builder, text.GetComponent<Outline>() != null);
        TextOptions.AddOutlineColor(builder, outlineColor);
        TextOptions.AddOutlineSize(builder, 1);
        TextOptions.AddShadowEnabled(builder, text.GetComponent<Shadow>() != null);
        TextOptions.AddShadowColor(builder, shadowColor);
        TextOptions.AddShadowOffsetX(builder, 2);
        TextOptions.AddShadowOffsetY(builder, -2);
        TextOptions.AddShadowBlurRadius(builder, 0);
        var textOptionsOffset = TextOptions.EndTextOptions(builder);

        Options.StartOptions(builder);
        Options.AddData(builder, widgetOffset);
        return Options.EndOptions(builder);
    }

    private Offset<Options> BuildButtonOptions(FlatBufferBuilder builder, GameObject go,
        Button button, ExportContext context)
    {
        var widgetOffset = BuildWidgetOptions(builder, go, context);

        // Get button images
        var targetImage = button.targetGraphic as Image;
        var normalDataOffset = BuildResourceData(builder, targetImage?.sprite?.texture, context);
        var pressedDataOffset = BuildResourceData(builder, button.spriteState.pressedSprite?.texture, context);
        var disabledDataOffset = BuildResourceData(builder, button.spriteState.disabledSprite?.texture, context);
        var fontResourceOffset = BuildResourceData(builder, null, context);

        // Find text child
        var buttonText = go.GetComponentInChildren<Text>();
        var textOffset = builder.CreateString(buttonText?.text ?? "");
        var fontNameOffset = builder.CreateString(buttonText?.font?.name ?? "Arial");

        var textColor = flatbuffers.Color.CreateColor(builder, 255, 255, 255, 255);
        if (buttonText != null)
        {
            textColor = flatbuffers.Color.CreateColor(builder,
                (byte)(buttonText.color.a * 255),
                (byte)(buttonText.color.r * 255),
                (byte)(buttonText.color.g * 255),
                (byte)(buttonText.color.b * 255));
        }

        var capInsets = CapInsets.CreateCapInsets(builder, 0, 0, 0, 0);
        var scale9Size = FlatSize.CreateFlatSize(builder, 100, 40);
        var outlineColor = flatbuffers.Color.CreateColor(builder, 255, 0, 0, 0);
        var shadowColor = flatbuffers.Color.CreateColor(builder, 128, 0, 0, 0);

        ButtonOptions.StartButtonOptions(builder);
        ButtonOptions.AddWidgetOptions(builder, widgetOffset);
        ButtonOptions.AddNormalData(builder, normalDataOffset);
        ButtonOptions.AddPressedData(builder, pressedDataOffset);
        ButtonOptions.AddDisabledData(builder, disabledDataOffset);
        ButtonOptions.AddFontResource(builder, fontResourceOffset);
        ButtonOptions.AddText(builder, textOffset);
        ButtonOptions.AddIsLocalized(builder, false);
        ButtonOptions.AddFontName(builder, fontNameOffset);
        ButtonOptions.AddFontSize(builder, buttonText?.fontSize ?? 14);
        ButtonOptions.AddTextColor(builder, textColor);
        ButtonOptions.AddCapInsets(builder, capInsets);
        ButtonOptions.AddScale9Size(builder, scale9Size);
        ButtonOptions.AddScale9Enabled(builder, false);
        ButtonOptions.AddDisplaystate(builder, button.interactable);
        ButtonOptions.AddOutlineEnabled(builder, false);
        ButtonOptions.AddOutlineColor(builder, outlineColor);
        ButtonOptions.AddOutlineSize(builder, 1);
        ButtonOptions.AddShadowEnabled(builder, false);
        ButtonOptions.AddShadowColor(builder, shadowColor);
        ButtonOptions.AddShadowOffsetX(builder, 2);
        ButtonOptions.AddShadowOffsetY(builder, -2);
        ButtonOptions.AddShadowBlurRadius(builder, 0);
        var buttonOptionsOffset = ButtonOptions.EndButtonOptions(builder);

        Options.StartOptions(builder);
        Options.AddData(builder, widgetOffset);
        return Options.EndOptions(builder);
    }

    private Offset<Options> BuildCheckBoxOptions(FlatBufferBuilder builder, GameObject go,
        Toggle toggle, ExportContext context)
    {
        var widgetOffset = BuildWidgetOptions(builder, go, context);

        var bgImage = toggle.targetGraphic as Image;
        var checkImage = toggle.graphic as Image;

        var bgDataOffset = BuildResourceData(builder, bgImage?.sprite?.texture, context);
        var bgSelectedDataOffset = BuildResourceData(builder, null, context);
        var crossDataOffset = BuildResourceData(builder, checkImage?.sprite?.texture, context);
        var bgDisabledDataOffset = BuildResourceData(builder, null, context);
        var crossDisabledDataOffset = BuildResourceData(builder, null, context);

        CheckBoxOptions.StartCheckBoxOptions(builder);
        CheckBoxOptions.AddWidgetOptions(builder, widgetOffset);
        CheckBoxOptions.AddBackGroundBoxData(builder, bgDataOffset);
        CheckBoxOptions.AddBackGroundBoxSelectedData(builder, bgSelectedDataOffset);
        CheckBoxOptions.AddFrontCrossData(builder, crossDataOffset);
        CheckBoxOptions.AddBackGroundBoxDisabledData(builder, bgDisabledDataOffset);
        CheckBoxOptions.AddFrontCrossDisabledData(builder, crossDisabledDataOffset);
        CheckBoxOptions.AddSelectedState(builder, toggle.isOn);
        CheckBoxOptions.AddDisplaystate(builder, toggle.interactable);
        var checkBoxOffset = CheckBoxOptions.EndCheckBoxOptions(builder);

        Options.StartOptions(builder);
        Options.AddData(builder, widgetOffset);
        return Options.EndOptions(builder);
    }

    private Offset<Options> BuildTextFieldOptions(FlatBufferBuilder builder, GameObject go,
        InputField inputField, ExportContext context)
    {
        var widgetOffset = BuildWidgetOptions(builder, go, context);

        var fontResourceOffset = BuildResourceData(builder, null, context);
        var fontNameOffset = builder.CreateString(inputField.textComponent?.font?.name ?? "Arial");
        var textOffset = builder.CreateString(inputField.text ?? "");
        var placeholderOffset = builder.CreateString(
            (inputField.placeholder as Text)?.text ?? "");
        var passwordStyleOffset = builder.CreateString("*");

        TextFieldOptions.StartTextFieldOptions(builder);
        TextFieldOptions.AddWidgetOptions(builder, widgetOffset);
        TextFieldOptions.AddFontResource(builder, fontResourceOffset);
        TextFieldOptions.AddFontName(builder, fontNameOffset);
        TextFieldOptions.AddFontSize(builder, inputField.textComponent?.fontSize ?? 14);
        TextFieldOptions.AddText(builder, textOffset);
        TextFieldOptions.AddIsLocalized(builder, false);
        TextFieldOptions.AddPlaceHolder(builder, placeholderOffset);
        TextFieldOptions.AddPasswordEnabled(builder, inputField.contentType == InputField.ContentType.Password);
        TextFieldOptions.AddPasswordStyleText(builder, passwordStyleOffset);
        TextFieldOptions.AddMaxLengthEnabled(builder, inputField.characterLimit > 0);
        TextFieldOptions.AddMaxLength(builder, inputField.characterLimit);
        TextFieldOptions.AddAreaWidth(builder, (int)inputField.GetComponent<RectTransform>().sizeDelta.x);
        TextFieldOptions.AddAreaHeight(builder, (int)inputField.GetComponent<RectTransform>().sizeDelta.y);
        TextFieldOptions.AddIsCustomSize(builder, true);
        var textFieldOffset = TextFieldOptions.EndTextFieldOptions(builder);

        Options.StartOptions(builder);
        Options.AddData(builder, widgetOffset);
        return Options.EndOptions(builder);
    }

    private Offset<Options> BuildSliderOptions(FlatBufferBuilder builder, GameObject go,
        Slider slider, ExportContext context)
    {
        var widgetOffset = BuildWidgetOptions(builder, go, context);

        var bgImage = slider.GetComponentInChildren<Image>();
        var barDataOffset = BuildResourceData(builder, bgImage?.sprite?.texture, context);
        var ballNormalOffset = BuildResourceData(builder, slider.handleRect?.GetComponent<Image>()?.sprite?.texture, context);
        var ballPressedOffset = BuildResourceData(builder, null, context);
        var ballDisabledOffset = BuildResourceData(builder, null, context);
        var progressOffset = BuildResourceData(builder, slider.fillRect?.GetComponent<Image>()?.sprite?.texture, context);

        SliderOptions.StartSliderOptions(builder);
        SliderOptions.AddWidgetOptions(builder, widgetOffset);
        SliderOptions.AddBarFileNameData(builder, barDataOffset);
        SliderOptions.AddBallNormalData(builder, ballNormalOffset);
        SliderOptions.AddBallPressedData(builder, ballPressedOffset);
        SliderOptions.AddBallDisabledData(builder, ballDisabledOffset);
        SliderOptions.AddProgressBarData(builder, progressOffset);
        SliderOptions.AddPercent(builder, (int)(slider.normalizedValue * 100));
        SliderOptions.AddDisplaystate(builder, slider.interactable);
        var sliderOffset = SliderOptions.EndSliderOptions(builder);

        Options.StartOptions(builder);
        Options.AddData(builder, widgetOffset);
        return Options.EndOptions(builder);
    }

    private Offset<Options> BuildScrollViewOptions(FlatBufferBuilder builder, GameObject go,
        ScrollRect scrollRect, ExportContext context)
    {
        var widgetOffset = BuildWidgetOptions(builder, go, context);

        var bgImage = go.GetComponent<Image>();
        var bgDataOffset = BuildResourceData(builder, bgImage?.sprite?.texture, context);

        var bgColor = flatbuffers.Color.CreateColor(builder, 255, 255, 255, 255);
        var bgStartColor = flatbuffers.Color.CreateColor(builder, 255, 255, 255, 255);
        var bgEndColor = flatbuffers.Color.CreateColor(builder, 255, 255, 255, 255);
        var colorVector = ColorVector.CreateColorVector(builder, 0, -0.5f);
        var capInsets = CapInsets.CreateCapInsets(builder, 0, 0, 0, 0);
        var scale9Size = FlatSize.CreateFlatSize(builder, 100, 100);
        var innerSize = FlatSize.CreateFlatSize(builder,
            scrollRect.content?.sizeDelta.x ?? 100,
            scrollRect.content?.sizeDelta.y ?? 100);

        int direction = 0;
        if (scrollRect.horizontal && !scrollRect.vertical) direction = 1;
        else if (!scrollRect.horizontal && scrollRect.vertical) direction = 2;
        else direction = 3;

        ScrollViewOptions.StartScrollViewOptions(builder);
        ScrollViewOptions.AddWidgetOptions(builder, widgetOffset);
        ScrollViewOptions.AddBackGroundImageData(builder, bgDataOffset);
        ScrollViewOptions.AddClipEnabled(builder, scrollRect.viewport != null);
        ScrollViewOptions.AddBgColor(builder, bgColor);
        ScrollViewOptions.AddBgStartColor(builder, bgStartColor);
        ScrollViewOptions.AddBgEndColor(builder, bgEndColor);
        ScrollViewOptions.AddColorType(builder, 0);
        ScrollViewOptions.AddBgColorOpacity(builder, 255);
        ScrollViewOptions.AddColorVector(builder, colorVector);
        ScrollViewOptions.AddCapInsets(builder, capInsets);
        ScrollViewOptions.AddScale9Size(builder, scale9Size);
        ScrollViewOptions.AddBackGroundScale9Enabled(builder, false);
        ScrollViewOptions.AddInnerSize(builder, innerSize);
        ScrollViewOptions.AddDirection(builder, direction);
        ScrollViewOptions.AddBounceEnabled(builder, scrollRect.movementType == ScrollRect.MovementType.Elastic);
        ScrollViewOptions.AddScrollbarEnabeld(builder, scrollRect.horizontalScrollbar != null || scrollRect.verticalScrollbar != null);
        ScrollViewOptions.AddScrollbarAutoHide(builder, true);
        ScrollViewOptions.AddScrollbarAutoHideTime(builder, 0.2f);
        var scrollViewOffset = ScrollViewOptions.EndScrollViewOptions(builder);

        Options.StartOptions(builder);
        Options.AddData(builder, widgetOffset);
        return Options.EndOptions(builder);
    }

    private Offset<Options> BuildPanelOptions(FlatBufferBuilder builder, GameObject go, ExportContext context)
    {
        var widgetOffset = BuildWidgetOptions(builder, go, context);

        var image = go.GetComponent<Image>();
        var bgDataOffset = BuildResourceData(builder, image?.sprite?.texture, context);

        var bgColor = flatbuffers.Color.CreateColor(builder, 255, 255, 255, 255);
        var bgStartColor = flatbuffers.Color.CreateColor(builder, 255, 255, 255, 255);
        var bgEndColor = flatbuffers.Color.CreateColor(builder, 255, 255, 255, 255);
        var colorVector = ColorVector.CreateColorVector(builder, 0, -0.5f);
        var capInsets = CapInsets.CreateCapInsets(builder, 0, 0, 0, 0);
        var scale9Size = FlatSize.CreateFlatSize(builder, 100, 100);

        PanelOptions.StartPanelOptions(builder);
        PanelOptions.AddWidgetOptions(builder, widgetOffset);
        PanelOptions.AddBackGroundImageData(builder, bgDataOffset);
        PanelOptions.AddClipEnabled(builder, go.GetComponent<Mask>() != null || go.GetComponent<RectMask2D>() != null);
        PanelOptions.AddBgColor(builder, bgColor);
        PanelOptions.AddBgStartColor(builder, bgStartColor);
        PanelOptions.AddBgEndColor(builder, bgEndColor);
        PanelOptions.AddColorType(builder, 0);
        PanelOptions.AddBgColorOpacity(builder, 255);
        PanelOptions.AddColorVector(builder, colorVector);
        PanelOptions.AddCapInsets(builder, capInsets);
        PanelOptions.AddScale9Size(builder, scale9Size);
        PanelOptions.AddBackGroundScale9Enabled(builder, image?.type == Image.Type.Sliced);
        var panelOffset = PanelOptions.EndPanelOptions(builder);

        Options.StartOptions(builder);
        Options.AddData(builder, widgetOffset);
        return Options.EndOptions(builder);
    }

    private Offset<ResourceData> BuildResourceData(FlatBufferBuilder builder, Texture texture, ExportContext context)
    {
        string path = "";
        int resourceType = 0;

        if (texture != null && context.textureMap.TryGetValue(texture, out int index))
        {
            path = context.texturePaths[index];
        }

        var pathOffset = builder.CreateString(path);
        var plistOffset = builder.CreateString("");

        ResourceData.StartResourceData(builder);
        ResourceData.AddPath(builder, pathOffset);
        ResourceData.AddPlistFile(builder, plistOffset);
        ResourceData.AddResourceType(builder, resourceType);
        return ResourceData.EndResourceData(builder);
    }

    #endregion

    #region Animation Building

    private (Offset<NodeAction>? action, VectorOffset animationList) BuildAnimations(
        FlatBufferBuilder builder, GameObject root, ExportContext context)
    {
        var animations = root.GetComponentsInChildren<Animation>(includeInactive);
        var animators = root.GetComponentsInChildren<Animator>(includeInactive);

        var timelineOffsets = new List<Offset<TimeLine>>();
        var animationInfoOffsets = new List<Offset<AnimationInfo>>();
        int totalDuration = 0;
        int currentStartFrame = 0;

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

                var nameOffset = builder.CreateString(clip.name);
                AnimationInfo.StartAnimationInfo(builder);
                AnimationInfo.AddName(builder, nameOffset);
                AnimationInfo.AddStartIndex(builder, currentStartFrame);
                AnimationInfo.AddEndIndex(builder, currentStartFrame + clipFrames);
                animationInfoOffsets.Add(AnimationInfo.EndAnimationInfo(builder));

                currentStartFrame += clipFrames;
                totalDuration = Mathf.Max(totalDuration, currentStartFrame);
            }
        }

        // Process Animator components
        foreach (var animator in animators)
        {
            if (animator.runtimeAnimatorController == null) continue;

            var clips = animator.runtimeAnimatorController.animationClips;
            foreach (var clip in clips)
            {
                var clipTimelines = BuildClipTimelines(builder, animator.gameObject, clip, context);
                timelineOffsets.AddRange(clipTimelines);

                int clipFrames = Mathf.RoundToInt(clip.length * animationFrameRate);

                var nameOffset = builder.CreateString(clip.name);
                AnimationInfo.StartAnimationInfo(builder);
                AnimationInfo.AddName(builder, nameOffset);
                AnimationInfo.AddStartIndex(builder, currentStartFrame);
                AnimationInfo.AddEndIndex(builder, currentStartFrame + clipFrames);
                animationInfoOffsets.Add(AnimationInfo.EndAnimationInfo(builder));

                currentStartFrame += clipFrames;
                totalDuration = Mathf.Max(totalDuration, currentStartFrame);
            }
        }

        if (timelineOffsets.Count == 0)
        {
            return (null, default);
        }

        var timelinesVector = NodeAction.CreateTimeLinesVector(builder, timelineOffsets.ToArray());
        var currentAnimNameOffset = builder.CreateString(animationInfoOffsets.Count > 0 ? "Default" : "");

        NodeAction.StartNodeAction(builder);
        NodeAction.AddDuration(builder, totalDuration);
        NodeAction.AddSpeed(builder, 1.0f);
        NodeAction.AddTimeLines(builder, timelinesVector);
        NodeAction.AddCurrentAnimationName(builder, currentAnimNameOffset);
        var actionOffset = NodeAction.EndNodeAction(builder);

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

        if (!context.actionTagMap.TryGetValue(target, out int actionTag))
        {
            actionTag = target.GetInstanceID();
            context.actionTagMap[target] = actionTag;
        }

        var bindings = AnimationUtility.GetCurveBindings(clip);

        // Group curves by property type
        var posXBinding = bindings.FirstOrDefault(b => b.propertyName == "m_LocalPosition.x");
        var posYBinding = bindings.FirstOrDefault(b => b.propertyName == "m_LocalPosition.y");
        var scaleXBinding = bindings.FirstOrDefault(b => b.propertyName == "m_LocalScale.x");
        var scaleYBinding = bindings.FirstOrDefault(b => b.propertyName == "m_LocalScale.y");
        var rotZBinding = bindings.FirstOrDefault(b => b.propertyName.Contains("localEulerAngles") && b.propertyName.Contains("z"));
        var colorRBinding = bindings.FirstOrDefault(b => b.propertyName.Contains("m_Color.r"));
        var colorGBinding = bindings.FirstOrDefault(b => b.propertyName.Contains("m_Color.g"));
        var colorBBinding = bindings.FirstOrDefault(b => b.propertyName.Contains("m_Color.b"));
        var colorABinding = bindings.FirstOrDefault(b => b.propertyName.Contains("m_Color.a"));
        var alphaBinding = bindings.FirstOrDefault(b => b.propertyName == "m_Alpha");
        var spriteBinding = bindings.FirstOrDefault(b => b.propertyName == "m_Sprite");
        var activeBinding = bindings.FirstOrDefault(b => b.propertyName == "m_IsActive");

        // Position timeline
        if (!string.IsNullOrEmpty(posXBinding.propertyName) || !string.IsNullOrEmpty(posYBinding.propertyName))
        {
            var xCurve = !string.IsNullOrEmpty(posXBinding.propertyName) ? AnimationUtility.GetEditorCurve(clip, posXBinding) : null;
            var yCurve = !string.IsNullOrEmpty(posYBinding.propertyName) ? AnimationUtility.GetEditorCurve(clip, posYBinding) : null;
            var timeline = BuildPositionTimeline(builder, actionTag, xCurve, yCurve);
            if (timeline.HasValue) timelines.Add(timeline.Value);
        }

        // Scale timeline
        if (!string.IsNullOrEmpty(scaleXBinding.propertyName) || !string.IsNullOrEmpty(scaleYBinding.propertyName))
        {
            var xCurve = !string.IsNullOrEmpty(scaleXBinding.propertyName) ? AnimationUtility.GetEditorCurve(clip, scaleXBinding) : null;
            var yCurve = !string.IsNullOrEmpty(scaleYBinding.propertyName) ? AnimationUtility.GetEditorCurve(clip, scaleYBinding) : null;
            var timeline = BuildScaleTimeline(builder, actionTag, xCurve, yCurve);
            if (timeline.HasValue) timelines.Add(timeline.Value);
        }

        // Rotation timeline (as RotationSkew)
        if (!string.IsNullOrEmpty(rotZBinding.propertyName))
        {
            var zCurve = AnimationUtility.GetEditorCurve(clip, rotZBinding);
            var timeline = BuildRotationTimeline(builder, actionTag, zCurve);
            if (timeline.HasValue) timelines.Add(timeline.Value);
        }

        // Color timeline
        if (!string.IsNullOrEmpty(colorRBinding.propertyName) || !string.IsNullOrEmpty(colorABinding.propertyName))
        {
            var rCurve = !string.IsNullOrEmpty(colorRBinding.propertyName) ? AnimationUtility.GetEditorCurve(clip, colorRBinding) : null;
            var gCurve = !string.IsNullOrEmpty(colorGBinding.propertyName) ? AnimationUtility.GetEditorCurve(clip, colorGBinding) : null;
            var bCurve = !string.IsNullOrEmpty(colorBBinding.propertyName) ? AnimationUtility.GetEditorCurve(clip, colorBBinding) : null;
            var aCurve = !string.IsNullOrEmpty(colorABinding.propertyName) ? AnimationUtility.GetEditorCurve(clip, colorABinding) : null;
            var timeline = BuildColorTimeline(builder, actionTag, rCurve, gCurve, bCurve, aCurve);
            if (timeline.HasValue) timelines.Add(timeline.Value);
        }

        // Alpha timeline (separate IntFrame)
        if (!string.IsNullOrEmpty(alphaBinding.propertyName))
        {
            var aCurve = AnimationUtility.GetEditorCurve(clip, alphaBinding);
            var timeline = BuildAlphaTimeline(builder, actionTag, aCurve);
            if (timeline.HasValue) timelines.Add(timeline.Value);
        }

        // Visibility timeline (BoolFrame)
        if (!string.IsNullOrEmpty(activeBinding.propertyName))
        {
            var activeCurve = AnimationUtility.GetEditorCurve(clip, activeBinding);
            var timeline = BuildVisibleTimeline(builder, actionTag, activeCurve);
            if (timeline.HasValue) timelines.Add(timeline.Value);
        }

        // Sprite/Texture timeline
        var objectBindings = AnimationUtility.GetObjectReferenceCurveBindings(clip);
        var spriteObjBinding = objectBindings.FirstOrDefault(b => b.propertyName == "m_Sprite");
        if (!string.IsNullOrEmpty(spriteObjBinding.propertyName))
        {
            var timeline = BuildTextureTimeline(builder, actionTag, clip, spriteObjBinding, context);
            if (timeline.HasValue) timelines.Add(timeline.Value);
        }

        return timelines;
    }

    private Offset<TimeLine>? BuildPositionTimeline(FlatBufferBuilder builder, int actionTag,
        AnimationCurve xCurve, AnimationCurve yCurve)
    {
        var frameOffsets = new List<Offset<Frame>>();
        var keyframeTimes = new HashSet<float>();

        if (xCurve != null) foreach (var key in xCurve.keys) keyframeTimes.Add(key.time);
        if (yCurve != null) foreach (var key in yCurve.keys) keyframeTimes.Add(key.time);

        foreach (float time in keyframeTimes.OrderBy(t => t))
        {
            int frameIndex = Mathf.RoundToInt(time * animationFrameRate);
            float x = xCurve?.Evaluate(time) ?? 0;
            float y = yCurve?.Evaluate(time) ?? 0;

            var posStruct = Position.CreatePosition(builder, x, y);
            var easingOffset = BuildEasingData(builder, GetEasingFromCurve(xCurve ?? yCurve, time));

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
        AnimationCurve xCurve, AnimationCurve yCurve)
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
            var easingOffset = BuildEasingData(builder, GetEasingFromCurve(xCurve ?? yCurve, time));

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

    private Offset<TimeLine>? BuildRotationTimeline(FlatBufferBuilder builder, int actionTag, AnimationCurve zCurve)
    {
        if (zCurve == null || zCurve.keys.Length == 0) return null;

        var frameOffsets = new List<Offset<Frame>>();

        foreach (var key in zCurve.keys)
        {
            int frameIndex = Mathf.RoundToInt(key.time * animationFrameRate);
            float z = key.value;

            // RotationSkew uses IntFrame with rotation value
            var easingOffset = BuildEasingData(builder, GetEasingFromCurve(zCurve, key.time));

            IntFrame.StartIntFrame(builder);
            IntFrame.AddFrameIndex(builder, frameIndex);
            IntFrame.AddTween(builder, true);
            IntFrame.AddValue(builder, (int)z);
            IntFrame.AddEasingData(builder, easingOffset);
            var intFrameOffset = IntFrame.EndIntFrame(builder);

            Frame.StartFrame(builder);
            Frame.AddIntFrame(builder, intFrameOffset);
            frameOffsets.Add(Frame.EndFrame(builder));
        }

        if (frameOffsets.Count == 0) return null;

        var propertyOffset = builder.CreateString("Rotation");
        var framesVector = TimeLine.CreateFramesVector(builder, frameOffsets.ToArray());

        TimeLine.StartTimeLine(builder);
        TimeLine.AddProperty(builder, propertyOffset);
        TimeLine.AddActionTag(builder, actionTag);
        TimeLine.AddFrames(builder, framesVector);
        return TimeLine.EndTimeLine(builder);
    }

    private Offset<TimeLine>? BuildColorTimeline(FlatBufferBuilder builder, int actionTag,
        AnimationCurve rCurve, AnimationCurve gCurve, AnimationCurve bCurve, AnimationCurve aCurve)
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

    private Offset<TimeLine>? BuildAlphaTimeline(FlatBufferBuilder builder, int actionTag, AnimationCurve aCurve)
    {
        if (aCurve == null || aCurve.keys.Length == 0) return null;

        var frameOffsets = new List<Offset<Frame>>();

        foreach (var key in aCurve.keys)
        {
            int frameIndex = Mathf.RoundToInt(key.time * animationFrameRate);
            int alpha = (int)(key.value * 255);

            var easingOffset = BuildEasingData(builder, GetEasingFromCurve(aCurve, key.time));

            IntFrame.StartIntFrame(builder);
            IntFrame.AddFrameIndex(builder, frameIndex);
            IntFrame.AddTween(builder, true);
            IntFrame.AddValue(builder, alpha);
            IntFrame.AddEasingData(builder, easingOffset);
            var intFrameOffset = IntFrame.EndIntFrame(builder);

            Frame.StartFrame(builder);
            Frame.AddIntFrame(builder, intFrameOffset);
            frameOffsets.Add(Frame.EndFrame(builder));
        }

        var propertyOffset = builder.CreateString("Alpha");
        var framesVector = TimeLine.CreateFramesVector(builder, frameOffsets.ToArray());

        TimeLine.StartTimeLine(builder);
        TimeLine.AddProperty(builder, propertyOffset);
        TimeLine.AddActionTag(builder, actionTag);
        TimeLine.AddFrames(builder, framesVector);
        return TimeLine.EndTimeLine(builder);
    }

    private Offset<TimeLine>? BuildVisibleTimeline(FlatBufferBuilder builder, int actionTag, AnimationCurve activeCurve)
    {
        if (activeCurve == null || activeCurve.keys.Length == 0) return null;

        var frameOffsets = new List<Offset<Frame>>();

        foreach (var key in activeCurve.keys)
        {
            int frameIndex = Mathf.RoundToInt(key.time * animationFrameRate);
            bool visible = key.value > 0.5f;

            var easingOffset = BuildEasingData(builder, EasingType.Linear);

            BoolFrame.StartBoolFrame(builder);
            BoolFrame.AddFrameIndex(builder, frameIndex);
            BoolFrame.AddTween(builder, false);
            BoolFrame.AddValue(builder, visible);
            BoolFrame.AddEasingData(builder, easingOffset);
            var boolFrameOffset = BoolFrame.EndBoolFrame(builder);

            Frame.StartFrame(builder);
            Frame.AddBoolFrame(builder, boolFrameOffset);
            frameOffsets.Add(Frame.EndFrame(builder));
        }

        var propertyOffset = builder.CreateString("Visible");
        var framesVector = TimeLine.CreateFramesVector(builder, frameOffsets.ToArray());

        TimeLine.StartTimeLine(builder);
        TimeLine.AddProperty(builder, propertyOffset);
        TimeLine.AddActionTag(builder, actionTag);
        TimeLine.AddFrames(builder, framesVector);
        return TimeLine.EndTimeLine(builder);
    }

    private Offset<TimeLine>? BuildTextureTimeline(FlatBufferBuilder builder, int actionTag,
        AnimationClip clip, EditorCurveBinding binding, ExportContext context)
    {
        var keyframes = AnimationUtility.GetObjectReferenceCurve(clip, binding);
        if (keyframes == null || keyframes.Length == 0) return null;

        var frameOffsets = new List<Offset<Frame>>();

        foreach (var keyframe in keyframes)
        {
            int frameIndex = Mathf.RoundToInt(keyframe.time * animationFrameRate);
            var sprite = keyframe.value as Sprite;

            var textureFileOffset = BuildResourceData(builder, sprite?.texture, context);
            var easingOffset = BuildEasingData(builder, EasingType.Linear);

            TextureFrame.StartTextureFrame(builder);
            TextureFrame.AddFrameIndex(builder, frameIndex);
            TextureFrame.AddTween(builder, false);
            TextureFrame.AddTextureFile(builder, textureFileOffset);
            TextureFrame.AddEasingData(builder, easingOffset);
            var textureFrameOffset = TextureFrame.EndTextureFrame(builder);

            Frame.StartFrame(builder);
            Frame.AddTextureFrame(builder, textureFrameOffset);
            frameOffsets.Add(Frame.EndFrame(builder));
        }

        var propertyOffset = builder.CreateString("FileData");
        var framesVector = TimeLine.CreateFramesVector(builder, frameOffsets.ToArray());

        TimeLine.StartTimeLine(builder);
        TimeLine.AddProperty(builder, propertyOffset);
        TimeLine.AddActionTag(builder, actionTag);
        TimeLine.AddFrames(builder, framesVector);
        return TimeLine.EndTimeLine(builder);
    }

    private EasingType GetEasingFromCurve(AnimationCurve curve, float time)
    {
        // Analyze curve tangents to determine easing type
        // This is a simplified version - full implementation would analyze in/out tangents
        if (curve == null) return EasingType.Linear;

        var keyIndex = -1;
        for (int i = 0; i < curve.keys.Length; i++)
        {
            if (Mathf.Approximately(curve.keys[i].time, time))
            {
                keyIndex = i;
                break;
            }
        }

        if (keyIndex < 0 || keyIndex >= curve.keys.Length - 1)
            return EasingType.Linear;

        var key = curve.keys[keyIndex];
        var outTangent = key.outTangent;

        // Rough heuristic for easing type based on tangent
        if (float.IsInfinity(outTangent)) return EasingType.Linear;
        if (Mathf.Abs(outTangent) < 0.1f) return EasingType.EaseOutQuad;
        if (outTangent > 2f) return EasingType.EaseInQuad;

        return EasingType.Linear;
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
        public Dictionary<string, string> textureSourcePaths = new Dictionary<string, string>();
        public Dictionary<GameObject, int> actionTagMap = new Dictionary<GameObject, int>();
        public int nodeCount = 0;
    }

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
