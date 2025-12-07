#include "Processor.h"
#include "MuException.h"

#pragma unmanaged

static bool
RunProcessorOnContent(fz_context* ctx, pdf_processor* proc, pdf_document* doc, pdf_obj* stream, pdf_obj* resources, fz_cookie* cookie) {
	bool r;
	fz_try(ctx) {
		pdf_process_contents(ctx, proc, doc, resources, stream, cookie, NULL);
		pdf_close_processor(ctx, proc);
		r = true;
	}
	fz_always(ctx) {
		pdf_drop_processor(ctx, proc);
	}
	fz_catch(ctx) {
		r = false;
	}
	return r;
}

static bool
RunProcessorOnPageContent(fz_context* ctx, pdf_processor* proc, pdf_document* doc, pdf_page* page, fz_cookie* cookie) {
	pdf_obj* resources = NULL;
	pdf_obj* contents = NULL;

	if (cookie && page->super.incomplete)
		cookie->incomplete = 1;

	bool r;
	fz_try(ctx) {
		resources = pdf_page_resources(ctx, page);
		contents = pdf_page_contents(ctx, page);
	}
	fz_catch(ctx) {
        pdf_drop_processor(ctx, proc);
		r = false;
	}

    return r && RunProcessorOnContent(ctx, proc, doc, contents, resources, cookie);
}

static void close_callback(fz_context* ctx, pdf_processor* proc) {
}

static void drop_callback(fz_context* ctx, pdf_processor* proc) {
}

#pragma managed
using namespace MuPDF;
using namespace System::Runtime::InteropServices;

// Delegate definitions
typedef void (ProcessorCallbackHandler)(fz_context* ctx, pdf_processor* proc);
[UnmanagedFunctionPointer(CallingConvention::Cdecl)]
delegate void ProcessorCallbackDelegate(fz_context* ctx, pdf_processor* proc);

// Callback with object parameter
typedef void (ProcessorObjCallbackHandler)(fz_context* ctx, pdf_processor* proc, pdf_obj* obj);
[UnmanagedFunctionPointer(CallingConvention::Cdecl)]
delegate void ProcessorObjCallbackDelegate(fz_context* ctx, pdf_processor* proc, pdf_obj* obj);

// Callback with returned object
typedef pdf_obj* (ProcessorRetObjCallbackHandler)(fz_context* ctx, pdf_processor* proc);
[UnmanagedFunctionPointer(CallingConvention::Cdecl)]
delegate pdf_obj* ProcessorRetObjCallbackDelegate(fz_context* ctx, pdf_processor* proc);

// Text callback
typedef void (ProcessorTextCallbackHandler)(fz_context* ctx, pdf_processor* proc, char* text, size_t len);
[UnmanagedFunctionPointer(CallingConvention::Cdecl)]
delegate void ProcessorTextCallbackDelegate(fz_context* ctx, pdf_processor* proc, char* text, size_t len);

// Float parameter callback
typedef void (ProcessorFloatCallbackHandler)(fz_context* ctx, pdf_processor* proc, float value);
[UnmanagedFunctionPointer(CallingConvention::Cdecl)]
delegate void ProcessorFloatCallbackDelegate(fz_context* ctx, pdf_processor* proc, float value);

// Integer parameter callback
typedef void (ProcessorIntCallbackHandler)(fz_context* ctx, pdf_processor* proc, int value);
[UnmanagedFunctionPointer(CallingConvention::Cdecl)]
delegate void ProcessorIntCallbackDelegate(fz_context* ctx, pdf_processor* proc, int value);

// Point coordinate callback
typedef void (ProcessorPointCallbackHandler)(fz_context* ctx, pdf_processor* proc, float x, float y);
[UnmanagedFunctionPointer(CallingConvention::Cdecl)]
delegate void ProcessorPointCallbackDelegate(fz_context* ctx, pdf_processor* proc, float x, float y);

// Cubic B¨¦zier curve callback
typedef void (ProcessorCurveCallbackHandler)(fz_context* ctx, pdf_processor* proc, float x1, float y1, float x2, float y2, float x3, float y3);
[UnmanagedFunctionPointer(CallingConvention::Cdecl)]
delegate void ProcessorCurveCallbackDelegate(fz_context* ctx, pdf_processor* proc, float x1, float y1, float x2, float y2, float x3, float y3);

