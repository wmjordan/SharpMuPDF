#include "TextPage.h"

MuPDF::TextOptions::operator fz_stext_options(TextOptions^ options) {
	fz_stext_options s{};
	if (options) {
		s.flags = (int)options->Flags;
		s.scale = options->Scale;
	}
	return s;
}

String^ MuPDF::TextSpan::ToString() {
    int l = _length;
    auto sb = gcnew StringBuilder(l);
    fz_stext_char* c = _ch;
    do {
        sb->Append((Char)c->c);
    } while (--l > 0 && (c = c->next));
    return sb->ToString();
}

String^ MuPDF::TextLine::ToString() {
    auto sb = gcnew StringBuilder(16);
    fz_stext_char* c = _line->first_char;
    do {
        sb->Append((Char)c->c);
    } while (c = c->next);
    return sb->ToString();
}

String^ MuPDF::TextBlock::ToString() {
	if (IsImageBlock) {
		return String::Empty;
	}
	StringBuilder^ sb = gcnew StringBuilder(16);
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

bool MaybeUtf8(const char* text) {
    unsigned int b = 0; // byte count
    const char* p = text;
    char c;
    while (c = *p) {
        if (b == 0) {
            if (c < 0x80) {
                goto NEXT;
            }
            // multibyte
            if (c < 0xC0) {
                b = 1;
            }
            else if (c < 0xE0) {
                b = 2;
            }
            else if (c < 0xF0) {
                b = 3;
            }
            else if (c < 0xF8) {
                b = 4;
            }
            else if (c < 0xFE) {
                b = 5;
            }
            else {
                return false;
            }
        }
        else {
            // multi-byte subsequent char: 10xxxxxx
            if ((c & 0xC0) != 0x80) {
                return false;
            }
            b--;
        }
    NEXT:
        ++p;
    }
    return b == 0;
}

bool MuPDF::TextLine::TextLineSpanContainer::MoveNext() {
    if (!_start) {
        return false;
    }
    auto end = _start;
    auto p = end->next;
    auto font = _start->font;
    auto color = _start->argb;
    auto size = _start->size;
    Quad quad = _start->quad;
    int length = 1;
    while (p) {
        if (p->font != font || p->size != size || p->argb != color) {
            _Current = gcnew TextSpan(_start, length, quad.Union(p->quad).ToBox(), _Line->IsVertical);
            _start = p;
            return true;
        }
        end = p;
        p = p->next;
        ++length;
    }
    _Current = gcnew TextSpan(_start, length, end == _start ? quad.ToBox() : quad.Union(end->quad).ToBox(), _Line->IsVertical);
    _start = NULL;
    return true;
}

void MuPDF::TextLine::TextLineSpanContainer::Reset() {
    _Current = nullptr;
    _active = NULL;
    _start = _Line->_line->first_char;
}
