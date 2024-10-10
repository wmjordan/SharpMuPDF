#include "mupdf/fitz.h"
#include "mupdf/pdf.h"
#include <cfloat>
#include <math.h>

#ifndef __GEOMETRY
#define __GEOMETRY

#pragma once
using namespace System;

namespace MuPDF {

public value struct Point : IEquatable<Point> {
public:
	initonly float X, Y;
	Point(float x, float y) { X = x; Y = y; }

	virtual int GetHashCode() override {
		return (static_cast<int>(X) << 16) | static_cast<int>(Y) ^ (static_cast<int>(X) >> 16);
	}
	virtual bool Equals(Object^ other) override {
		Point p;
		return other && other->GetType() == Point::typeid
			&& (p = safe_cast<Point>(other)).X == X
			&& Y == p.Y;
	}
	virtual bool Equals(Point other) {
		return *this == other;
	}
	static bool operator == (Point a, Point b) {
		return a.X == b.X && a.Y == b.Y;
	}
	static bool operator != (Point a, Point b) {
		return a.X != b.X || a.Y != b.Y;
	}
	static operator Point(fz_point p) {
		return Point(p.x, p.y);
	}
	virtual String^ ToString() override {
		return String::Concat(X, ",", Y);
	}
};

value struct BBox;
value struct Matrix;

public value struct Box {
public:
	initonly float X0, Y0, X1, Y1;
	static initonly Box Unit = { 0, 0, 1, 1 };
	static initonly Box Invalid = { 0, 0, -1, -1 };
	Box(float x0, float y0, float x1, float y1) : X0(x0), X1(x1), Y0(y0), Y1(y1) {};
	property bool IsEmpty {
		bool get() {
			return X0 == X1 || Y0 == Y1;
		}
	}
	property float Width {
		float get() {
			return X1 > X0 ? X1 - X0 : 0;
		}
	}
	property float Height {
		float get() {
			return Y1 > Y0 ? Y1 - Y0 : 0;
		}
	}
	property float Top { float get() { return Y0; } }
	property float Left { float get() { return X0; } }
	property float Bottom { float get() { return Y1; } }
	property float Right { float get() { return X1; } }
	property bool IsInfinite {
		bool get() {
			return X0 == FZ_MIN_INF_RECT && X1 == FZ_MAX_INF_RECT &&
				Y0 == FZ_MIN_INF_RECT && Y1 == FZ_MAX_INF_RECT;
		}
	}
	property bool IsValid {
		bool get() { return X0 <= X1 && Y0 <= Y1; }
	}
	property float Area {
		float get() { return IsValid ? (X1 - X0) * (Y1 - Y0) : 0; }
	}

	bool Contains(Point p) {
		return p.X >= X0 && p.X < X1 && p.Y >= Y0 && p.Y < Y1;
	}
	Box Union(Box other) {
		return *this | other;
	}
	Box Intersect(Box other) {
		return *this & other;
	}
	Box Translate(float offsetX, float offsetY) {
		if (IsInfinite) return *this;
		return { X0 + offsetX, Y0 + offsetY, X1 + offsetX, Y1 + offsetY };
	}
	Box Transform(Matrix matrix);
	BBox Round();
	virtual String^ ToString() override {
		return String::Concat("(", X0, ",", Y0, ")-(", X1, ",", Y1, ")");
	}
	static explicit operator Box(BBox box);
	static operator Box(fz_rect rect) {
		return Box (rect.x0, rect.y0, rect.x1, rect.y1);
	}
	static Box operator & (Box a, Box b);
	static Box operator | (Box a, Box b);
	static operator fz_rect (Box b) {
		return { b.X0, b.Y0, b.X1, b.Y1 };
	}
};

public value struct BBox {
public:
	initonly int X0, Y0, X1, Y1;
	static initonly BBox Unit = { 0, 0, 1, 1 };
	static initonly BBox Invalid = { 0, 0, -1, -1 };
	BBox(int x0, int y0, int x1, int y1) : X0(x0), X1(x1), Y0(y0), Y1(y1) {};
	property int Width {
		int get() {
			return abs(X1 - X0);
		}
	}
	property int Height {
		int get() {
			return abs(Y1 - Y0);
		}
	}
	property int Top { int get() { return Y0; } }
	property int Left { int get() { return X0; } }
	property int Bottom { int get() { return Y1; } }
	property int Right { int get() { return X1; } }
	property bool IsInfinite {
		bool get() {
			return X0 == FZ_MIN_INF_RECT && X1 == FZ_MAX_INF_RECT &&
				Y0 == FZ_MIN_INF_RECT && Y1 == FZ_MAX_INF_RECT;
		}
	}
	property bool IsValid {
		bool get() { return X0 <= X1 && Y0 <= Y1; }
	}
	bool ContainsPoint(int x, int y) {
		return x >= X0 && x < X1 && y >= Y0 && y < Y1;
	}
	BBox Intersect(BBox other) {
		return *this & other;
	}
	BBox Union(BBox other) {
		return *this | other;
	}
	virtual String^ ToString() override {
		return String::Concat("(", X0, ",", Y0, ")-(", X1, ",", Y1, ")");
	}
	static explicit operator BBox(Box box);
	static BBox operator & (BBox a, BBox b);
	static BBox operator | (BBox a, BBox b);
	static operator BBox(fz_irect rect) {
		return BBox(rect.x0, rect.y0, rect.x1, rect.y1);
	}
	static operator fz_irect (BBox b) {
		return { b.X0, b.Y0, b.X1, b.Y1 };
	}
};

public value struct Quad {
public:
	initonly Point UpperLeft, UpperRight, LowerLeft, LowerRight;
	Quad(Point ul, Point ur, Point ll, Point lr) : UpperLeft(ul), UpperRight(ur), LowerLeft(ll), LowerRight(lr) {};

	bool Contains(Point p) {
		return
			IsPointInsideTriangle(p, UpperLeft, UpperRight, LowerRight) ||
			IsPointInsideTriangle(p, UpperLeft, LowerRight, LowerLeft);
	}
	bool Contains(Quad other) {
		return
			Contains(other.UpperLeft) &&
			Contains(other.UpperRight) &&
			Contains(other.LowerLeft) &&
			Contains(other.LowerRight);
	}
	Quad Union(Quad other);
	Box ToBox();

	virtual String^ ToString() override {
		return String::Concat("{(", UpperLeft.X, ",", UpperLeft.Y, "),(", UpperRight.X, ",", UpperRight.Y, "),(", LowerLeft.X, ",", LowerLeft.Y, "),(", LowerRight.X, ",", LowerRight.Y, ")}");
	}
	static operator Quad(fz_quad quad) {
		return Quad(quad.ul, quad.ur, quad.ll, quad.lr);
	}
	static operator fz_quad(Quad quad) {
		pin_ptr<Quad> p = &quad;
		return *(fz_quad*)p;
	}
	static operator Quad(fz_rect rect) {
		return Quad(Point(rect.x0, rect.y0), Point(rect.x1, rect.y0), Point(rect.x0, rect.y1), Point(rect.x1, rect.y1));
	}
	static operator Quad(Box rect) {
		return Quad(Point(rect.X0, rect.Y0), Point(rect.X1, rect.Y0), Point(rect.X0, rect.Y1), Point(rect.X1, rect.Y1));
	}

	static bool IsPointInsideTriangle(Point p, Point a, Point b, Point c);
};

public value struct Matrix {
public:
	initonly float A, B, C, D, E, F;
	static initonly Matrix Identity = Matrix(1, 0, 0, 1, 0, 0);
	static initonly Matrix VerticalFlip = Matrix(1, 0, 0, -1, 0, 0);
	static initonly Matrix HorizontalFlip = Matrix(-1, 0, 0, 1, 0, 0);

	Matrix(float a, float b, float c, float d, float e, float f) : A(a), B(b), C(c), D(d), E(e), F(f) {};
	Matrix Concat(Matrix value);
	Matrix PreScale(float sx, float sy) {
		return Matrix(A * sx, B * sx, C * sy, D * sy, 0, 0);
	}
	Matrix PostScale(float sx, float sy) {
		return Matrix(A * sx, B * sx, C * sy, D * sy, E * sx, F * sy);
	}
	Matrix RotateTo(float theta);
	Matrix ShearTo(float h, float v);
	Matrix TranslateTo(float tx, float ty) {
		return Matrix(A, B, C, D, tx * A + ty * C + E, tx * B + ty * D + F);
	}
	static Matrix Rotate(float theta);
	static Matrix Scale(float x, float y) {
		return Matrix(x, 0, 0, y, 0, 0);
	}
	static Matrix Shear(float h, float v) {
		return Matrix(1, v, h, 1, 0, 0);
	}
	static Matrix Translate(float tx, float ty) {
		return Matrix(1, 0, 0, 1, tx, ty);
	}
	virtual String^ ToString() override {
		return String::Concat("[", A, ",", B, ",", C, ",", D, ",", E, ",", F, "]");
	}
	static operator fz_matrix(Matrix matrix) {
		pin_ptr<Matrix> p = &matrix;
		return *(fz_matrix*)p;
		//return { matrix.A,matrix.B,matrix.C,matrix.D,matrix.E,matrix.F };
	}
};
};

#endif // !__GEOMETRY