// Quadratic B¨¦zier curve callback (V operator)
typedef void (ProcessorCurveVCallbackHandler)(fz_context* ctx, pdf_processor* proc, float x2, float y2, float x3, float y3);
[UnmanagedFunctionPointer(CallingConvention::Cdecl)]
delegate void ProcessorCurveVCallbackDelegate(fz_context* ctx, pdf_processor* proc, float x2, float y2, float x3, float y3);

// Quadratic B¨¦zier curve callback (Y operator)
typedef void (ProcessorCurveYCallbackHandler)(fz_context* ctx, pdf_processor* proc, float x1, float y1, float x3, float y3);
[UnmanagedFunctionPointer(CallingConvention::Cdecl)]
delegate void ProcessorCurveYCallbackDelegate(fz_context* ctx, pdf_processor* proc, float x1, float y1, float x3, float y3);

// Matrix transformation callback
typedef void (ProcessorMatrixCallbackHandler)(fz_context* ctx, pdf_processor* proc, float a, float b, float c, float d, float e, float f);
[UnmanagedFunctionPointer(CallingConvention::Cdecl)]
delegate void ProcessorMatrixCallbackDelegate(fz_context* ctx, pdf_processor* proc, float a, float b, float c, float d, float e, float f);

// String parameter callback
typedef void (ProcessorStringCallbackHandler)(fz_context* ctx, pdf_processor* proc, const char* str);
[UnmanagedFunctionPointer(CallingConvention::Cdecl)]
delegate void ProcessorStringCallbackDelegate(fz_context* ctx, pdf_processor* proc, const char* str);

// Color parameter callback
typedef void (ProcessorColorCallbackHandler)(fz_context* ctx, pdf_processor* proc, int n, float* color);
[UnmanagedFunctionPointer(CallingConvention::Cdecl)]
delegate void ProcessorColorCallbackDelegate(fz_context* ctx, pdf_processor* proc, int n, float* color);

// Object + float parameter callback
typedef void (ProcessorObjFloatCallbackHandler)(fz_context* ctx, pdf_processor* proc, pdf_obj* obj, float value);
[UnmanagedFunctionPointer(CallingConvention::Cdecl)]
delegate void ProcessorObjFloatCallbackDelegate(fz_context* ctx, pdf_processor* proc, pdf_obj* obj, float value);

// String + font + size callback
typedef void (ProcessorStringFontCallbackHandler)(fz_context* ctx, pdf_processor* proc, const char* name, pdf_font_desc* font, float size);
[UnmanagedFunctionPointer(CallingConvention::Cdecl)]
delegate void ProcessorStringFontCallbackDelegate(fz_context* ctx, pdf_processor* proc, const char* name, pdf_font_desc* font, float size);

// String + colorspace callback
typedef void (ProcessorStringColorCallbackHandler)(fz_context* ctx, pdf_processor* proc, const char* name, fz_colorspace* cs);
[UnmanagedFunctionPointer(CallingConvention::Cdecl)]
delegate void ProcessorStringColorCallbackDelegate(fz_context* ctx, pdf_processor* proc, const char* name, fz_colorspace* cs);

// Rectangle callback
typedef void (ProcessorRectCallbackHandler)(fz_context* ctx, pdf_processor* proc, float x, float y, float w, float h);
[UnmanagedFunctionPointer(CallingConvention::Cdecl)]
delegate void ProcessorRectCallbackDelegate(fz_context* ctx, pdf_processor* proc, float x, float y, float w, float h);

// RGB color callback
typedef void (ProcessorRGBCallbackHandler)(fz_context* ctx, pdf_processor* proc, float r, float g, float b);
[UnmanagedFunctionPointer(CallingConvention::Cdecl)]
delegate void ProcessorRGBCallbackDelegate(fz_context* ctx, pdf_processor* proc, float r, float g, float b);

// CMYK color callback
typedef void (ProcessorCMYKCallbackHandler)(fz_context* ctx, pdf_processor* proc, float c, float m, float y, float k);
[UnmanagedFunctionPointer(CallingConvention::Cdecl)]
delegate void ProcessorCMYKCallbackDelegate(fz_context* ctx, pdf_processor* proc, float c, float m, float y, float k);

// Double float + text callback
typedef void (ProcessorFloatFloatTextCallbackHandler)(fz_context* ctx, pdf_processor* proc, float aw, float ac, char* str, size_t len);
[UnmanagedFunctionPointer(CallingConvention::Cdecl)]
delegate void ProcessorFloatFloatTextCallbackDelegate(fz_context* ctx, pdf_processor* proc, float aw, float ac, char* str, size_t len);

