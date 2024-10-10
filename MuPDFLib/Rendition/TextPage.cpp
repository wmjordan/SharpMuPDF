#include "TextPage.h"

MuPDF::TextOptions::operator fz_stext_options(TextOptions^ options) {
	fz_stext_options s{};
	if (options) {
		s.flags = (int)options->Flags;
		s.scale = options->Scale;
	}
	return s;
}

String^ MuPDF::TextBlock::ToString() {
	if (IsImageBlock) {
		return String::Empty;
	}
	StringBuilder^ sb = gcnew StringBuilder();
	fz_stext_line* l = _block->u.t.first_line;
	do {
		fz_stext_char* c = l->first_char;
		do {
			sb->Append((Char)c->c);
		} while (c = c->next);
		sb->AppendLine();
	} while (l = l->next);
	return sb->ToString();
}
