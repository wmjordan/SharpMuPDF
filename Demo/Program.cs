using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;
using MuPDF;

namespace Demo
{
	static class Program
	{
		static void Main (string[] args) {
			HashSet<PdfObject> visited = new HashSet<PdfObject>(10);
			using (var cookie = new Cookie())
			using (var pageInfo = new StreamWriter("pages.txt", false)) { // Creates the context
				try {
					using (var doc = Document.Open("test.pdf")) {
						Console.WriteLine("Page count: " + doc.PageCount);
						Console.WriteLine("Object count: " + doc.ObjectCount);

						// test dict modification with Unicode (Chinese) PDF string
						doc.Info.Set(PdfNames.Subject, "MuPDF \u4E2D\u6587 Test Document");
						doc.Info.Set(PdfNames.Producer, "MuPDF#");
						doc.Info.Set(PdfNames.ModDate, DateTime.Now);
						PrintPdfDictionary(doc.Info, pageInfo, visited);

						var pn = doc.PageCount;
						for (int i = 0; i < pn; i++) {
							Console.WriteLine("Rendering page " + (i + 1));
							using (var p = doc.LoadPage(i)) {
								File.WriteAllBytes($"Contents{i + 1}.bin.txt", p.GetContentBytes());
								var b = p.Bound;
								pageInfo.WriteLine("Page bound: " + b);
								PrintPdfDictionary(p.PdfObject, pageInfo, visited);
								using (var resources = p.PdfObject.Locate(PdfNames.Resources, PdfNames.XObject) as PdfDictionary) {
									if (resources != null) {
										foreach (var item in resources) {
											if (item.Value.UnderlyingObject is PdfDictionary d) {
												pageInfo.WriteLine(item.Key + " is image");
											}
										}
									}
									else {
										pageInfo.WriteLine("No XObject detected");
									}
								}
								pageInfo.WriteLine("Text in page:");
								using (var tp = p.TextPage) {
									foreach (var block in tp) {
										foreach (var line in block) {
											pageInfo.WriteLine($"{line}({line.FirstCharacter.Font.Name}, {line.FirstCharacter.Size} {line.FirstCharacter.Font.Flags})");
											foreach (var span in line.GetSpans()) {
												pageInfo.Write('\t');
												pageInfo.WriteLine(span.ToString());
											}
										}
									}
								}
								using (var bmp = RenderPage(doc, cookie, p, b)) {
									Console.WriteLine("Saving picture: " + (i + 1) + ".png");
									bmp.Save((i + 1) + ".png"); // saves the bitmap to a file
								}
							}
						}
					}
				}
				catch (MuException ex) {
					Console.Error.WriteLine("Error occurred while rendering document!");
					Console.Error.WriteLine("Error code: " + ex.Code);
					Console.Error.WriteLine(ex.ToString());
				}
			}
			Console.WriteLine ("Program finished. Press any key to quit.");
			Console.ReadKey (true);
		}

		static void PrintPdfDictionary(PdfDictionary dict, StreamWriter writer, HashSet<PdfObject> visited, int indent = 0) {
			int count = dict.Count;
			writer.WriteLine(dict.Type + ": " + count + " items");
			for (int i = 0; i < count; i++) {
				var item = dict[i];
				using (PdfObject o = item.Value.UnderlyingObject) {
					writer.Write(new string(' ', indent << 1));
					writer.Write(item.Key + ": ");
					if (visited.Add(o) == false) {
						writer.WriteLine(item.Value.ToString());
						continue;
					}
					switch (o.TypeKind) {
						case Kind.Array:
							PrintPdfArray(writer, o as PdfArray);
							continue;
						case Kind.Dictionary:
							PrintPdfDictionary(o as PdfDictionary, writer, visited, indent + 1);
							continue;
					}
					writer.WriteLine(o);
				}
			}
		}

		static void PrintPdfArray(StreamWriter writer, PdfArray array) {
			int count = array.Count;
			StringBuilder sb = new StringBuilder(20)
				.Append('[');
			for (int i = 0; i < count; i++) {
				var item = array[i];
				if (i != 0) {
					sb.Append(' ');
				}
				sb.Append(item);
			}
			writer.WriteLine(sb.Append(']').ToString());
		}

		static Bitmap RenderPage (Document document, Cookie cookie, Page page, Box pageBound) {
			int width = (int)(pageBound.Width); // gets the size of the page
			int height = (int)(pageBound.Height);
			// creates a pixmap the same size as the width and height of the page
			using (Pixmap
#if UNSAFE
			pix = Pixmap.Create(ColorspaceKind.Rgb, width, height)
#else
			// use BGR color space to save byte conversions
			pix = Pixmap.Create(ColorspaceKind.Bgr, width, height)
#endif
			) {
				// sets white color as the background color of the pixmap
				pix.SetBackgroundWhite();
				// creates a drawing device
				using (var dev = Device.NewDraw(pix)) {
					// draws the page on the device created from the pixmap
					page.Run(dev, cookie);
					// ends the drawing procedure
					dev.Close();
				}

				// creates a colorful bitmap of the same size of the pixmap
				Bitmap bmp = new Bitmap (width, height, PixelFormat.Format24bppRgb); 
				var imageData = bmp.LockBits (new System.Drawing.Rectangle (0, 0, width, height), ImageLockMode.ReadWrite, bmp.PixelFormat);

	#if UNSAFE
				// note: unsafe conversion from pixmap to bitmap
				// without the overhead of P/Invokes, the following code can run faster than the safe-conversion code below
				unsafe { // converts the pixmap data to Bitmap data
					byte* ptrSrc = (byte*)pix.Samples; // gets the rendered data from the pixmap
					byte* ptrDest = (byte*)imageData.Scan0;
					for (int y = 0; y < height; y++) {
						byte* pl = ptrDest;
						byte* sl = ptrSrc;
						for (int x = 0; x < width; x++) {
							//Swap these here instead of in MuPDF because most pdf images will be rgb or cmyk.
							//Since we are going through the pixels one by one anyway swap here to save a conversion from rgb to bgr.
							pl[2] = sl[0]; //b-r
							pl[1] = sl[1]; //g-g
							pl[0] = sl[2]; //r-b
							pl += 3;
							sl += 3;
						}
						ptrDest += imageData.Stride;
						ptrSrc += width * 3;
					}
				}
	#else
				// note: Safe-conversion from pixmap to bitmap
				var source = pix.Samples;
				var target = imageData.Scan0;
				for (int y = 0; y < height; y++) {
					// copy memory line by line
					NativeMethods.RtlMoveMemory(target, source, width * 3);
					target = (IntPtr)(target.ToInt64() + imageData.Stride);
					source = (IntPtr)(source.ToInt64() + width * 3);
				}
	#endif
				bmp.UnlockBits(imageData);
				return bmp;
			}
		}
		static class NativeMethods
		{
			[DllImport("kernel32.dll")]
			public static extern void RtlMoveMemory(IntPtr dest, IntPtr src, int byteCount);
		}
	}
}
