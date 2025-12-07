using System;
using System.IO;
using MuPDF;

namespace Demo
{
	public class ProcessorLogger : Processor
	{
		public ProcessorLogger(StreamWriter writer) {
			Writer = writer;
		}

		public StreamWriter Writer { get; }

		protected override void BeginText() {
			Writer.WriteLine($"BeginText() called");
		}

		protected override void EndText() {
			Writer.WriteLine($"EndText() called");
		}

		protected override void ShowText(string text) {
			Writer.WriteLine($"ShowText(text: {text}) called");
		}

		protected override void ShowTextNewLine(string text) {
			Writer.WriteLine($"ShowTextNewLine(text: {text}) called");
		}

		protected override void ShowTextWithPositioning(PdfObject array) {
			if (array.IsArray) {
				Writer.Write("ShowTextWithPositioning: ");
				foreach (var item in (PdfArray)array) {
					if (item.IsString) {
						Writer.Write(BitConverter.ToString(((PdfString)item).GetBytes()));
					}
					else {
						Writer.Write('(');
						Writer.Write(item.FloatValue);
						Writer.Write(')');
					}
				}
				Writer.WriteLine();
			}
			else {
				Writer.WriteLine($"ShowTextWithPositioning(array: {array}) called");
			}
		}

		protected override void ShowTextNewLineWithSpacing(float aw, float ac, string text) {
			Writer.WriteLine($"ShowTextNewLineWithSpacing(aw: {aw}, ac: {ac}, text: {text}) called");
		}

		protected override void SetLineWidth(float width) {
			Writer.WriteLine($"SetLineWidth(width: {width}) called");
		}

		protected override void SetLineJoin(int linejoin) {
			Writer.WriteLine($"SetLineJoin(linejoin: {linejoin}) called");
		}

		protected override void SetLineCap(int linecap) {
			Writer.WriteLine($"SetLineCap(linecap: {linecap}) called");
		}

		protected override void SetMiterLimit(float limit) {
			Writer.WriteLine($"SetMiterLimit(limit: {limit}) called");
		}

		protected override void SetDashPattern(PdfObject array, float phase) {
			Writer.WriteLine($"SetDashPattern(array: {array}, phase: {phase}) called");
		}

		protected override void SetRenderingIntent(string intent) {
			Writer.WriteLine($"SetRenderingIntent(intent: {intent}) called");
		}

		protected override void SetFlatness(float flatness) {
			Writer.WriteLine($"SetFlatness(flatness: {flatness}) called");
		}

		protected override void SaveState() {
			Writer.WriteLine($"SaveState() called");
		}

		protected override void RestoreState() {
			Writer.WriteLine($"RestoreState() called");
		}

		protected override void SetTransform(float a, float b, float c, float d, float e, float f) {
			Writer.WriteLine($"SetTransform(a: {a}, b: {b}, c: {c}, d: {d}, e: {e}, f: {f}) called");
		}

		protected override void MoveTo(float x, float y) {
			Writer.WriteLine($"MoveTo(x: {x}, y: {y}) called");
		}

		protected override void LineTo(float x, float y) {
			Writer.WriteLine($"LineTo(x: {x}, y: {y}) called");
		}

		protected override void CurveTo(float x1, float y1, float x2, float y2, float x3, float y3) {
			Writer.WriteLine($"CurveTo(x1: {x1}, y1: {y1}, x2: {x2}, y2: {y2}, x3: {x3}, y3: {y3}) called");
		}

		protected override void CurveToV(float x2, float y2, float x3, float y3) {
			Writer.WriteLine($"CurveToV(x2: {x2}, y2: {y2}, x3: {x3}, y3: {y3}) called");
		}

		protected override void CurveToY(float x1, float y1, float x3, float y3) {
			Writer.WriteLine($"CurveToY(x1: {x1}, y1: {y1}, x3: {x3}, y3: {y3}) called");
		}

		protected override void ClosePath() {
			Writer.WriteLine($"ClosePath() called");
		}

		protected override void Rectangle(float x, float y, float w, float h) {
			Writer.WriteLine($"Rectangle(x: {x}, y: {y}, w: {w}, h: {h}) called");
		}

		protected override void StrokePath() {
			Writer.WriteLine($"StrokePath() called");
		}

