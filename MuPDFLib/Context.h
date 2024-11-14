#include "mupdf/fitz.h"
#include "mupdf/pdf.h"

using namespace System;
using namespace System::Runtime::InteropServices;
using namespace System::Threading;

#ifndef __CONTEXT
#define __CONTEXT

namespace MuPDF {

#pragma warning( push )
#pragma warning( disable : 4091 )
typedef ref class Document;
typedef ref class Colorspace;
typedef enum class ColorspaceKind;
typedef ref class Pixmap;
typedef value struct BBox;
// in mupdf_load_system_font.c
extern "C" void install_load_windows_font_funcs(fz_context* ctx);
#pragma warning (pop)

public ref class Context : IDisposable {
public:
	~Context() {
		ReleaseHandle();
	}
	static property Context^ Instance {
		Context^ get();
	}

	/// <summary>
	/// Gets or sets render anti-alias level. Valid values are ranged [0, 8].
	/// </summary>
	property int AntiAlias {
		int get() { return fz_aa_level(_context); }
		void set(int value) { fz_set_aa_level(_context, value); }
	}
	property int TextAntiAlias {
		int get() { return fz_text_aa_level(_context); }
		void set(int value) { fz_set_text_aa_level(_context, value); }
	}

	Document^ OpenDocument(String^ filePath);

	Colorspace^ GetColorspace(ColorspaceKind kind);

	Pixmap^ CreatePixmap(ColorspaceKind colorspace, int width, int height);

	Pixmap^ CreatePixmap(ColorspaceKind colorspace, BBox box);

internal:
	static property fz_context* Ptr {
		fz_context* get() { return Instance->_context; }
	}

protected:
	!Context() {
		ReleaseHandle();
	}

private:
	fz_context* _context;
	bool _disposed;
	initonly bool _isCloned;

	static Context^ MakeMainContext();
	static Context^ _MainInstance = MakeMainContext();
	[System::ThreadStaticAttribute]
		static Context^ _Instance = _MainInstance;

	Context(fz_context* ctx, bool isCloned) : _context(ctx), _isCloned(isCloned) {
		if (!ctx) {
			throw gcnew InvalidOperationException("fz_context is null");
		}
		install_load_windows_font_funcs(ctx);
		fz_register_document_handlers(ctx);
	}

	void ReleaseHandle();

	fz_colorspace* GetFzColorspace(ColorspaceKind kind);
};

};

#endif // !__CONTEXT
