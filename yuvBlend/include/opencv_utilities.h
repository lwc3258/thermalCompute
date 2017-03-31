

inline int round_opencv(float value)
{
	//return (int)(floor(num + 0.5f));
#if (defined _MSC_VER && defined _M_X64) || (defined __GNUC__ && defined __x86_64__ && defined __SSE2__ && !defined __APPLE__)
	__m128d t = _mm_set_sd(value);
	return _mm_cvtsd_si32(t);
#elif defined _MSC_VER && defined _M_IX86
	int t;
	__asm
	{
		fld value;
		fistp t;
	}
	return t;
#elif defined _MSC_VER && defined _M_ARM && defined HAVE_TEGRA_OPTIMIZATION
	TEGRA_ROUND(value);
#elif defined CV_ICC || defined __GNUC__
#  ifdef HAVE_TEGRA_OPTIMIZATION
	TEGRA_ROUND(value);
#  else
	return (int)lrint(value);
#  endif
#else
	double intpart, fractpart;
	fractpart = modf(value, &intpart);
	if ((fabs(fractpart) != 0.5) || ((((int)intpart) % 2) != 0))
		return (int)(value + (value >= 0 ? 0.5 : -0.5));
	else
		return (int)intpart;
#endif
}



/*!
Aligns pointer by the certain number of bytes

This small inline function aligns the pointer by the certian number of bytes by shifting
it forward by 0 or a positive offset.
*/
template<typename _Tp> static inline _Tp* align_ptr_opencv(_Tp* ptr, int n = (int)sizeof(_Tp))
{
	return (_Tp*)(((size_t)ptr + n - 1) & -n);
}


/*!
Aligns buffer size by the certain number of bytes

This small inline function aligns a buffer size by the certian number of bytes by enlarging it.
*/
static inline size_t align_size_opencv(size_t sz, int n)
{
	assert((n & (n - 1)) == 0); // n is a power of 2
	return (sz + n - 1) & -n;
}