		protected override void CloseAndStrokePath() {
			Writer.WriteLine($"CloseAndStrokePath() called");
		}

		protected override void FillPath() {
			Writer.WriteLine($"FillPath() called");
		}

		protected override void FillPathEvenOdd() {
			Writer.WriteLine($"FillPathEvenOdd() called");
		}

		protected override void FillAndStrokePath() {
			Writer.WriteLine($"FillAndStrokePath() called");
		}

		protected override void FillAndStrokePathEvenOdd() {
			Writer.WriteLine($"FillAndStrokePathEvenOdd() called");
		}

		protected override void CloseFillAndStrokePath() {
			Writer.WriteLine($"CloseFillAndStrokePath() called");
		}

		protected override void CloseFillAndStrokePathEvenOdd() {
			Writer.WriteLine($"CloseFillAndStrokePathEvenOdd() called");
		}

		protected override void EndPath() {
			Writer.WriteLine($"EndPath() called");
		}

		protected override void ClipPath() {
			Writer.WriteLine($"ClipPath() called");
		}

		protected override void ClipPathEvenOdd() {
			Writer.WriteLine($"ClipPathEvenOdd() called");
		}

		protected override void SetCharSpacing(float spacing) {
			Writer.WriteLine($"SetCharSpacing(spacing: {spacing}) called");
		}

		protected override void SetWordSpacing(float spacing) {
			Writer.WriteLine($"SetWordSpacing(spacing: {spacing}) called");
		}

		protected override void SetHorizontalScaling(float scale) {
			Writer.WriteLine($"SetHorizontalScaling(scale: {scale}) called");
		}

		protected override void SetLeading(float leading) {
			Writer.WriteLine($"SetLeading(leading: {leading}) called");
		}

		protected override void SetFont(string name, FontDescriptor font, float size) {
			Writer.WriteLine($"SetFont(name: {name}({font.Font.Name}), size: {size}) called");
		}

		protected override void SetTextRenderMode(int mode) {
			Writer.WriteLine($"SetTextRenderMode(mode: {mode}) called");
		}

		protected override void SetTextRise(float rise) {
			Writer.WriteLine($"SetTextRise(rise: {rise}) called");
		}

		protected override void TextMove(float tx, float ty) {
			Writer.WriteLine($"TextMove(tx: {tx}, ty: {ty}) called");
		}

		protected override void TextMoveSetLeading(float tx, float ty) {
			Writer.WriteLine($"TextMoveSetLeading(tx: {tx}, ty: {ty}) called");
		}

		protected override void SetTextMatrix(float a, float b, float c, float d, float e, float f) {
			Writer.WriteLine($"SetTextMatrix(a: {a}, b: {b}, c: {c}, d: {d}, e: {e}, f: {f}) called");
		}

		protected override void TextNewLine() {
			Writer.WriteLine($"TextNewLine() called");
		}

		protected override void SetStrokeColorSpace(string name) {
			Writer.WriteLine($"SetStrokeColorSpace(name: {name}) called");
		}

		protected override void SetFillColorSpace(string name) {
			Writer.WriteLine($"SetFillColorSpace(name: {name}) called");
		}

		protected override void SetStrokeColor(int n, float[] color) {
			Writer.WriteLine($"SetStrokeColor(n: {n}, color: [{string.Join(", ", color)}]) called");
		}

		protected override void SetFillColor(int n, float[] color) {
			Writer.WriteLine($"SetFillColor(n: {n}, color: [{string.Join(", ", color)}]) called");
		}

		protected override void SetStrokeGray(float gray) {
			Writer.WriteLine($"SetStrokeGray(gray: {gray}) called");
		}

		protected override void SetFillGray(float gray) {
			Writer.WriteLine($"SetFillGray(gray: {gray}) called");
		}

		protected override void SetStrokeRGB(float r, float g, float b) {
			Writer.WriteLine($"SetStrokeRGB(r: {r}, g: {g}, b: {b}) called");
		}

		protected override void SetFillRGB(float r, float g, float b) {
			Writer.WriteLine($"SetFillRGB(r: {r}, g: {g}, b: {b}) called");
		}

		protected override void SetStrokeCMYK(float c, float m, float y, float k) {
			Writer.WriteLine($"SetStrokeCMYK(c: {c}, m: {m}, y: {y}, k: {k}) called");
		}