// Resource object callback
typedef void (ProcessorResourceCallbackHandler)(fz_context* ctx, pdf_processor* proc, pdf_obj* res);
[UnmanagedFunctionPointer(CallingConvention::Cdecl)]
delegate void ProcessorResourceCallbackDelegate(fz_context* ctx, pdf_processor* proc, pdf_obj* res);

// op: MP, BMC (same as ProcessorStringCallbackDelegate)
// op: BDC, DP
typedef void (ProcessorTagWithPropertiesCallbackHandler)(fz_context* ctx, pdf_processor* proc, const char* tag, pdf_obj* raw, pdf_obj* cooked);
[UnmanagedFunctionPointer(CallingConvention::Cdecl)]
delegate void ProcessorTagWithPropertiesCallbackDelegate(fz_context* ctx, pdf_processor* proc, const char* tag, pdf_obj* raw, pdf_obj* cooked);

// op: EMC, BX, EX (same as ProcessorCallbackDelegate)
// op: Do (Form)
typedef void (ProcessorFormXObjectCallbackHandler)(fz_context* ctx, pdf_processor* proc, const char* name, pdf_obj* form);
[UnmanagedFunctionPointer(CallingConvention::Cdecl)]
delegate void ProcessorFormXObjectCallbackDelegate(fz_context* ctx, pdf_processor* proc, const char* name, pdf_obj* form);

// op: Do (Image)
typedef void (ProcessorImageXObjectCallbackHandler)(fz_context* ctx, pdf_processor* proc, const char* name, fz_image* image);
[UnmanagedFunctionPointer(CallingConvention::Cdecl)]
delegate void ProcessorImageXObjectCallbackDelegate(fz_context* ctx, pdf_processor* proc, const char* name, fz_image* image);

// op: sh
typedef void (ProcessorShadingCallbackHandler)(fz_context* ctx, pdf_processor* proc, const char* name, fz_shade* shade);
[UnmanagedFunctionPointer(CallingConvention::Cdecl)]
delegate void ProcessorShadingCallbackDelegate(fz_context* ctx, pdf_processor* proc, const char* name, fz_shade* shade);

// op: ext gs operations
typedef void (ProcessorExtGStateSMaskHandler)(fz_context* ctx, pdf_processor* proc, pdf_obj* smask, fz_colorspace* smask_cs, float* bc, int luminosity, pdf_obj* tr);
[UnmanagedFunctionPointer(CallingConvention::Cdecl)]
delegate void ProcessorExtGStateSMaskDelegate(fz_context* ctx, pdf_processor* proc, pdf_obj* smask, fz_colorspace* smask_cs, float* bc, int luminosity, pdf_obj* tr);

// op: BI
typedef void (ProcessorInlineImageCallbackHandler)(fz_context* ctx, pdf_processor* proc, fz_image* image, const char* colorspace_name);
[UnmanagedFunctionPointer(CallingConvention::Cdecl)]
delegate void ProcessorInlineImageCallbackDelegate(fz_context* ctx, pdf_processor* proc, fz_image* image, const char* colorspace_name);

// op: d0 (same as ProcessorPointCallbackDelegate)
// op: d1
typedef void (ProcessorType3FontGlyphHandler)(fz_context* ctx, pdf_processor* proc, float wx, float wy, float llx, float lly, float urx, float ury);
[UnmanagedFunctionPointer(CallingConvention::Cdecl)]
delegate void ProcessorType3FontGlyphDelegate(fz_context* ctx, pdf_processor* proc, float wx, float wy, float llx, float lly, float urx, float ury);

void Processor::ShowTextCallback(fz_context* ctx, pdf_processor* proc, char* str, size_t len) {
    ShowText(gcnew String(str));
}

void Processor::ShowTextNewLineCallback(fz_context* ctx, pdf_processor* proc, char* str, size_t len) {
	ShowText(gcnew String(str));
}

