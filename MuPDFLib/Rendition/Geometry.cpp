#include "Geometry.h"
#include <cfloat>

MuPDF::Box MuPDF::Box::Transform(Matrix matrix) {
	return fz_transform_rect(*this, matrix);
}

MuPDF::BBox MuPDF::Box::Round() {
	return (BBox)*this;
}

MuPDF::Box::operator Box(BBox box) {
	return { (float)box.X0, (float)box.Y0, (float)box.X1, (float)box.Y1 };
}

MuPDF::Box MuPDF::Box::operator&(Box a, Box b) {
	if (b.IsInfinite) return a;
	if (a.IsInfinite) return b;
	return { a.X0 < b.X0 ? b.X0 : a.X0,
		a.Y0 < b.Y0 ? b.Y0 : a.Y0,
		a.X1 > b.X1 ? b.X1 : a.X1,
		a.Y1 > b.Y1 ? b.Y1 : a.Y1
	};
}

MuPDF::Box MuPDF::Box::operator|(Box a, Box b) {
	/* Check for empty box before infinite box */
	if (!b.IsValid) return a;
	if (!a.IsValid) return b;
	if (a.IsInfinite) return a;
	if (b.IsInfinite) return b;
	return { a.X0 > b.X0 ? b.X0 : a.X0,
		a.Y0 > b.Y0 ? b.Y0 : a.Y0,
		a.X1 < b.X1 ? b.X1 : a.X1,
		a.Y1 < b.Y1 ? b.Y1 : a.Y1 };
}

MuPDF::BBox MuPDF::BBox::operator|(BBox a, BBox b) {
	/* Check for empty box before infinite box */
	if (!b.IsValid) return a;
	if (!a.IsValid) return b;
	if (a.IsInfinite) return a;
	if (b.IsInfinite) return b;
	return { a.X0 > b.X0 ? b.X0 : a.X0,
		a.Y0 > b.Y0 ? b.Y0 : a.Y0,
		a.X1 < b.X1 ? b.X1 : a.X1,
		a.Y1 < b.Y1 ? b.Y1 : a.Y1 };
}

MuPDF::BBox::operator BBox(Box box) {
	const static float t = 0.001f;
	return { (int)floor(box.X0 + t), (int)floor(box.Y0 + t), (int)ceil(box.X1 - t), (int)ceil(box.Y1 - t) };
}

MuPDF::BBox MuPDF::BBox::operator&(BBox a, BBox b) {
	if (b.IsInfinite) return a;
	if (a.IsInfinite) return b;
	return { a.X0 < b.X0 ? b.X0 : a.X0,
		a.Y0 < b.Y0 ? b.Y0 : a.Y0,
		a.X1 > b.X1 ? b.X1 : a.X1,
		a.Y1 > b.Y1 ? b.Y1 : a.Y1
	};
}

MuPDF::Matrix MuPDF::Matrix::Concat(Matrix value) {
	return Matrix(
		A * value.A + B * value.C,
		A * value.B + B * value.D,
		C * value.A + D * value.C,
		C * value.B + D * value.D,
		E * value.A + F * value.C + value.E,
		E * value.B + F * value.D + value.F);
}

MuPDF::Matrix MuPDF::Matrix::RotateTo(float theta) {
	while (theta < 0)
		theta += 360;
	while (theta >= 360)
		theta -= 360;

	if (fabs(0 - theta) < FLT_EPSILON) {
		return *this;
	}
	if (fabs(90.0f - theta) < FLT_EPSILON) {
		return Matrix(C, D, -A, -B, E, F);
	}
	if (fabs(180.0f - theta) < FLT_EPSILON) {
		return Matrix(-A, -B, -C, -D, E, F);
	}
	if (fabs(270.0f - theta) < FLT_EPSILON) {
		return Matrix(-C, -D, A, B, E, F);
	}
	float s = sin(theta * FZ_PI / 180);
	float c = cos(theta * FZ_PI / 180);
	return Matrix(c * A + s * C, c * B + s * D, -s * A + c * C, -s * B + c * D, E, F);
}

MuPDF::Matrix MuPDF::Matrix::ShearTo(float h, float v) {
	return Matrix(
		v * C + A,
		v * D + B,
		h * A + C,
		h * B + D,
		E, F);
}

MuPDF::Matrix MuPDF::Matrix::Rotate(float theta) {
	float s;
	float c;

	while (theta < 0)
		theta += 360;
	while (theta >= 360)
		theta -= 360;

	if (fabs(0 - theta) < FLT_EPSILON) {
		s = 0;
		c = 1;
	}
	else if (fabs(90.0f - theta) < FLT_EPSILON) {
		s = 1;
		c = 0;
	}
	else if (fabs(180.0f - theta) < FLT_EPSILON) {
		s = 0;
		c = -1;
	}
	else if (fabs(270.0f - theta) < FLT_EPSILON) {
		s = -1;
		c = 0;
	}
	else {
		s = sin(theta * FZ_PI / 180);
		c = cos(theta * FZ_PI / 180);
	}
	return MuPDF::Matrix(c, s, -s, c, 0, 0);
}

MuPDF::Quad MuPDF::Quad::Union(Quad other) {
	float x1 = fz_min(fz_min(UpperLeft.X, other.UpperLeft.X), fz_min(LowerLeft.X, other.LowerLeft.X));
	float x2 = fz_max(fz_max(UpperLeft.X, other.UpperLeft.X), fz_max(LowerLeft.X, other.LowerLeft.X));
	float y1 = fz_min(fz_min(UpperLeft.Y, other.UpperLeft.Y), fz_min(LowerLeft.Y, other.LowerLeft.Y));
	float y2 = fz_max(fz_max(UpperLeft.Y, other.UpperLeft.Y), fz_max(LowerLeft.Y, other.LowerLeft.Y));
	return Quad(Point(x1, y1), Point(x2, y2), Point(x1, y2), Point(x2, y2));
}

MuPDF::Box MuPDF::Quad::ToBox() {
	float x0, y0, x1, y1;
	x0 = fz_min(fz_min(LowerLeft.X, LowerRight.X), fz_min(UpperLeft.X, UpperRight.X));
	y0 = fz_min(fz_min(LowerLeft.Y, LowerRight.Y), fz_min(UpperLeft.Y, UpperRight.Y));
	x1 = fz_max(fz_max(LowerLeft.X, LowerRight.X), fz_max(UpperLeft.X, UpperRight.X));
	y1 = fz_max(fz_max(LowerLeft.Y, LowerRight.Y), fz_max(UpperLeft.Y, UpperRight.Y));
	return Box(x0, y0, x1, y1);
}

bool MuPDF::Quad::IsPointInsideTriangle(Point p, Point a, Point b, Point c) {
	float s, t, area;
	s = a.Y * c.X - a.X * c.Y + (c.Y - a.Y) * p.X + (a.X - c.X) * p.Y;
	t = a.X * b.Y - a.Y * b.X + (a.Y - b.Y) * p.X + (b.X - a.X) * p.Y;

	if ((s < 0) != (t < 0))
		return 0;

	area = -b.Y * c.X + a.Y * (c.X - b.X) + a.X * (b.Y - c.Y) + b.X * c.Y;

	return area < 0 ?
		(s <= 0 && s + t >= area) :
		(s >= 0 && s + t <= area);
}