		protected override void SetFillCMYK(float c, float m, float y, float k) {
			Writer.WriteLine($"SetFillCMYK(c: {c}, m: {m}, y: {y}, k: {k}) called");
		}

		protected override void PushResources(PdfObject res) {
			Writer.WriteLine($"PushResources(res: {res}) called");
		}

		protected override void PopResources(PdfObject res) {
			Writer.WriteLine($"PopResources() called");
		}

		protected override void EndOfData() {
			Writer.WriteLine($"EndOfData() called");
		}

		protected override void End() {
			Writer.WriteLine($"End() called");
		}

		// Marked Content
		protected override void BeginMarkedContent(string tag) {
			Writer.WriteLine($"BeginMarkedContent(tag: {tag}) called");
		}

		protected override void BeginMarkedContentWithProps(string tag, PdfObject raw, PdfObject cooked) {
			Writer.WriteLine($"BeginMarkedContentWithProps(tag: {tag}, raw: {raw}, cooked: {cooked}) called");
		}

		protected override void EndMarkedContent() {
			Writer.WriteLine($"EndMarkedContent() called");
		}

		protected override void MarkedContentPoint(string tag) {
			Writer.WriteLine($"MarkedContentPoint(tag: {tag}) called");
		}

		protected override void MarkedContentPointWithProps(string tag, PdfObject raw, PdfObject cooked) {
			Writer.WriteLine($"MarkedContentPointWithProps(tag: {tag}, raw: {raw}, cooked: {cooked}) called");
		}

		// Compatibility
		protected override void BeginCompatibilitySection() {
			Writer.WriteLine($"BeginCompatibilitySection() called");
		}

		protected override void EndCompatibilitySection() {
			Writer.WriteLine($"EndCompatibilitySection() called");
		}

		// XObjects
		protected override void DoFormXObject(string name, PdfObject form) {
			Writer.WriteLine($"DoFormXObject(name: {name}, form: {form}) called");
		}

		protected override void DoImageXObject(string name, Image image) {
			Writer.WriteLine($"DoImageXObject(name: {name}, image: {image?.ToString()}) called");
		}

		// Shading
		protected override void PaintShading(string name) {
			Writer.WriteLine($"PaintShading(name: {name} called");
		}

		// Inline Image
		protected override void PaintInlineImage(Image image, string colorspaceName) {
			Writer.WriteLine($"PaintInlineImage(image: {image}, colorspaceName: {colorspaceName}) called");
		}

		// ExtGState
		protected override void BeginExtGState(string name, PdfObject extgstate) {
			Writer.WriteLine($"BeginExtGState(name: {name}, extgstate: {extgstate}) called");
		}

		protected override void EndExtGState() {
			Writer.WriteLine($"EndExtGState() called");
		}

		protected override void SetExtGStateBlendMode(string blendmode) {
			Writer.WriteLine($"SetExtGStateBlendMode(blendmode: {blendmode}) called");
		}

		protected override void SetExtGStateStrokeAlpha(float alpha) {
			Writer.WriteLine($"SetExtGStateStrokeAlpha(alpha: {alpha}) called");
		}

		protected override void SetExtGStateFillAlpha(float alpha) {
			Writer.WriteLine($"SetExtGStateFillAlpha(alpha: {alpha}) called");
		}

		protected override void SetExtGStateSoftMask(PdfObject smask, Colorspace smask_cs, float[] bc, bool luminosity, PdfObject tr) {
			Writer.WriteLine($"SetExtGStateSoftMask(smask: {smask}, smask_cs: {smask_cs}, " +
							 $"bc: [{string.Join(", ", bc)}], luminosity: {luminosity}, tr: {tr}) called");
		}

		// Type3 Fonts
		protected override void SetType3FontSize(float wx, float wy) {
			Writer.WriteLine($"SetType3FontSize(wx: {wx}, wy: {wy}) called");
		}

		protected override void SetType3FontGlyph(float wx, float wy, float llx, float lly, float urx, float ury) {
			Writer.WriteLine($"SetType3FontGlyph(wx: {wx}, wy: {wy}, llx: {llx}, lly: {lly}, urx: {urx}, ury: {ury}) called");
		}
	}
}
