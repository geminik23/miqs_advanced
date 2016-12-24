#pragma once
#include "test_base.h"

#define TEST_CLASSNAME test_pitch_shift

namespace miqs
{



namespace test
{

template <typename _Ty>
struct test_pitch_shift: base_test<_Ty>
{
	virtual void initialize() override;
	virtual void process(miqs::array_ref<value_type>& in, size_t ich, miqs::array_ref<value_type>& out, size_t och) override;

private:
	miqs::pitch_shift<value_type, 3> m_ps1,m_ps2;
	
	sine_440<value_type> m_sine;
};

template<typename _Ty>
inline void test_pitch_shift<_Ty>::initialize()
{
	// init with samplerate
	m_ps1.samplerate(samplerate());
	m_ps2.samplerate(samplerate());

	m_sine.set(samplerate());

	// set pitch_ratio
	m_ps1.pitch_ratio(miqs::semitone_to_ratio(7));	//left
	m_ps2.pitch_ratio(miqs::semitone_to_ratio(-7));	//right


}

template<typename _Ty>
inline void test_pitch_shift<_Ty>::process(miqs::array_ref<value_type>& in, size_t ich, miqs::array_ref<value_type>& out, size_t och)
{
	miqs::array<value_type> i1(vector_size()), i2(vector_size());
	auto i_ch1 = miqs::ptr_interleaved(miqs::ptr_begin(in), ich, 0);
	auto i_ch2 = miqs::ptr_interleaved(miqs::ptr_begin(in), ich, 1);

	auto o_ch1 = miqs::ptr_interleaved(miqs::ptr_begin(out), och, 0);
	auto o_ch2 = miqs::ptr_interleaved(miqs::ptr_begin(out), och, 1);

	/* microphone */
	//ptr_copy(i_ch1, i_ch1 + vector_size(), ptr_begin(i1));
	//ptr_copy(i_ch2, i_ch2 + vector_size(), ptr_begin(i2));
	/* microphone end*/

	/* sinewave */
	miqs::ptr_generate(miqs::ptr_begin(i1), miqs::ptr_end(i1), std::ref(m_sine));
	miqs::ptr_copy(miqs::ptr_begin(i1), miqs::ptr_end(i1), ptr_begin(i2));
	/* sinewave end*/

	m_ps1(ptr_arg(i1));
	m_ps2(ptr_arg(i2));

	ptr_copy(ptr_begin(i1), ptr_end(i1), o_ch1);
	ptr_copy(ptr_begin(i2), ptr_end(i2), o_ch2);

}

}
}