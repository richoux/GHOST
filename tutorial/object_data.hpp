#pragma once

class ObjectData
{
	double _size;
	double _value;

public:
	ObjectData( double, double );
	inline double size() const { return _size; }
	inline double value() const { return _value; }
};