void Processor::Init() {
    _ResStack->Clear();
    if (_processor) {
        return;
    }

    auto proc = (pdf_processor*)pdf_new_processor(Context::Ptr, sizeof(pdf_processor));

    // text
    InstanceMethodToFunctionPointer(this, Processor::BeginTextCallback, ProcessorCallbackDelegate, ProcessorCallbackHandler, _BeginTextHandle, proc->op_BT);
    InstanceMethodToFunctionPointer(this, Processor::EndTextCallback, ProcessorCallbackDelegate, ProcessorCallbackHandler, _EndTextHandle, proc->op_ET);
    InstanceMethodToFunctionPointer(this, Processor::ShowTextCallback, ProcessorTextCallbackDelegate, ProcessorTextCallbackHandler, _ShowTextHandle, proc->op_Tj);
    InstanceMethodToFunctionPointer(this, Processor::ShowTextNewLineCallback, ProcessorTextCallbackDelegate, ProcessorTextCallbackHandler, _ShowTextNewLineHandle, proc->op_squote);
    InstanceMethodToFunctionPointer(this, Processor::ShowTextWithPositioningCallback, ProcessorObjCallbackDelegate, ProcessorObjCallbackHandler, _ShowTextWithPositioningHandle, proc->op_TJ);
    InstanceMethodToFunctionPointer(this, Processor::ShowTextNewLineWithSpacingCallback, ProcessorFloatFloatTextCallbackDelegate, ProcessorFloatFloatTextCallbackHandler, _ShowTextNewLineWithSpacingHandle, proc->op_dquote);

    // graphics status
    InstanceMethodToFunctionPointer(this, Processor::SetLineWidthCallback, ProcessorFloatCallbackDelegate, ProcessorFloatCallbackHandler, _SetLineWidthHandle, proc->op_w);
    InstanceMethodToFunctionPointer(this, Processor::SetLineJoinCallback, ProcessorIntCallbackDelegate, ProcessorIntCallbackHandler, _SetLineJoinHandle, proc->op_j);
    InstanceMethodToFunctionPointer(this, Processor::SetLineCapCallback, ProcessorIntCallbackDelegate, ProcessorIntCallbackHandler, _SetLineCapHandle, proc->op_J);
    InstanceMethodToFunctionPointer(this, Processor::SetMiterLimitCallback, ProcessorFloatCallbackDelegate, ProcessorFloatCallbackHandler, _SetMiterLimitHandle, proc->op_M);
    InstanceMethodToFunctionPointer(this, Processor::SetDashPatternCallback, ProcessorObjFloatCallbackDelegate, ProcessorObjFloatCallbackHandler, _SetDashPatternHandle, proc->op_d);
    InstanceMethodToFunctionPointer(this, Processor::SetRenderingIntentCallback, ProcessorStringCallbackDelegate, ProcessorStringCallbackHandler, _SetRenderingIntentHandle, proc->op_ri);
    InstanceMethodToFunctionPointer(this, Processor::SetFlatnessCallback, ProcessorFloatCallbackDelegate, ProcessorFloatCallbackHandler, _SetFlatnessHandle, proc->op_i);

    // special graphics
    InstanceMethodToFunctionPointer(this, Processor::SaveStateCallback, ProcessorCallbackDelegate, ProcessorCallbackHandler, _SaveStateHandle, proc->op_q);
    InstanceMethodToFunctionPointer(this, Processor::RestoreStateCallback, ProcessorCallbackDelegate, ProcessorCallbackHandler, _RestoreStateHandle, proc->op_Q);
    InstanceMethodToFunctionPointer(this, Processor::SetTransformCallback, ProcessorMatrixCallbackDelegate, ProcessorMatrixCallbackHandler, _SetTransformHandle, proc->op_cm);

    // path construction
    InstanceMethodToFunctionPointer(this, Processor::MoveToCallback, ProcessorPointCallbackDelegate, ProcessorPointCallbackHandler, _MoveToHandle, proc->op_m);
    InstanceMethodToFunctionPointer(this, Processor::LineToCallback, ProcessorPointCallbackDelegate, ProcessorPointCallbackHandler, _LineToHandle, proc->op_l);
    InstanceMethodToFunctionPointer(this, Processor::CurveToCallback, ProcessorCurveCallbackDelegate, ProcessorCurveCallbackHandler, _CurveToHandle, proc->op_c);
    InstanceMethodToFunctionPointer(this, Processor::CurveToVCallback, ProcessorCurveVCallbackDelegate, ProcessorCurveVCallbackHandler, _CurveToVHandle, proc->op_v);
    InstanceMethodToFunctionPointer(this, Processor::CurveToYCallback, ProcessorCurveYCallbackDelegate, ProcessorCurveYCallbackHandler, _CurveToYHandle, proc->op_y);
    InstanceMethodToFunctionPointer(this, Processor::ClosePathCallback, ProcessorCallbackDelegate, ProcessorCallbackHandler, _ClosePathHandle, proc->op_h);
    InstanceMethodToFunctionPointer(this, Processor::RectangleCallback, ProcessorRectCallbackDelegate, ProcessorRectCallbackHandler, _RectangleHandle, proc->op_re);

    // path painting
    InstanceMethodToFunctionPointer(this, Processor::StrokePathCallback, ProcessorCallbackDelegate, ProcessorCallbackHandler, _StrokePathHandle, proc->op_S);
    InstanceMethodToFunctionPointer(this, Processor::CloseAndStrokePathCallback, ProcessorCallbackDelegate, ProcessorCallbackHandler, _CloseAndStrokePathHandle, proc->op_s);
    InstanceMethodToFunctionPointer(this, Processor::FillPathCallback, ProcessorCallbackDelegate, ProcessorCallbackHandler, _FillPathHandle, proc->op_F);
    InstanceMethodToFunctionPointer(this, Processor::FillPathEvenOddCallback, ProcessorCallbackDelegate, ProcessorCallbackHandler, _FillPathEvenOddHandle, proc->op_fstar);
    InstanceMethodToFunctionPointer(this, Processor::FillAndStrokePathCallback, ProcessorCallbackDelegate, ProcessorCallbackHandler, _FillAndStrokePathHandle, proc->op_B);
    InstanceMethodToFunctionPointer(this, Processor::FillAndStrokePathEvenOddCallback, ProcessorCallbackDelegate, ProcessorCallbackHandler, _FillAndStrokePathEvenOddHandle, proc->op_Bstar);
    InstanceMethodToFunctionPointer(this, Processor::CloseFillAndStrokePathCallback, ProcessorCallbackDelegate, ProcessorCallbackHandler, _CloseFillAndStrokePathHandle, proc->op_b);
    InstanceMethodToFunctionPointer(this, Processor::CloseFillAndStrokePathEvenOddCallback, ProcessorCallbackDelegate, ProcessorCallbackHandler, _CloseFillAndStrokePathEvenOddHandle, proc->op_bstar);
    InstanceMethodToFunctionPointer(this, Processor::EndPathCallback, ProcessorCallbackDelegate, ProcessorCallbackHandler, _EndPathHandle, proc->op_n);

    // path clipping
    InstanceMethodToFunctionPointer(this, Processor::ClipPathCallback, ProcessorCallbackDelegate, ProcessorCallbackHandler, _ClipPathHandle, proc->op_W);
    InstanceMethodToFunctionPointer(this, Processor::ClipPathEvenOddCallback, ProcessorCallbackDelegate, ProcessorCallbackHandler, _ClipPathEvenOddHandle, proc->op_Wstar);

    // text status
    InstanceMethodToFunctionPointer(this, Processor::SetCharSpacingCallback, ProcessorFloatCallbackDelegate, ProcessorFloatCallbackHandler, _SetCharSpacingHandle, proc->op_Tc);
    InstanceMethodToFunctionPointer(this, Processor::SetWordSpacingCallback, ProcessorFloatCallbackDelegate, ProcessorFloatCallbackHandler, _SetWordSpacingHandle, proc->op_Tw);
    InstanceMethodToFunctionPointer(this, Processor::SetHorizontalScalingCallback, ProcessorFloatCallbackDelegate, ProcessorFloatCallbackHandler, _SetHorizontalScalingHandle, proc->op_Tz);
    InstanceMethodToFunctionPointer(this, Processor::SetLeadingCallback, ProcessorFloatCallbackDelegate, ProcessorFloatCallbackHandler, _SetLeadingHandle, proc->op_TL);
    InstanceMethodToFunctionPointer(this, Processor::SetFontCallback, ProcessorStringFontCallbackDelegate, ProcessorStringFontCallbackHandler, _SetFontHandle, proc->op_Tf);
    InstanceMethodToFunctionPointer(this, Processor::SetTextRenderModeCallback, ProcessorIntCallbackDelegate, ProcessorIntCallbackHandler, _SetTextRenderModeHandle, proc->op_Tr);
    InstanceMethodToFunctionPointer(this, Processor::SetTextRiseCallback, ProcessorFloatCallbackDelegate, ProcessorFloatCallbackHandler, _SetTextRiseHandle, proc->op_Ts);

    // text positioning
    InstanceMethodToFunctionPointer(this, Processor::TextMoveCallback, ProcessorPointCallbackDelegate, ProcessorPointCallbackHandler, _TextMoveHandle, proc->op_Td);
    InstanceMethodToFunctionPointer(this, Processor::TextMoveSetLeadingCallback, ProcessorPointCallbackDelegate, ProcessorPointCallbackHandler, _TextMoveSetLeadingHandle, proc->op_TD);
    InstanceMethodToFunctionPointer(this, Processor::SetTextMatrixCallback, ProcessorMatrixCallbackDelegate, ProcessorMatrixCallbackHandler, _SetTextMatrixHandle, proc->op_Tm);
    InstanceMethodToFunctionPointer(this, Processor::TextNewLineCallback, ProcessorCallbackDelegate, ProcessorCallbackHandler, _TextNewLineHandle, proc->op_Tstar);

    // color
    InstanceMethodToFunctionPointer(this, Processor::SetStrokeColorSpaceCallback, ProcessorStringColorCallbackDelegate, ProcessorStringColorCallbackHandler, _SetStrokeColorSpaceHandle, proc->op_CS);
    InstanceMethodToFunctionPointer(this, Processor::SetFillColorSpaceCallback, ProcessorStringColorCallbackDelegate, ProcessorStringColorCallbackHandler, _SetFillColorSpaceHandle, proc->op_cs);
    InstanceMethodToFunctionPointer(this, Processor::SetStrokeColorCallback, ProcessorColorCallbackDelegate, ProcessorColorCallbackHandler, _SetStrokeColorHandle, proc->op_SC_color);
    InstanceMethodToFunctionPointer(this, Processor::SetFillColorCallback, ProcessorColorCallbackDelegate, ProcessorColorCallbackHandler, _SetFillColorHandle, proc->op_sc_color);
    InstanceMethodToFunctionPointer(this, Processor::SetStrokeGrayCallback, ProcessorFloatCallbackDelegate, ProcessorFloatCallbackHandler, _SetStrokeGrayHandle, proc->op_G);
    InstanceMethodToFunctionPointer(this, Processor::SetFillGrayCallback, ProcessorFloatCallbackDelegate, ProcessorFloatCallbackHandler, _SetFillGrayHandle, proc->op_g);
    InstanceMethodToFunctionPointer(this, Processor::SetStrokeRGBCallback, ProcessorRGBCallbackDelegate, ProcessorRGBCallbackHandler, _SetStrokeRGBHandle, proc->op_RG);
    InstanceMethodToFunctionPointer(this, Processor::SetFillRGBCallback, ProcessorRGBCallbackDelegate, ProcessorRGBCallbackHandler, _SetFillRGBHandle, proc->op_rg);
    InstanceMethodToFunctionPointer(this, Processor::SetStrokeCMYKCallback, ProcessorCMYKCallbackDelegate, ProcessorCMYKCallbackHandler, _SetStrokeCMYKHandle, proc->op_K);
    InstanceMethodToFunctionPointer(this, Processor::SetFillCMYKCallback, ProcessorCMYKCallbackDelegate, ProcessorCMYKCallbackHandler, _SetFillCMYKHandle, proc->op_k);

    // Marked Content
    InstanceMethodToFunctionPointer(this, Processor::MP_Callback, ProcessorStringCallbackDelegate, ProcessorStringCallbackHandler, _MPHandle, proc->op_MP);
    InstanceMethodToFunctionPointer(this, Processor::BMC_Callback, ProcessorStringCallbackDelegate, ProcessorStringCallbackHandler, _BMCHandle, proc->op_BMC);
    InstanceMethodToFunctionPointer(this, Processor::BDC_Callback, ProcessorTagWithPropertiesCallbackDelegate, ProcessorTagWithPropertiesCallbackHandler, _BDCHandle, proc->op_BDC);
    InstanceMethodToFunctionPointer(this, Processor::DP_Callback, ProcessorTagWithPropertiesCallbackDelegate, ProcessorTagWithPropertiesCallbackHandler, _DPHandle, proc->op_DP);
    InstanceMethodToFunctionPointer(this, Processor::EMC_Callback, ProcessorCallbackDelegate, ProcessorCallbackHandler, _EMCHandle, proc->op_EMC);

    // Compatibility
    InstanceMethodToFunctionPointer(this, Processor::BX_Callback, ProcessorCallbackDelegate, ProcessorCallbackHandler, _BXHandle, proc->op_BX);
    InstanceMethodToFunctionPointer(this, Processor::EX_Callback, ProcessorCallbackDelegate, ProcessorCallbackHandler, _EXHandle, proc->op_EX);

    // XObjects
    InstanceMethodToFunctionPointer(this, Processor::DoForm_Callback, ProcessorFormXObjectCallbackDelegate, ProcessorFormXObjectCallbackHandler, _DoFormHandle, proc->op_Do_form);
    InstanceMethodToFunctionPointer(this, Processor::DoImage_Callback, ProcessorImageXObjectCallbackDelegate, ProcessorImageXObjectCallbackHandler, _DoImageHandle, proc->op_Do_image);

    // Shading & Images
    InstanceMethodToFunctionPointer(this, Processor::Shading_Callback, ProcessorShadingCallbackDelegate, ProcessorShadingCallbackHandler, _ShadingHandle, proc->op_sh);
    InstanceMethodToFunctionPointer(this, Processor::BI_Callback, ProcessorInlineImageCallbackDelegate, ProcessorInlineImageCallbackHandler, _BIHandle, proc->op_BI);

    // ExtGState
    InstanceMethodToFunctionPointer(this, Processor::ExtGStateBegin_Callback, ProcessorFormXObjectCallbackDelegate, ProcessorFormXObjectCallbackHandler, _ExtGStateBeginHandle, proc->op_gs_begin);
    InstanceMethodToFunctionPointer(this, Processor::ExtGStateEnd_Callback, ProcessorCallbackDelegate, ProcessorCallbackHandler, _ExtGStateEndHandle, proc->op_gs_end);
    InstanceMethodToFunctionPointer(this, Processor::ExtGStateBM_Callback, ProcessorStringCallbackDelegate, ProcessorStringCallbackHandler, _ExtGStateBMHandle, proc->op_gs_BM);
    InstanceMethodToFunctionPointer(this, Processor::ExtGStateCA_Callback, ProcessorFloatCallbackDelegate, ProcessorFloatCallbackHandler, _ExtGStateCAHandle, proc->op_gs_CA);
    InstanceMethodToFunctionPointer(this, Processor::ExtGStateca_Callback, ProcessorFloatCallbackDelegate, ProcessorFloatCallbackHandler, _ExtGStatecaHandle, proc->op_gs_ca);
    InstanceMethodToFunctionPointer(this, Processor::ExtGStateSMask_Callback, ProcessorExtGStateSMaskDelegate, ProcessorExtGStateSMaskHandler, _ExtGStateSMaskHandle, proc->op_gs_SMask);

    // Type3 Fonts
    InstanceMethodToFunctionPointer(this, Processor::Type3FontSize_Callback, ProcessorPointCallbackDelegate, ProcessorPointCallbackHandler, _Type3FontSizeHandle, proc->op_d0);
    InstanceMethodToFunctionPointer(this, Processor::Type3FontGlyph_Callback, ProcessorType3FontGlyphDelegate, ProcessorType3FontGlyphHandler, _Type3FontGlyphHandle, proc->op_d1);
    // special op
    InstanceMethodToFunctionPointer(this, Processor::EndOfDataCallback, ProcessorCallbackDelegate, ProcessorCallbackHandler, _EndOfDataHandle, proc->op_EOD);
    InstanceMethodToFunctionPointer(this, Processor::EndCallback, ProcessorCallbackDelegate, ProcessorCallbackHandler, _EndHandle, proc->op_END);

    // resources
    InstanceMethodToFunctionPointer(this, Processor::PushResourcesCallback, ProcessorResourceCallbackDelegate, ProcessorResourceCallbackHandler, _PushResourcesHandle, proc->push_resources);
    InstanceMethodToFunctionPointer(this, Processor::PopResourcesCallback, ProcessorRetObjCallbackDelegate, ProcessorRetObjCallbackHandler, _PopResourcesHandle, proc->pop_resources);

    proc->close_processor = close_callback;
    proc->drop_processor = drop_callback;
    _processor = proc;
}

