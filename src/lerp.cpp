#include "lerp.hpp"

double lerp(double t1, double t2, double t3)
{
	return (t1 * (1.0 - t3)) + (t2 * t3);
}

double easeIn(double t) { return t*t; }
double easeOut(double t)
{
	double inverse = 1.0 - t;
	return 1.0 - (inverse * inverse);
}
double easeInAndOut(double t) { return lerp(easeIn(t), easeOut(t), t); }

double easeInQuart(double t) { return t*t*t*t; }
double easeOutQuart(double t)
{
    double inverse = 1.0 - t;
    return 1.0 - (inverse * inverse * inverse * inverse);
}
double easeInAndOutQuart(double t)
{
    if (t < 0.5) return 8*t*t*t*t;

    double t_ = -2.0 * t + 2.0;

    return 1.0 - (t_*t_*t_*t_) / 2.0;
}

double easeOutBack(double t)
{
    const double c1 = 1.70158;
    const double c2 = c1 + 1.0;

    double tminus = t - 1.0;

    return 1.0 + c2 * (tminus * tminus * tminus) + c1 * (tminus * tminus);
}