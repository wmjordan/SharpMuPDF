#include "mupdf/fitz.h"
#include "mupdf/pdf.h"
#include "MuPDF.h"

#ifndef __PROCESSOR
#define __PROCESSOR

#pragma once
using namespace System;
using namespace System::Collections::Generic;

namespace MuPDF
{

public ref class Processor
{
public:
    void ProcessPageContent(Document^ doc, Page^ page);
    void ProcessContent(Document^ doc, PdfStream^ stream, PdfDictionary^ resources);

    property bool IsClosed {
        bool get() { return !_processor || _processor->closed; }
    }

protected:
    ~Processor();
    !Processor();

    // Text operators
    virtual void BeginText() {}
    virtual void EndText() {}
    virtual void ShowText(String^ text) {}
    virtual void ShowTextNewLine(String^ text) {}
    virtual void ShowTextWithPositioning(PdfObject^ array) {}
    virtual void ShowTextNewLineWithSpacing(float aw, float ac, String^ text) {}

    // Graphics state operators
    virtual void SetLineWidth(float width) {}
    virtual void SetLineJoin(int linejoin) {}
    virtual void SetLineCap(int linecap) {}
    virtual void SetMiterLimit(float limit) {}
    virtual void SetDashPattern(PdfObject^ array, float phase) {}
    virtual void SetRenderingIntent(String^ intent) {}
    virtual void SetFlatness(float flatness) {}

    // Special graphics state
    virtual void SaveState() {}
    virtual void RestoreState() {}
    virtual void SetTransform(float a, float b, float c, float d, float e, float f) {}

    // Path construction
    virtual void MoveTo(float x, float y) {}
    virtual void LineTo(float x, float y) {}
    virtual void CurveTo(float x1, float y1, float x2, float y2, float x3, float y3) {}
    virtual void CurveToV(float x2, float y2, float x3, float y3) {}
    virtual void CurveToY(float x1, float y1, float x3, float y3) {}
    virtual void ClosePath() {}
    virtual void Rectangle(float x, float y, float w, float h) {}

    // Path painting
    virtual void StrokePath() {}
    virtual void CloseAndStrokePath() {}
    virtual void FillPath() {}
    virtual void FillPathEvenOdd() {}
    virtual void FillAndStrokePath() {}
    virtual void FillAndStrokePathEvenOdd() {}
    virtual void CloseFillAndStrokePath() {}
    virtual void CloseFillAndStrokePathEvenOdd() {}
    virtual void EndPath() {}

    // Clipping paths
    virtual void ClipPath() {}
    virtual void ClipPathEvenOdd() {}

    // Text state
    virtual void SetCharSpacing(float spacing) {}
    virtual void SetWordSpacing(float spacing) {}
    virtual void SetHorizontalScaling(float scale) {}
    virtual void SetLeading(float leading) {}
    virtual void SetFont(String^ name, FontDescriptor^ font, float size) {}
    virtual void SetTextRenderMode(int mode) {}
    virtual void SetTextRise(float rise) {}

    // Text positioning
    virtual void TextMove(float tx, float ty) {}
    virtual void TextMoveSetLeading(float tx, float ty) {}
    virtual void SetTextMatrix(float a, float b, float c, float d, float e, float f) {}
    virtual void TextNewLine() {}

    // Color
    virtual void SetStrokeColorSpace(String^ name) {}
    virtual void SetFillColorSpace(String^ name) {}
    virtual void SetStrokeColor(int n, array<float>^ color) {}
    virtual void SetFillColor(int n, array<float>^ color) {}
    virtual void SetStrokeGray(float gray) {}
    virtual void SetFillGray(float gray) {}
    virtual void SetStrokeRGB(float r, float g, float b) {}
    virtual void SetFillRGB(float r, float g, float b) {}
    virtual void SetStrokeCMYK(float c, float m, float y, float k) {}
    virtual void SetFillCMYK(float c, float m, float y, float k) {}

    // Resources
    virtual void PushResources(PdfObject^ res) {}
    virtual void PopResources(PdfObject^ res) {}

    // Marked Content
    virtual void BeginMarkedContent(String^ tag) {}
    virtual void BeginMarkedContentWithProps(String^ tag, PdfObject^ raw, PdfObject^ cooked) {}
    virtual void EndMarkedContent() {}
    virtual void MarkedContentPoint(String^ tag) {}
    virtual void MarkedContentPointWithProps(String^ tag, PdfObject^ raw, PdfObject^ cooked) {}

    // Compatibilities
    virtual void BeginCompatibilitySection() {}
    virtual void EndCompatibilitySection() {}

    // XObject
    virtual void DoFormXObject(String^ name, PdfObject^ form) {}
    virtual void DoImageXObject(String^ name, Image^ image) {}

    // Painting
    virtual void PaintShading(String^ name) {}
    virtual void PaintInlineImage(Image^ image, String^ colorspaceName) {}

    // ExtGState
    virtual void BeginExtGState(String^ name, PdfObject^ extgstate) {}
    virtual void EndExtGState() {}
    virtual void SetExtGStateBlendMode(String^ blendmode) {}
    virtual void SetExtGStateStrokeAlpha(float alpha) {}
    virtual void SetExtGStateFillAlpha(float alpha) {}
    virtual void SetExtGStateSoftMask(PdfObject^ smask, Colorspace^ smask_cs, array<float>^ bc, bool luminosity, PdfObject^ tr) {}

    // Type3 Fonts
    virtual void SetType3FontSize(float wx, float wy) {}
    virtual void SetType3FontGlyph(float wx, float wy, float llx, float lly, float urx, float ury) {}

    virtual void EndOfData() {}
    virtual void End() {}

private:
    property pdf_processor* Ptr { pdf_processor* get() { return _processor; } }
    void Init();
    pdf_processor* _processor;

    // Handles grouped by functionality
    // Text operators
    GCHandle _BeginTextHandle, _EndTextHandle, _ShowTextHandle, _ShowTextNewLineHandle;
    GCHandle _ShowTextWithPositioningHandle, _ShowTextNewLineWithSpacingHandle;

    // Graphics state
    GCHandle _SetLineWidthHandle, _SetLineJoinHandle, _SetLineCapHandle;
    GCHandle _SetMiterLimitHandle, _SetDashPatternHandle, _SetRenderingIntentHandle, _SetFlatnessHandle;

    // Special graphics state
    GCHandle _SaveStateHandle, _RestoreStateHandle, _SetTransformHandle;

    // Path construction
    GCHandle _MoveToHandle, _LineToHandle, _CurveToHandle, _CurveToVHandle, _CurveToYHandle;
    GCHandle _ClosePathHandle, _RectangleHandle;

    // Path painting
    GCHandle _StrokePathHandle, _CloseAndStrokePathHandle, _FillPathHandle, _FillPathEvenOddHandle;
    GCHandle _FillAndStrokePathHandle, _FillAndStrokePathEvenOddHandle;
    GCHandle _CloseFillAndStrokePathHandle, _CloseFillAndStrokePathEvenOddHandle, _EndPathHandle;

    // Clipping paths
    GCHandle _ClipPathHandle, _ClipPathEvenOddHandle;

    // Text state
    GCHandle _SetCharSpacingHandle, _SetWordSpacingHandle, _SetHorizontalScalingHandle;
    GCHandle _SetLeadingHandle, _SetFontHandle, _SetTextRenderModeHandle, _SetTextRiseHandle;

    // Text positioning
    GCHandle _TextMoveHandle, _TextMoveSetLeadingHandle, _SetTextMatrixHandle, _TextNewLineHandle;

    // Color
    GCHandle _SetStrokeColorSpaceHandle, _SetFillColorSpaceHandle;
    GCHandle _SetStrokeColorHandle, _SetFillColorHandle;
    GCHandle _SetStrokeGrayHandle, _SetFillGrayHandle;
    GCHandle _SetStrokeRGBHandle, _SetFillRGBHandle;
    GCHandle _SetStrokeCMYKHandle, _SetFillCMYKHandle;

    // regions
    GCHandle _MPHandle, _BMCHandle, _BDCHandle, _DPHandle, _EMCHandle, _BXHandle, _EXHandle;

    // Resources
    GCHandle _PushResourcesHandle, _PopResourcesHandle;
    GCHandle _DoFormHandle, _DoImageHandle, _ShadingHandle, _BIHandle;
    GCHandle _ExtGStateBeginHandle, _ExtGStateEndHandle, _ExtGStateBMHandle, _ExtGStateCAHandle, _ExtGStatecaHandle, _ExtGStateSMaskHandle;
    GCHandle _Type3FontSizeHandle, _Type3FontGlyphHandle;

    // Special
    GCHandle _EndOfDataHandle, _EndHandle;

    List<PdfObject^>^ _ResStack = gcnew List<PdfObject^>(3);

    // Callback methods with operator comments
    // op: BT
    void BeginTextCallback(fz_context* ctx, pdf_processor* proc) {
        BeginText();
    }

    // op: ET
    void EndTextCallback(fz_context* ctx, pdf_processor* proc) {
        EndText();
    }

    // op: Tj
    void ShowTextCallback(fz_context* ctx, pdf_processor* proc, char* str, size_t len);

    // op: '
    void ShowTextNewLineCallback(fz_context* ctx, pdf_processor* proc, char* str, size_t len);

    // op: w
    void SetLineWidthCallback(fz_context* ctx, pdf_processor* proc, float linewidth) {
        SetLineWidth(linewidth);
    }

    // op: j
    void SetLineJoinCallback(fz_context* ctx, pdf_processor* proc, int linejoin) {
        SetLineJoin(linejoin);
    }

    // op: J
    void SetLineCapCallback(fz_context* ctx, pdf_processor* proc, int linecap) {
        SetLineCap(linecap);
    }

    // op: M
    void SetMiterLimitCallback(fz_context* ctx, pdf_processor* proc, float miterlimit) {
        SetMiterLimit(miterlimit);
    }

    // op: d
    void SetDashPatternCallback(fz_context* ctx, pdf_processor* proc, pdf_obj* array, float phase) {
        SetDashPattern(PdfObject::Wrap(array), phase);
    }

    // op: ri
    void SetRenderingIntentCallback(fz_context* ctx, pdf_processor* proc, const char* intent) {
        SetRenderingIntent(gcnew String(intent));
    }

    // op: i
    void SetFlatnessCallback(fz_context* ctx, pdf_processor* proc, float flatness) {
        SetFlatness(flatness);
    }

    // op: q
    void SaveStateCallback(fz_context* ctx, pdf_processor* proc) {
        SaveState();
    }

    // op: Q
    void RestoreStateCallback(fz_context* ctx, pdf_processor* proc) {
        RestoreState();
    }

    // op: cm
    void SetTransformCallback(fz_context* ctx, pdf_processor* proc, float a, float b, float c, float d, float e, float f) {
        SetTransform(a, b, c, d, e, f);
    }

    // op: m
    void MoveToCallback(fz_context* ctx, pdf_processor* proc, float x, float y) {
        MoveTo(x, y);
    }

    // op: l
    void LineToCallback(fz_context* ctx, pdf_processor* proc, float x, float y) {
        LineTo(x, y);
    }

    // op: c
    void CurveToCallback(fz_context* ctx, pdf_processor* proc, float x1, float y1, float x2, float y2, float x3, float y3) {
        CurveTo(x1, y1, x2, y2, x3, y3);
    }

    // op: v
    void CurveToVCallback(fz_context* ctx, pdf_processor* proc, float x2, float y2, float x3, float y3) {
        CurveToV(x2, y2, x3, y3);
    }

    // op: y
    void CurveToYCallback(fz_context* ctx, pdf_processor* proc, float x1, float y1, float x3, float y3) {
        CurveToY(x1, y1, x3, y3);
    }

    // op: h
    void ClosePathCallback(fz_context* ctx, pdf_processor* proc) {
        ClosePath();
    }

    // op: re
    void RectangleCallback(fz_context* ctx, pdf_processor* proc, float x, float y, float w, float h) {
        Rectangle(x, y, w, h);
    }

    // op: S
    void StrokePathCallback(fz_context* ctx, pdf_processor* proc) {
        StrokePath();
    }

    // op: s
    void CloseAndStrokePathCallback(fz_context* ctx, pdf_processor* proc) {
        CloseAndStrokePath();
    }

    // op: F
    void FillPathCallback(fz_context* ctx, pdf_processor* proc) {
        FillPath();
    }

    // op: f*
    void FillPathEvenOddCallback(fz_context* ctx, pdf_processor* proc) {
        FillPathEvenOdd();
    }

    // op: B
    void FillAndStrokePathCallback(fz_context* ctx, pdf_processor* proc) {
        FillAndStrokePath();
    }

    // op: B*
    void FillAndStrokePathEvenOddCallback(fz_context* ctx, pdf_processor* proc) {
        FillAndStrokePathEvenOdd();
    }

    // op: b
    void CloseFillAndStrokePathCallback(fz_context* ctx, pdf_processor* proc) {
        CloseFillAndStrokePath();
    }

    // op: b*
    void CloseFillAndStrokePathEvenOddCallback(fz_context* ctx, pdf_processor* proc) {
        CloseFillAndStrokePathEvenOdd();
    }

    // op: n
    void EndPathCallback(fz_context* ctx, pdf_processor* proc) {
        EndPath();
    }

    // op: W
    void ClipPathCallback(fz_context* ctx, pdf_processor* proc) {
        ClipPath();
    }

    // op: W*
    void ClipPathEvenOddCallback(fz_context* ctx, pdf_processor* proc) {
        ClipPathEvenOdd();
    }

    // op: Tc
    void SetCharSpacingCallback(fz_context* ctx, pdf_processor* proc, float charspace) {
        SetCharSpacing(charspace);
    }

    // op: Tw
    void SetWordSpacingCallback(fz_context* ctx, pdf_processor* proc, float wordspace) {
        SetWordSpacing(wordspace);
    }

    // op: Tz
    void SetHorizontalScalingCallback(fz_context* ctx, pdf_processor* proc, float scale) {
        SetHorizontalScaling(scale);
    }

    // op: TL
    void SetLeadingCallback(fz_context* ctx, pdf_processor* proc, float leading) {
        SetLeading(leading);
    }

    // op: Tf
    void SetFontCallback(fz_context* ctx, pdf_processor* proc, const char* name, pdf_font_desc* font, float size) {
        SetFont(gcnew String(name), gcnew FontDescriptor(font), size);
    }

    // op: Tr
    void SetTextRenderModeCallback(fz_context* ctx, pdf_processor* proc, int render) {
        SetTextRenderMode(render);
    }

    // op: Ts
    void SetTextRiseCallback(fz_context* ctx, pdf_processor* proc, float rise) {
        SetTextRise(rise);
    }

    // op: Td
    void TextMoveCallback(fz_context* ctx, pdf_processor* proc, float tx, float ty) {
        TextMove(tx, ty);
    }

    // op: TD
    void TextMoveSetLeadingCallback(fz_context* ctx, pdf_processor* proc, float tx, float ty) {
        TextMoveSetLeading(tx, ty);
    }

    // op: Tm
    void SetTextMatrixCallback(fz_context* ctx, pdf_processor* proc, float a, float b, float c, float d, float e, float f) {
        SetTextMatrix(a, b, c, d, e, f);
    }

    // op: T*
    void TextNewLineCallback(fz_context* ctx, pdf_processor* proc) {
        TextNewLine();
    }

    // op: TJ
    void ShowTextWithPositioningCallback(fz_context* ctx, pdf_processor* proc, pdf_obj* array) {
        ShowTextWithPositioning(PdfObject::Wrap(array));
    }

    // op: "
    void ShowTextNewLineWithSpacingCallback(fz_context* ctx, pdf_processor* proc, float aw, float ac, char* str, size_t len) {
        ShowTextNewLineWithSpacing(aw, ac, gcnew String(str, 0, len, System::Text::Encoding::UTF8));
    }

    // op: CS
    void SetStrokeColorSpaceCallback(fz_context* ctx, pdf_processor* proc, const char* name, fz_colorspace* cs) {
        SetStrokeColorSpace(gcnew String(name));
    }

    // op: cs
    void SetFillColorSpaceCallback(fz_context* ctx, pdf_processor* proc, const char* name, fz_colorspace* cs) {
        SetFillColorSpace(gcnew String(name));
    }

    // op: SC
    void SetStrokeColorCallback(fz_context* ctx, pdf_processor* proc, int n, float* color) {
        array<float>^ arr = gcnew array<float>(n);
        for (int i = 0; i < n; i++) arr[i] = color[i];
        SetStrokeColor(n, arr);
    }

    // op: sc
    void SetFillColorCallback(fz_context* ctx, pdf_processor* proc, int n, float* color) {
        array<float>^ arr = gcnew array<float>(n);
        for (int i = 0; i < n; i++) arr[i] = color[i];
        SetFillColor(n, arr);
    }

    // op: G
    void SetStrokeGrayCallback(fz_context* ctx, pdf_processor* proc, float g) {
        SetStrokeGray(g);
    }

    // op: g
    void SetFillGrayCallback(fz_context* ctx, pdf_processor* proc, float g) {
        SetFillGray(g);
    }

    // op: RG
    void SetStrokeRGBCallback(fz_context* ctx, pdf_processor* proc, float r, float g, float b) {
        SetStrokeRGB(r, g, b);
    }

    // op: rg
    void SetFillRGBCallback(fz_context* ctx, pdf_processor* proc, float r, float g, float b) {
        SetFillRGB(r, g, b);
    }

    // op: K
    void SetStrokeCMYKCallback(fz_context* ctx, pdf_processor* proc, float c, float m, float y, float k) {
        SetStrokeCMYK(c, m, y, k);
    }

    // op: k
    void SetFillCMYKCallback(fz_context* ctx, pdf_processor* proc, float c, float m, float y, float k) {
        SetFillCMYK(c, m, y, k);
    }


    // Marked Content
    void MP_Callback(fz_context* ctx, pdf_processor* proc, const char* tag) {
        MarkedContentPoint(gcnew String(tag));
    }

    void BMC_Callback(fz_context* ctx, pdf_processor* proc, const char* tag) {
        BeginMarkedContent(gcnew String(tag));
    }

    void BDC_Callback(fz_context* ctx, pdf_processor* proc, const char* tag, pdf_obj* raw, pdf_obj* cooked) {
        BeginMarkedContentWithProps(gcnew String(tag), PdfObject::Wrap(raw), PdfObject::Wrap(cooked));
    }

    void DP_Callback(fz_context* ctx, pdf_processor* proc, const char* tag, pdf_obj* raw, pdf_obj* cooked) {
        MarkedContentPointWithProps(gcnew String(tag), PdfObject::Wrap(raw), PdfObject::Wrap(cooked));
    }

    void EMC_Callback(fz_context* ctx, pdf_processor* proc) {
        EndMarkedContent();
    }

    // Compatibility
    void BX_Callback(fz_context* ctx, pdf_processor* proc) {
        BeginCompatibilitySection();
    }

    void EX_Callback(fz_context* ctx, pdf_processor* proc) {
        EndCompatibilitySection();
    }

    // XObjects
    void DoForm_Callback(fz_context* ctx, pdf_processor* proc, const char* name, pdf_obj* form) {
        DoFormXObject(gcnew String(name), PdfObject::Wrap(form));
    }

    void DoImage_Callback(fz_context* ctx, pdf_processor* proc, const char* name, fz_image* image) {
        DoImageXObject(gcnew String(name), image ? gcnew Image(image) : nullptr);
    }

    // Shading
    void Shading_Callback(fz_context* ctx, pdf_processor* proc, const char* name, fz_shade* shade) {
        PaintShading(gcnew String(name));
    }

    // Inline Image
    void BI_Callback(fz_context* ctx, pdf_processor* proc, fz_image* image, const char* colorspace_name) {
        PaintInlineImage(gcnew Image(image), gcnew String(colorspace_name));
    }

    // ExtGState
    void ExtGStateBegin_Callback(fz_context* ctx, pdf_processor* proc, const char* name, pdf_obj* extgstate) {
        BeginExtGState(gcnew String(name), PdfObject::Wrap(extgstate));
    }

    void ExtGStateEnd_Callback(fz_context* ctx, pdf_processor* proc) {
        EndExtGState();
    }

    void ExtGStateBM_Callback(fz_context* ctx, pdf_processor* proc, const char* blendmode) {
        SetExtGStateBlendMode(gcnew String(blendmode));
    }

    void ExtGStateCA_Callback(fz_context* ctx, pdf_processor* proc, float alpha) {
        SetExtGStateStrokeAlpha(alpha);
    }

    void ExtGStateca_Callback(fz_context* ctx, pdf_processor* proc, float alpha) {
        SetExtGStateFillAlpha(alpha);
    }

    void ExtGStateSMask_Callback(fz_context* ctx, pdf_processor* proc, pdf_obj* smask, fz_colorspace* smask_cs, float* bc, int luminosity, pdf_obj* tr) {
        array<float>^ bcArr = gcnew array<float>(smask_cs->n);
        Marshal::Copy(IntPtr(bc), bcArr, 0, smask_cs->n);
        SetExtGStateSoftMask(
            PdfObject::Wrap(smask),
            gcnew Colorspace(smask_cs),
            bcArr,
            luminosity != 0,
            PdfObject::Wrap(tr)
        );
    }

    // Type3 Fonts
    void Type3FontSize_Callback(fz_context* ctx, pdf_processor* proc, float wx, float wy) {
        SetType3FontSize(wx, wy);
    }

    void Type3FontGlyph_Callback(fz_context* ctx, pdf_processor* proc, float wx, float wy, float llx, float lly, float urx, float ury) {
        SetType3FontGlyph(wx, wy, llx, lly, urx, ury);
    }

    // op: EOD
    void EndOfDataCallback(fz_context* ctx, pdf_processor* proc) {
        EndOfData();
    }

    void EndCallback(fz_context* ctx, pdf_processor* proc) {
        End();
    }
    void PushResourcesCallback(fz_context* ctx, pdf_processor* proc, pdf_obj* obj) {
        auto r = PdfObject::Wrap(obj);
        _ResStack->Add(r);
        PushResources(r);
    }
    pdf_obj* PopResourcesCallback(fz_context* ctx, pdf_processor* proc) {
        pdf_obj* r = NULL;
        if (_ResStack->Count != 0) {
            r = _ResStack[_ResStack->Count - 1]->Ptr;
        }
        PopResources(PdfObject::Wrap(r));
        return r;
    }
    // ... other callbacks  
};

}

#endif