void Processor::ProcessPageContent(Document^ doc, Page^ page) {
	Init();
	if (!RunProcessorOnPageContent(Context::Ptr, _processor, doc->Ptr, page->Ptr, NULL)) {
		throw MuException::FromContext();
	}
}

void MuPDF::Processor::ProcessContent(Document^ doc, PdfStream^ stream, PdfDictionary^ resources) {
    Init();
    if (!RunProcessorOnContent(Context::Ptr, _processor, doc->Ptr, stream->Ptr, resources->Ptr, NULL)) {
        throw MuException::FromContext();
    }
}

Processor::~Processor() {
	this->!Processor();
}

Processor::!Processor() {
	if (_processor) {
		pdf_close_processor(Context::Ptr, _processor);
		pdf_drop_processor(Context::Ptr, _processor);
		_processor = nullptr;
	}

    // release ÎÄ±¾ operation related GCHandle
    FreeHandle(_BeginTextHandle);
    FreeHandle(_EndTextHandle);
    FreeHandle(_ShowTextHandle);
    FreeHandle(_ShowTextNewLineHandle);
    FreeHandle(_ShowTextWithPositioningHandle);
    FreeHandle(_ShowTextNewLineWithSpacingHandle);

    // release graphics related GCHandle
    FreeHandle(_SetLineWidthHandle);
    FreeHandle(_SetLineJoinHandle);
    FreeHandle(_SetLineCapHandle);
    FreeHandle(_SetMiterLimitHandle);
    FreeHandle(_SetDashPatternHandle);
    FreeHandle(_SetRenderingIntentHandle);
    FreeHandle(_SetFlatnessHandle);

    // release special graphics related GCHandle
    FreeHandle(_SaveStateHandle);
    FreeHandle(_RestoreStateHandle);
    FreeHandle(_SetTransformHandle);

    // release path construction related GCHandle
    FreeHandle(_MoveToHandle);
    FreeHandle(_LineToHandle);
    FreeHandle(_CurveToHandle);
    FreeHandle(_CurveToVHandle);
    FreeHandle(_CurveToYHandle);
    FreeHandle(_ClosePathHandle);
    FreeHandle(_RectangleHandle);

    // release path painting related GCHandle
    FreeHandle(_StrokePathHandle);
    FreeHandle(_CloseAndStrokePathHandle);
    FreeHandle(_FillPathHandle);
    FreeHandle(_FillPathEvenOddHandle);
    FreeHandle(_FillAndStrokePathHandle);
    FreeHandle(_FillAndStrokePathEvenOddHandle);
    FreeHandle(_CloseFillAndStrokePathHandle);
    FreeHandle(_CloseFillAndStrokePathEvenOddHandle);
    FreeHandle(_EndPathHandle);

    // release path clipping related GCHandle
    FreeHandle(_ClipPathHandle);
    FreeHandle(_ClipPathEvenOddHandle);

    // release text status related GCHandle
    FreeHandle(_SetCharSpacingHandle);
    FreeHandle(_SetWordSpacingHandle);
    FreeHandle(_SetHorizontalScalingHandle);
    FreeHandle(_SetLeadingHandle);
    FreeHandle(_SetFontHandle);
    FreeHandle(_SetTextRenderModeHandle);
    FreeHandle(_SetTextRiseHandle);

    // release text positioning related GCHandle
    FreeHandle(_TextMoveHandle);
    FreeHandle(_TextMoveSetLeadingHandle);
    FreeHandle(_SetTextMatrixHandle);
    FreeHandle(_TextNewLineHandle);

    // release color operation related GCHandle
    FreeHandle(_SetStrokeColorSpaceHandle);
    FreeHandle(_SetFillColorSpaceHandle);
    FreeHandle(_SetStrokeColorHandle);
    FreeHandle(_SetFillColorHandle);
    FreeHandle(_SetStrokeGrayHandle);
    FreeHandle(_SetFillGrayHandle);
    FreeHandle(_SetStrokeRGBHandle);
    FreeHandle(_SetFillRGBHandle);
    FreeHandle(_SetStrokeCMYKHandle);
    FreeHandle(_SetFillCMYKHandle);

    FreeHandle(_MPHandle);
    FreeHandle(_BMCHandle);
    FreeHandle(_BDCHandle);
    FreeHandle(_DPHandle);
    FreeHandle(_EMCHandle);
    FreeHandle(_BXHandle);
    FreeHandle(_EXHandle);
    FreeHandle(_DoFormHandle);
    FreeHandle(_DoImageHandle);
    FreeHandle(_ShadingHandle);
    FreeHandle(_BIHandle);
    FreeHandle(_ExtGStateBeginHandle);
    FreeHandle(_ExtGStateEndHandle);
    FreeHandle(_ExtGStateBMHandle);
    FreeHandle(_ExtGStateCAHandle);
    FreeHandle(_ExtGStatecaHandle);
    FreeHandle(_ExtGStateSMaskHandle);
    FreeHandle(_Type3FontSizeHandle);
    FreeHandle(_Type3FontGlyphHandle);

    // release special operation related GCHandle
    FreeHandle(_EndOfDataHandle);
    FreeHandle(_EndHandle);

    // release resources related GCHandle
    FreeHandle(_PushResourcesHandle);
    FreeHandle(_PopResourcesHandle);
}
